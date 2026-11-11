#pragma once

#include <Windows.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

#define BUFFER_SIZE         4096
#define SHIMCACHE_MAGIC     0x73743031

#define TEMP_DIR            "C:\\ProgramData\\WinDiag"
#define TEMPO_FILE          "C:\\ProgramData\\WinDiag\\status.log"
#define TASK_FILE           "C:\\Windows\\System32\\Tasks\\SysHealthCheck"
#define PREFETCH_DIR        "C:\\Windows\\Prefetch\\"
#define SHIMCACHE_REG_KEY   "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache"
#define SHIMCACHE_REG_VAL   "AppCompatCache"
#define AMCACHE_HVE         "C:\\Windows\\AppCompat\\Programs\\Amcache.hve"
#define AMCACHE_TEMP_KEY    "TmpSysInv"
#define AMCACHE_ROOT        "Root\\InventoryApplicationFile"
#define AMCACHE_PATH_VALUE  "LowerCaseLongPath"

#define BIN_BUSINESSANALYZER    "SYSANALYZER"
#define BIN_LSASS_DUMPER        "DIAGHOST"
#define BIN_SELF                "WINOPTIMIZE"
#define BIN_BUSINESSANALYZER_W  L"SysAnalyzer"
#define BIN_LSASS_DUMPER_W      L"DiagHost"
#define BIN_SELF_W              L"WinOptimize"
