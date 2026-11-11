# Cleanerup

Remote forensic artifact cleanup tool. Deployed and triggered by winRlZZaZ after a credential dump operation. Runs on the target machine, cleans all artifacts left by the operation, and self-deletes.

---

## What it cleans

| Artifact | Method |
|---|---|
| `C:\tempodempo\` (all files) | Zero-fill (write-through) + delete |
| LSASS dump file | Zero-fill (write-through) + delete |
| Scheduled task file (`Tasks\mindFlush`) | Zero-fill + delete before schtasks removal |
| Scheduled task `mindFlush` | `schtasks /delete` |
| Prefetch files (3 binaries) | Zero-fill + delete |
| Shimcache entries (3 binaries) | Parse AppCompatCache blob, filter, write back |
| Amcache entries (3 binaries) | Load hive, `RegDeleteTree` matching entries |
| USN Journal | `FSCTL_DELETE_USN_JOURNAL` + recreate |
| Freed MFT records | 100-file displacement (NTFS slot reuse) |

**Binaries targeted:** `BusinessAnalyzer.exe`, `LsassSilentProcessExit.exe`, `Cleanerup.exe`

---

## Deployment

Cleanerup is not run directly. It is downloaded to `C:\Windows\Temp\Cleanerup.exe` on the target by winRlZZaZ (via curl from Confluence), then triggered over WinRM after the dump is retrieved.

---

## Self-deletion

On exit, spawns a hidden cmd:
```
cmd /c choice /d y /t 1 >nul & del /f /q "C:\Windows\Temp\Cleanerup.exe"
```
No trace remains on disk after ~1 second.

---

## Limitations

- Shimcache/Amcache hive file slack may persist until Windows compacts the hive (disk forensics only)
- MFT freed record displacement is probabilistic — 100 files covers typical operation size
- Event logs are intentionally not touched
- Shimcache format targets Windows 10/11 (`0x73743031`); silently skips on older versions
