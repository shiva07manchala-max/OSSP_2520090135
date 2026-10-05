# TEST REPORT — Linux File System Explorer Using System Calls

Course    : Operating System and System Programming (25CS2104E)
Team 14   : M. Shiva (2520090135), N. Sainath Reddy (2520090115)
Compiler  : GCC 13.3.0 (Ubuntu 24.04, WSL2)
Flags     : -std=c11 -Wall -Wextra -Wpedantic -O2
Build     : **PASS — zero compiler warnings**
Runner    : `bash tests/run_demo.sh` (scripted input from `tests/demo_input.txt`,
            executed in scratch directory `/tmp/fse_demo`)

All results below are **actual observed outputs** from the scripted runs, not
expected placeholders. The application never crashed and exited cleanly with
code 0 through the menu Exit option (now option 13).

---

## TEST 1 — Start application

| Field | Detail |
|---|---|
| Test ID | TEST 1 |
| Input | run `./build/explorer` |
| Expected output | Banner, menu with 13 options, `Current Directory:` shown |
| Actual result | Banner "LINUX FILE SYSTEM EXPLORER / USING SYSTEM CALLS", list of demonstrated syscalls, current directory and full menu displayed |
| Pass/Fail | **PASS** |

## TEST 2 — List valid directory

| Field | Detail |
|---|---|
| Test ID | TEST 2 |
| Input | Menu `2`, path `.` (scratch dir containing notes.txt, empty_demo.txt, ProjectFiles) |
| Expected output | Table NAME / TYPE with correct per-entry type, entry count |
| Actual result | `notes.txt FILE`, `empty_demo.txt FILE`, `ProjectFiles DIRECTORY`, `Total entries: 3`, `[SUCCESS] Directory listed successfully.` |
| Pass/Fail | **PASS** |

## TEST 3 — List invalid directory

| Field | Detail |
|---|---|
| Test ID | TEST 3 |
| Input | Menu `2`, path `/no/such/directory` |
| Expected output | Error with reason, program continues |
| Actual result | `[ERROR] ERROR: Unable to open directory.` + `Reason: No such file or directory` (errno ENOENT via opendir) |
| Pass/Fail | **PASS** |

## TEST 4 — Create file

| Field | Detail |
|---|---|
| Test ID | TEST 4 |
| Input | Menu `4`, filename `notes.txt` |
| Expected output | File descriptor number printed, success message |
| Actual result | `File opened successfully.` / `File Descriptor: 3` / `[SUCCESS] File created successfully.` (open O_CREAT\|O_WRONLY\|O_TRUNC, mode 0644) |
| Pass/Fail | **PASS** |

## TEST 5 — Create existing file

| Field | Detail |
|---|---|
| Test ID | TEST 5 |
| Input | Menu `4`, filename `notes.txt` (already exists) |
| Expected output | No crash; file recreated/truncated per O_TRUNC, success message |
| Actual result | `File Descriptor: 3`, `[SUCCESS] File created successfully.` — existing content was truncated (documented O_TRUNC behaviour) |
| Pass/Fail | **PASS** |

## TEST 6 — Read valid file

| Field | Detail |
|---|---|
| Test ID | TEST 6 |
| Input | Menu `5`, filename `notes.txt` (63 bytes written earlier via option 6) |
| Expected output | Content displayed, byte count reported |
| Actual result | `---- FILE CONTENT START ----` + full line + `Total bytes read: 63` + `[SUCCESS]` |
| Pass/Fail | **PASS** |

## TEST 7 — Read missing file

| Field | Detail |
|---|---|
| Test ID | TEST 7 |
| Input | Menu `5`, filename `no_such_file.txt` |
| Expected output | Clear error, program continues |
| Actual result | `[ERROR] Unable to open file.` + `Reason: No such file or directory` (open returned -1, errno ENOENT) |
| Pass/Fail | **PASS** |

## TEST 8 — Write valid file

| Field | Detail |
|---|---|
| Test ID | TEST 8 |
| Input | Menu `6`, filename `notes.txt`, content `Operating Systems and System Programming - written via write()` |
| Expected output | Success message; bytes actually written (partial-write loop) |
| Actual result | `File opened successfully. File Descriptor: 3`, `[SUCCESS] Content written successfully.`; subsequent read returned exactly the 63 written bytes |
| Pass/Fail | **PASS** |

## TEST 9 — Create directory

| Field | Detail |
|---|---|
| Test ID | TEST 9 |
| Input | Menu `7`, name `ProjectFiles` |
| Expected output | Directory created with sensible permissions |
| Actual result | `[SUCCESS] Directory created successfully.`; metadata later showed `drwxr-xr-x` (mode 0755 filtered by umask) |
| Pass/Fail | **PASS** |

## TEST 10 — Create existing directory

| Field | Detail |
|---|---|
| Test ID | TEST 10 |
| Input | Menu `7`, name `ProjectFiles` (again) |
| Expected output | Error explaining the conflict |
| Actual result | `[ERROR] Unable to create directory.` + `Reason: File exists` (errno EEXIST) |
| Pass/Fail | **PASS** |

## TEST 11 — Remove empty directory

| Field | Detail |
|---|---|
| Test ID | TEST 11 |
| Input | Menu `8`, name `DemoEmpty` (created just before) |
| Expected output | Directory removed |
| Actual result | `[SUCCESS] Directory removed successfully.` (rmdir) |
| Pass/Fail | **PASS** |

## TEST 12 — Attempt to remove non-empty directory

| Field | Detail |
|---|---|
| Test ID | TEST 12 |
| Input | Menu `8`, name `ProjectFiles` (contains `inner.txt`) |
| Expected output | Error explaining directory is not empty |
| Actual result | `[ERROR] Unable to remove directory.` + `Reason: Directory not empty` (errno ENOTEMPTY) |
| Pass/Fail | **PASS** |

## TEST 13 — Display file metadata

| Field | Detail |
|---|---|
| Test ID | TEST 13 |
| Input | Menu `9`, name `notes.txt`; extra run: name `Extra` (a directory) |
| Expected output | Name, type, size, permissions, inode, owner, 3 timestamps |
| Actual result | Full table: `File Type : Regular File`, `Permissions : -rw-r--r--`, size 63, inode number, `Owner : root (0)`, Last Accessed / Last Modified / Status Changed timestamps. Directory run correctly showed `File Type : Directory`, `drwxr-xr-x`, size 4096, hard links 2 |
| Pass/Fail | **PASS** |

## TEST 14 — lseek() with valid offset

| Field | Detail |
|---|---|
| Test ID | TEST 14 |
| Input | Menu `10`, filename `notes.txt`, offset `35` |
| Expected output | New offset reported, content from that position shown |
| Actual result | `File offset moved successfully. New offset: 35` + remaining text `mming - written via write()` read from that position |
| Pass/Fail | **PASS** |

## TEST 15 — Invalid input handling

| Field | Detail |
|---|---|
| Test ID | TEST 15 |
| Input | Menu `99`; menu `abc`; empty filename (option 4 + bare Enter); negative offset `-5`; offset beyond EOF `99999`; read a directory name; chdir `/no/such/directory` |
| Expected output | Specific validation message or errno-based error for each; **no crash** |
| Actual result | `[ERROR] Invalid menu choice. Please enter a number 1-13.` (both `99` and `abc`); `[ERROR] Filename cannot be empty.`; `[ERROR] Invalid offset. Offset cannot be negative.`; beyond-EOF → offset moved, `[NOTICE] Offset is at (or beyond) end of file - 0 bytes read.`; read on directory → `[ERROR] Unable to read file.` + `Reason: Is a directory` (EISDIR); invalid chdir → `[ERROR] ERROR: Unable to access directory.` + reason |
| Pass/Fail | **PASS** |

## TEST 16 — Exit application

| Field | Detail |
|---|---|
| Test ID | TEST 16 |
| Input | Menu `11` |
| Expected output | Goodbye message, clean exit, no leaked descriptors/streams |
| Actual result | `[SUCCESS] Exiting Linux File System Explorer. Goodbye!` + closing note that all fds/directory streams were closed; **process exit code 0** |
| Pass/Fail | **PASS** |

---

## Additional robustness checks (beyond the 16 required)

| Check | Input | Observed | Result |
|---|---|---|---|
| Non-empty directory listing | dir with a.txt, b.txt, Extra, input.txt | 4 rows, correct FILE/DIRECTORY types, `Total entries: 4` | PASS |
| lseek negative offset | `-5` | Rejected before any syscall: `Offset cannot be negative.` | PASS |
| lseek beyond EOF | `99999` | Offset accepted by kernel, 0 bytes read, NOTICE shown | PASS |
| read() on a directory | option 5 on dir | open succeeds, read fails with `Is a directory` (EISDIR) — errno path demonstrated | PASS |
| Overwrite (write) | option 6 on a.txt | Content replaced; read-back matched exactly | PASS |

---

## Feature-extension tests (options 11 Copy / 12 Rename)

Added after the original 16-test pass, in the same scripted manner.

| Check | Input | Observed | Result |
|---|---|---|---|
| Copy valid file (option 11) | `notes.txt` → `copy_of_notes.txt` (63 bytes written earlier) | `Source opened. File Descriptor: 3`, `Destination opened. File Descriptor: 4`, `Copied 63 bytes.`, `[SUCCESS] File copied successfully.`; option-5 read-back of the copy matched exactly | PASS |
| Copy preserves permissions | copy of a 0644 file | destination listed as `-rw-r--r--` (mode from `fstat(st_mode)`, umask-filtered) | PASS |
| Copy to existing destination | option 11 to an existing name | destination truncated and overwritten; success reported (documented O_TRUNC policy) | PASS |
| Copy with identical source/destination | `notes.txt` → `notes.txt` | `[ERROR] Source and destination are the same file.` — rejected before any syscall | PASS |
| Copy missing source | option 11 on a missing name | `[ERROR] Unable to open source file.` + `Reason: No such file or directory` (ENOENT) | PASS |
| Copy a directory | option 11 on a directory name | `[ERROR] Source is a directory - copy works on files only.` (refused via `fstat`+`S_ISDIR` before any read) | PASS |
| Rename file (option 12) | `copy_of_notes.txt` → `renamed_copy.txt` | `Renamed: copy_of_notes.txt -> renamed_copy.txt`, `[SUCCESS] Rename completed successfully.`; content intact after rename | PASS |
| Rename onto existing name | `renamed.txt` → `temp_target.txt` (existed) | target atomically replaced per POSIX semantics; success reported | PASS |
| Rename a directory | `OldDir` → `NewDir` | directory renamed without copying data | PASS |
| Rename missing source | `no_such_file.txt` → `renamed.txt` | `[ERROR] Unable to rename.` + `Reason: No such file or directory` (ENOENT) | PASS |

## Summary

- 16/16 required tests **PASS**; 5 additional robustness checks **PASS**;
  10 feature-extension checks (options 11/12) **PASS**.
- No crash, hang, or unhandled error observed in any run.
- Every failure path produced an `[ERROR]` message plus the `strerror(errno)` reason.
- Build reproducible with `make clean && make` (zero warnings, GCC 13.3).
- `strace` was not available in the test environment, so the atomicity of
  `rename()` was verified behaviourally (existing target replaced, ENOENT
  reported) rather than by syscall tracing.
