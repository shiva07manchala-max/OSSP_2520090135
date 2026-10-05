# LINUX FILE SYSTEM EXPLORER USING SYSTEM CALLS

**Course:** Operating System and System Programming
**Course Code:** 25CS2104E
**Team 14, Section 12** · Faculty: M. Raghupathi
**Language:** C (C11) · **Platform:** Ubuntu/Linux · **Compiler:** GCC
**IDE:** VS Code + Linux Terminal

---

## 1. Project Description

A menu-driven command-line application written in C that performs real file
and directory operations by talking **directly to the Linux kernel** through
low-level POSIX system calls — `open()`, `read()`, `write()`, `close()`,
`lseek()`, `stat()`, `opendir()`, `readdir()`, `mkdir()`, `rmdir()` and more.
It does **not** simulate file operations and does **not** use the buffered
standard-I/O functions (`fopen`/`fgets`/`fprintf`) for its core file
demonstrations: every operation is a genuine kernel interaction.

## 2. Problem Statement

> Many file-management applications hide the low-level operations performed
> by the operating system. This makes it difficult for students to understand
> how Linux handles files, directories, metadata, permissions, and file
> descriptors.

## 3. Proposed Solution

A simple command-line **File System Explorer** that directly uses Linux/POSIX
system calls to perform common file and directory operations and demonstrate
communication between a user program and the Linux kernel. Every system call
return value is checked, errors are reported through `errno`/`perror()`/
`strerror()`, and invalid user input is handled without ever crashing.

## 4. Objectives

1. Implement a Linux command-line file-system explorer using C and
   POSIX/Linux system calls.
2. Provide directory navigation, file creation, reading, writing, and listing.
3. Display file metadata such as size, type, permissions, and timestamps.
4. Demonstrate file descriptors, system calls, directory handling, and kernel
   interaction.

## 5. Features

| # | Feature | Menu | System calls demonstrated |
|---|---------|------|---------------------------|
| 1 | Show current directory | 1 | `getcwd()` |
| 2 | List directory contents | 2 | `opendir()`, `readdir()`, `closedir()`, `lstat()` |
| 3 | Change directory | 3 | `chdir()` |
| 4 | Create file | 4 | `open()` (`O_CREAT\|O_WRONLY\|O_TRUNC`), `close()` — prints the **file descriptor** |
| 5 | Read file | 5 | `open()`, `read()`, `close()` — handles empty file, EISDIR, partial reads |
| 6 | Write to file | 6 | `open()`, `write()`, `close()` — handles partial writes |
| 7 | Create directory | 7 | `mkdir()` (mode 0755, filtered by umask) |
| 8 | Remove directory | 8 | `rmdir()` (explains ENOTEMPTY, ENOENT) |
| 9 | File information | 9 | `stat()`/`lstat()`, `S_ISREG/S_ISDIR/S_ISLNK/…`, permissions, inode, 3 timestamps |
| 10 | File offset / random access | 10 | `lseek()` (`SEEK_SET`) + `read()` from an offset |
| 11 | Copy file | 11 | `open()` ×2, `fstat()`, `read()`/`write()` loop, `close()` ×2 — byte-exact kernel-level copy |
| 12 | Rename (move) file/directory | 12 | `rename()` — one atomic kernel directory-entry move |
| 13 | Exit | 13 | clean shutdown — all fds and directory streams closed |

Additional behaviour: file-descriptor demonstration (0 = stdin, 1 = stdout,
2 = stderr explained in code and output), strict input validation, and
`[SUCCESS] / [ERROR] / [NOTICE]` message styling.

## 6. Technologies Used

- **C (ISO C11)** — no external libraries; POSIX/Linux APIs only.
- **GCC** (tested with GCC 13.3 on Ubuntu 24.04) with `-Wall -Wextra -Wpedantic`.
- **GNU Make** for the build.
- Runs on any modern Linux (tested in WSL2 Ubuntu 24.04).

## 7. Linux/POSIX APIs Used

| API | Where | Purpose |
|-----|-------|---------|
| `open()` | file_operations.c | open/create files; returns a file descriptor |
| `read()` | file_operations.c | copy bytes from the file into a user buffer |
| `write()` | file_operations.c | copy bytes from a user buffer into the file |
| `close()` | file_operations.c | release the file descriptor |
| `lseek()` | file_operations.c | move the file offset (random access) |
| `fstat()` | file_operations.c | metadata of an *open* file (by descriptor) — source mode for the copy |
| `rename()` | file_operations.c | atomically move a directory entry to a new name |
| `stat()` / `lstat()` | metadata.c (+ listing) | fetch file metadata (`struct stat`); `lstat` does not follow symlinks |
| `opendir()` / `readdir()` / `closedir()` | directory_operations.c | directory stream handling |
| `mkdir()` | directory_operations.c | create a directory |
| `rmdir()` | directory_operations.c | remove an empty directory |
| `getcwd()` / `chdir()` | navigation.c | query / change the process working directory |
| `errno`, `perror()`, `strerror()` | utils.c | error detection and reporting |

## 8. Project Architecture

```
USER
  ↓
COMMAND-LINE INTERFACE            (menu, banners, prompts — utils.c, main.c)
  ↓
INPUT VALIDATION                  (safe line input, integer parsing, path checks)
  ↓
OPERATION MODULE                  (file_operations / directory_operations /
                                   metadata / navigation)
  ↓
LINUX/POSIX API                   (open, read, write, lseek, stat, opendir, …)
  ↓
SYSTEM CALL / KERNEL              (syscall trap → VFS → driver/file system)
  ↓
FILE SYSTEM                       (ext4 / tmpfs / NTFS-in-WSL …)
  ↓
RESULT / ERROR                    (return value + errno)
  ↓
COMMAND-LINE INTERFACE            ([SUCCESS] / [ERROR] + strerror(reason))
```

**Program flow:** START → initialize → show current directory → show menu →
accept choice → validate → execute operation → call POSIX API → check return
value → display result/error → back to menu → EXIT on option 13.

## 9. Folder Structure

```
LinuxFileSystemExplorer/
├── src/
│   ├── main.c                    # menu loop and dispatch
│   ├── file_operations.c         # open/read/write/close/lseek
│   ├── directory_operations.c    # opendir/readdir, mkdir, rmdir
│   ├── metadata.c                # stat/lstat, type, perms, timestamps, inode
│   ├── navigation.c              # getcwd, chdir
│   └── utils.c                   # safe input, menus, [SUCCESS]/[ERROR] output
├── include/                      # matching headers (include guards)
├── build/                        # objects + binary (generated, git-ignored)
├── tests/
│   ├── demo_input.txt            # scripted input driving all 18 tests
│   ├── run_demo.sh               # runs the script in a scratch directory
│   └── test_cases.md             # formal test report with ACTUAL results
├── screenshots/                  # place terminal screenshots here
├── docs/
│   └── PROJECT_DOCUMENTATION.md  # formal report (Abstract → Conclusion)
├── Makefile
├── README.md
└── .gitignore
```

## 10. Installation Requirements

- Ubuntu (or any) Linux with `gcc`, `make` and `libc-dev`:

```bash
sudo apt update && sudo apt install -y build-essential
```

- (Windows users) WSL2 Ubuntu works unchanged — the project was built and
  tested under WSL2 Ubuntu 24.04.

## 11. Compilation Instructions

Using Make (recommended):

```bash
make            # build  → build/explorer
make run        # build if needed and start the explorer
make clean      # remove build outputs
make help       # show targets
```

Direct GCC one-liner (equivalent):

```bash
mkdir -p build && gcc -std=c11 -Wall -Wextra -Wpedantic -O2 \
    -Iinclude -Isrc src/*.c -o build/explorer
```

The project compiles with **zero warnings** under `-Wall -Wextra -Wpedantic`.

## 12. Execution Instructions

```bash
./build/explorer
```

or simply `make run`. Then choose options 1–11 from the menu.

Scripted demonstration (uses `tests/demo_input.txt`):

```bash
bash tests/run_demo.sh        # runs in /tmp scratch dir, leaves project clean
```

## 13. Sample Output

```text
============================================================
              LINUX FILE SYSTEM EXPLORER
                 USING SYSTEM CALLS
============================================================
 Language    : C (POSIX / Linux system calls)
 ...
Current Directory: /tmp/fse_demo
------------------------------------------------------------
 NAVIGATION
    1. Show Current Directory
    2. List Directory Contents
    3. Change Directory
 FILE OPERATIONS
    4. Create File
    5. Read File
    6. Write to File
 DIRECTORY OPERATIONS
    7. Create Directory
    8. Remove Directory
 FILE INFORMATION
    9. File Information
   10. File Offset / Random Access
 ADVANCED FILE OPERATIONS
   11. Copy File
   12. Rename (Move) File
   13. Exit
------------------------------------------------------------
Enter your choice:
```

Creating, writing, reading and metadata (abridged real output):

```text
Enter filename: notes.txt
File opened successfully.
File Descriptor: 3
[SUCCESS] File created successfully.

Enter content (one line): Operating Systems and System Programming - written via write()
[SUCCESS] Content written successfully.

---- FILE CONTENT START ----
Operating Systems and System Programming - written via write()
---- FILE CONTENT END ----
Total bytes read: 63
[SUCCESS] File read successfully.

File Name       : notes.txt
File Type       : Regular File
Permissions     : -rw-r--r--
File Size       : 63 bytes
Inode           : 17919
Hard Links      : 1
Owner           : root (0)
Group           : root (0)
Last Accessed   : 2026-09-17 05:36:47
Last Modified   : 2026-09-17 05:36:47
Status Changed  : 2026-09-17 05:36:47
[SUCCESS] File information displayed.
```

Random access with `lseek()` (offset 35):

```text
Enter offset from beginning of file: 35
File offset moved successfully. New offset: 35
Content from offset 35:
---- READ FROM OFFSET ----
mming - written via write()
---- END ----
[SUCCESS] Read from offset completed.
```

Error handling examples (real messages):

```text
[ERROR] Unable to open file.
Reason: No such file or directory

[ERROR] Unable to create directory.
Reason: File exists

[ERROR] Unable to remove directory.
Reason: Directory not empty
```

## 14. Testing

16 required test scenarios (plus 5 extra robustness checks) were executed
via the scripted runner and **all passed** — see
[`tests/test_cases.md`](tests/test_cases.md) for the complete report with
actual observed outputs.

```bash
make clean && make
bash tests/run_demo.sh
```

## 15. Error Handling

- **Every** system call return value is checked; failures are never ignored.
- `errno` is captured immediately and reported as
  `[ERROR] <context>` + `Reason: <strerror(errno)>` (`perror()` and
  `strerror()` are both demonstrated in `utils.c`).
- Specific errno cases handled: `ENOENT`, `EACCES`, `EEXIST`, `ENOTEMPTY`,
  `EISDIR`, `ENOTDIR`, `EINTR` (retried in read/write loops).
- Input validation: empty filenames, empty paths, over-long input, non-numeric
  menu choices, non-numeric/negative offsets — all rejected with clear
  messages. No `gets()`/`scanf()` buffer overflows; only bounded `fgets()`.
- `readdir()` errors are distinguished from end-of-directory by pre-zeroing
  `errno`.
- The program never crashes on normal invalid input and always releases file
  descriptors and directory streams on every path.

## 16. Learning Outcomes

- How a C program requests kernel services through system calls.
- File descriptors as process-local handles to kernel open-file objects.
- Unbuffered I/O vs buffered stdio; partial read/write semantics.
- Random access with `lseek()` — the offset is a property of the open file
  description, not the file.
- File metadata through `struct stat`: type, size, permissions, inode, and
  the three timestamps.
- Directory streams, `.` and `..`, and the permission/umask model.
- Defensive systems programming: validation, errno discipline, resource
  cleanup.

## 17. Future Enhancements

File copy and file rename were added as options 11 and 12 (documented
below). The remaining items are listed as future work — deliberately
**not** implemented so the core project stays focused:

- Recursive directory listing
- File search by name
- Permission modification (`chmod()`)
- Symbolic link information display
- Disk usage information
- Command history, arrow-key interface (`ncurses`)

## 18. Demo Sequence (recommended for review)

| Step | Menu | API demonstrated |
|------|------|------------------|
| 1 | start | banner + menu |
| 2 | 1 | `getcwd()` |
| 3 | 2 | `opendir()`/`readdir()` |
| 4 | 7 | `mkdir()` |
| 5 | 4 | `open()`/`close()` + file descriptor |
| 6 | 6 | `open()`/`write()`/`close()` |
| 7 | 5 | `open()`/`read()`/`close()` |
| 8 | 9 | `stat()` metadata |
| 9 | 10 | `lseek()` random access |
| 10 | 11 | `open`/`fstat`/`read`/`write`/`close` file copy |
| 11 | 12 | `rename()` atomic move |
| 12 | 8 | `rmdir()` (on an empty dir) |
| 13 | 13 | clean exit |

## 19. Team Members

| Name | Roll Number |
|------|-------------|
| M. Shiva | 2520090135 |
| N. Sainath Reddy | 2520090115 |

Team 14 · Section 12 · Faculty: M. Raghupathi
GitHub: `KLH_CSIT_2026-27_14_FileSystemExplorer`

---

# VIVA QUESTIONS AND ANSWERS

**1. What is a system call?**
A system call is the controlled entry point through which a user program
requests a service from the operating system kernel (e.g. opening a file).
The C library wrapper places the call number/arguments in registers and
executes a trap instruction; the CPU switches from user mode to kernel mode,
the kernel performs the work, and the result is returned to the process.

**2. What is a file descriptor?**
A small non-negative integer, local to a process, that identifies an open
file in the kernel's open-file table. By convention 0 = standard input,
1 = standard output, 2 = standard error; `open()` returns the lowest
free number (usually 3 for the first file a program opens).

**3. What does `open()` return?**
On success, the lowest-numbered unused file descriptor. On failure it returns
-1 and sets `errno` to describe the problem (e.g. `ENOENT`, `EACCES`).

**4. Difference between `open()` and `fopen()`?**
`open()` is a thin system-call wrapper returning a file descriptor and doing
no buffering. `fopen()` is a C library function that itself calls `open()`
and wraps the descriptor in a `FILE*` with a user-space buffer, plus
formatting support (`fprintf` etc.). Our project uses `open()` to demonstrate
the kernel interface directly.

**5. What does `read()` do?**
`read(fd, buf, count)` copies up to `count` bytes from the file (at its
current offset) into `buf`. It returns the number of bytes actually read
(0 = end of file, -1 = error with `errno` set). Fewer bytes than requested
can be returned — that is why we loop.

**6. What does `write()` do?**
`write(fd, buf, count)` copies up to `count` bytes from `buf` into the file
at its current offset, advancing the offset. It returns the number of bytes
actually written, which may be less than requested (partial write) — so we
loop until everything is written.

**7. Why is `close()` required?**
It releases the file descriptor back to the kernel, flushes kernel state
associated with the open file, and updates metadata (e.g. mtime on last
close). Every process has a limited descriptor table; leaking descriptors by
never closing files eventually makes `open()` fail with `EMFILE`.

**8. What is `lseek()`?**
`lseek(fd, offset, whence)` changes the file offset associated with the open
file description — the position where the next `read()`/`write()` happens.
`whence` is `SEEK_SET` (from beginning), `SEEK_CUR` or `SEEK_END`. It performs
no I/O itself; it just moves the offset. Our option 10 uses `SEEK_SET`.

**9. What is `stat()`?**
`stat(path, &buf)` asks the kernel for the metadata of the file named by
`path` and fills a `struct stat`: type/mode, size, inode, link count, uid/gid
and the three timestamps. `lstat()` is identical except it does **not** follow
a final symbolic link — our project uses `lstat()` when listing so symlinks
show up as links.

**10. What is an inode?**
The inode ("index node") is the on-disk data structure that stores a file's
metadata (type, permissions, size, timestamps, data-block pointers). Directory
entries map names to inode numbers; `st_ino` shows the inode of a file. Many
names (hard links) can point to one inode.

**11. Difference between a file and a directory?**
A regular file stores raw bytes. A directory is itself a special file that
stores a table of *name → inode* entries. The kernel returns different type
bits in `st_mode`, testable with `S_ISREG()` / `S_ISDIR()`; directories can
be opened with `opendir()` but `read()` on them fails with `EISDIR`
(demonstrated in our tests).

**12. What are file permissions?**
Nine mode bits per file: read/write/execute for owner, group and others
(e.g. `rw-r--r--` = 0644). They are checked by the kernel at `open()`
time. `mkdir()` takes a mode argument that is filtered by the process
umask — our directories appear as `drwxr-xr-x` (0755).

**13. What are `opendir()` and `readdir()`?**
`opendir(path)` opens a directory stream (it calls `open()` internally);
`readdir()` returns one `struct dirent` per call (the entry name, and
`d_type` on Linux), and NULL at end-of-stream; `closedir()` releases it.
That is exactly how our listing option works.

**14. What does `mkdir()` do?**
Creates a new empty directory with the requested mode (filtered by umask).
The kernel automatically creates its `.` and `..` entries. It fails with
`EEXIST` if the name exists and `ENOENT`/`EACCES` if the path is invalid or
permission is denied.

**15. What does `rmdir()` do?**
Deletes an **empty** directory. If the directory still contains entries the
kernel refuses with `ENOTEMPTY` — we demonstrate that exact case in TEST 12.

**16. What is `errno`?**
A per-thread integer, set by system calls and library functions when they
fail, holding a code for what went wrong (e.g. `ENOENT` = no such file or
directory). It is only meaningful when the call's return value indicates
failure.

**17. Why is `perror()` useful?**
It prints the message string you supply, followed by a colon and the human-
readable text for the current `errno` — turning numeric error codes into
`Reason: No such file or directory` style messages. `strerror(errno)` gives
the same text as a string, which we format into `[ERROR]` blocks.

**18. What happens when a system call fails?**
The kernel returns -1 (or another error sentinel) and sets `errno`; the
calling program is expected to check the return value and react — retry
(`EINTR`), report, or abort. Our program checks every call and prints
`[ERROR]` with the `strerror()` reason, never continuing silently.

**19. What is a kernel?**
The core of the operating system that runs in privileged (kernel) mode and
manages CPU, memory, processes and I/O — including the file system. User
programs cannot touch hardware directly; they request services via system
calls.

**20. How does a user program interact with the Linux kernel?**
Through the system-call interface: the program calls a libc wrapper
(e.g. `read()`), which puts the syscall number and arguments in registers and
traps into the kernel; the kernel validates the arguments, performs the
operation on the file system, and returns a result (value or -1 + `errno`)
to the user process. Every feature of this project is one of those round
trips.

**21. (Bonus) What does `rename()` do and why is it called atomic?**
`rename(oldpath, newpath)` moves a directory entry from one name to another
in a single kernel operation — no file data is read or written. It is atomic
because observers see either the old name or the new name, never a partial
state; if `newpath` exists (same file system, regular file) it is replaced.
Our option 12 demonstrates it on both files and directories.

**22. (Bonus) Difference between `stat()` and `fstat()`?**
Both fill a `struct stat`, but `stat()` takes a *path* (the kernel resolves
the name at call time) while `fstat()` takes an already-open *file
descriptor*. `fstat()` is therefore race-free against the file being renamed
or swapped after `open()`, and it is the natural choice once a descriptor is
in hand — our copy option uses `fstat()` to learn the source's mode and to
reject directories before reading.
