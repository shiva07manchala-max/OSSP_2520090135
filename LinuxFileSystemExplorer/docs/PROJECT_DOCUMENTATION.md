# PROJECT DOCUMENTATION

**Linux File System Explorer Using System Calls**
Operating System and System Programming (25CS2104E)
Team 14, Section 12 — M. Shiva (2520090135), N. Sainath Reddy (2520090115)
Faculty: M. Raghupathi · GitHub: `KLH_CSIT_2026-27_14_FileSystemExplorer`

---

## A. Project Abstract

This project presents a menu-driven command-line application, written in C,
that explores the Linux file system by performing real operations through
low-level POSIX system calls rather than emulating them in user space. The
explorer demonstrates the complete path from a user process to the Linux
kernel: input validation → operation module → POSIX API (`open`, `read`,
`write`, `close`, `lseek`, `stat`, `opendir`, `readdir`, `mkdir`, `rmdir`,
`getcwd`, `chdir`) → system call → VFS/file system → result or error. Every
return value is checked and reported through `errno`/`strerror()`, making the
program both a functional file tool and a living demonstration of
operating-system concepts for the laboratory and viva.

## B. Introduction

Modern applications interact with files through high-level abstractions
(GUIs, frameworks, buffered streams) that conceal what the operating system
actually does. At the heart of every such operation is a **system call** —
the controlled doorway through which user code asks the kernel for a service.
This project opens that doorway deliberately: it is a C program whose entire
functionality is built on raw, unbuffered POSIX file and directory calls, so
that each menu option visibly demonstrates one kernel interaction: file
descriptors, file offsets, metadata (`struct stat`), permission bits, inode
numbers, and directory streams.

## C. Problem Statement

> Many file-management applications hide the low-level operations performed
> by the operating system. This makes it difficult for students to understand
> how Linux handles files, directories, metadata, permissions, and file
> descriptors.

*(Core statement as approved in the project form; reproduced verbatim.)*

## D. Proposed Solution

A simple command-line **File System Explorer** that directly uses Linux/POSIX
system calls to perform common file and directory operations and demonstrate
communication between a user program and the Linux kernel. The solution is
modular (one module per concern), strictly validates all input, checks every
system-call return value, reports failures with `perror()`/`strerror(errno)`
explanations, and never crashes on invalid input.

## E. Objectives

1. Implement a Linux command-line file-system explorer using C and
   POSIX/Linux system calls.
2. Provide directory navigation, file creation, reading, writing, and listing.
3. Display file metadata such as size, type, permissions, and timestamps.
4. Demonstrate file descriptors, system calls, directory handling, and kernel
   interaction.

## F. Hardware Requirements

- Any PC capable of running Linux (x86-64 recommended); tested on WSL2
  (Windows 11 host).
- Minimum: 2 GB RAM, 200 MB free disk space.
- No special hardware — the project uses only the standard file system.

## G. Software Requirements

- **OS:** Ubuntu Linux 22.04/24.04 (or any modern distribution; WSL2 Ubuntu
  24.04 was used for development and testing).
- **Compiler:** GCC (tested with GCC 13.3.0).
- **Build tool:** GNU Make (tested with Make 4.3).
- **Libraries:** C standard library + POSIX (glibc) — **no external
  dependencies**.
- **Editor/IDE:** VS Code + Linux terminal.

## H. System Architecture

```
USER
  ↓
COMMAND-LINE INTERFACE            menu, banners, prompts (utils.c, main.c)
  ↓
INPUT VALIDATION                  safe line input, integer parsing, path checks
  ↓
OPERATION MODULE                  file_operations / directory_operations /
                                  metadata / navigation
  ↓
LINUX/POSIX API                   open, read, write, close, lseek, stat,
                                  opendir, readdir, mkdir, rmdir, getcwd, chdir
  ↓
SYSTEM CALL / KERNEL              syscall trap → VFS → ext4/tmpfs driver
  ↓
FILE SYSTEM                       stores data, inodes, permissions, timestamps
  ↓
RESULT / ERROR                    return value + errno
  ↓
COMMAND-LINE INTERFACE            [SUCCESS] / [ERROR] + strerror(reason)
```

The loop returns to the menu after every operation; only option 13 (Exit)
exits the program, closing all resources first.

## I. Methodology

1. **Requirement analysis** — the ten mandated POSIX APIs and eleven menu
   features were mapped to modules.
2. **Modular design** — each module owns one OS concept and exposes a small
   header interface; `utils.c` centralises input and error reporting.
3. **Viva-first implementation** — every major system call is preceded by a
   comment block explaining *what / why / parameters / return / failure /
   kernel interaction*.
4. **Defensive programming** — bounded `fgets()` input, integer parsing with
   `strtol`, partial-read/write loops, `errno` capture before any other call,
   and unconditional resource cleanup.
5. **Verification** — zero-warning compilation, then a scripted end-to-end
   run covering 16 test scenarios plus 15 extra checks (robustness and the
   file-copy/rename extension, options 11–12); results recorded in
   `tests/test_cases.md`.

## J. APIs / System Calls Used

| API | Module | Description |
|-----|--------|-------------|
| `open()` | file_operations | open/create; returns a file descriptor (`O_CREAT\|O_WRONLY\|O_TRUNC`, mode 0644) |
| `read()` | file_operations | read bytes at current offset; 0 = EOF; partial reads looped; `EINTR` retried |
| `write()` | file_operations | write bytes; partial writes looped |
| `close()` | file_operations | release descriptor; checked on every path |
| `lseek()` | file_operations | move file offset (`SEEK_SET`); negative input rejected before the call |
| `fstat()` | file_operations | metadata of an open file by descriptor; gives the copy its source mode and rejects directories |
| `rename()` | file_operations | atomic directory-entry move for option 12 (file or directory, target replaced) |
| `stat()` / `lstat()` | metadata, dir listing | fetch `struct stat`; `lstat` used for listings so symlinks appear as links |
| `opendir()` / `readdir()` / `closedir()` | directory_operations | directory stream; `errno` pre-zeroed to distinguish EOF from error |
| `mkdir()` | directory_operations | create directory, mode 0755 (umask-filtered) |
| `rmdir()` | directory_operations | remove empty directory (ENOTEMPTY explained) |
| `getcwd()` / `chdir()` | navigation | query / change working directory |
| `errno`, `perror()`, `strerror()` | utils | error capture and human-readable reporting |
| `S_ISREG/S_ISDIR/S_ISLNK/S_ISCHR/S_ISBLK/S_ISFIFO/S_ISSOCK` | metadata | file-type classification from `st_mode` |

## K. Module Description

| Module | Responsibility |
|--------|----------------|
| `main.c` | banner, menu loop, choice validation, dispatch, exit |
| `file_operations.c` | create/read/write files, file-descriptor demo, `lseek()` random access |
| `directory_operations.c` | listing (`opendir`/`readdir`), `mkdir`, `rmdir` |
| `metadata.c` | `stat`/`lstat`, type tags, permission string, timestamps, inode, owner |
| `navigation.c` | `getcwd`, `chdir` and related messaging |
| `utils.c` | bounded line input, integer parsing, banners, `[SUCCESS]/[ERROR]/[NOTICE]` output, `print_errno()` |

## L. Implementation Explanation

- **File descriptors:** every successful `open()` prints
  `File opened successfully. File Descriptor: N`; code comments explain that
  0/1/2 are stdin/stdout/stderr and the kernel hands out the lowest free
  number — no hard-coded values are assumed.
- **Reading:** a loop calls `read()` into a bounded buffer, accumulating the
  total; `0` on the first call means an empty file (`[NOTICE] File is empty`),
  `-1` with `EISDIR` is reported as "Is a directory".
- **Writing:** the user's line is written with a partial-write loop; the
  success message reflects that the write loop completed.
- **lseek:** the requested offset is parsed with `strtol` and rejected if
  negative; after a successful `SEEK_SET` the new offset is printed and the
  remaining content is read so the position change is *visible*; offsets past
  EOF are legal and produce a `[NOTICE]` with 0 bytes read.
- **Metadata:** `stat()` fills the table (name, type via `S_IS*` macros,
  `-rw-r--r--`-style permission string built from `st_mode`, size, inode,
  hard links, owner/group names via `getpwuid`/`getgrgid`, and
  atime/mtime/ctime formatted with `strftime`).
- **Listing:** `opendir()` + `readdir()` iterate entries, `.` and `..` are
  skipped, each entry's full path is built with bounds-checked `snprintf()`
  and typed via `lstat()`.
- **Error discipline:** every failing call funnels into `print_errno()`,
  which prints `[ERROR] context` plus `Reason: strerror(errno)`.
- **File copy (option 11):** a genuine kernel-level copy — `open()` source,
  `fstat()` for type and permission bits, `open()` destination with the same
  mode, `read()`/`write()` loop with partial-transfer and `EINTR` handling,
  then `close()` of both descriptors (the destination's `close()` is checked
  because it can surface deferred write errors).
- **Rename (option 12):** a single atomic `rename()` syscall — no data is
  copied; identical source/destination and empty inputs are rejected before
  the call.

## M. TestingSixteen required scenarios (startup, valid/invalid listing, create file,
create existing file, read valid/missing, write, create dir, create existing
 dir, remove empty/non-empty dir, metadata, lseek, invalid input, exit), five
extra robustness checks (non-empty listing, negative offset, beyond-EOF
offset, `read()` on a directory, overwrite) and ten feature-extension checks
for the file copy (option 11) and rename (option 12) additions were executed
as a scripted session under GCC 13.3 / Ubuntu 24.04. **All passed**; the full
evidence is in [`../tests/test_cases.md`](../tests/test_cases.md). The build
itself is warning-free under `-std=c11 -Wall -Wextra -Wpedantic -O2`.

## N. Expected Outcome

A working explorer that (1) performs all eleven menu operations through real
system calls, (2) prints file descriptors, metadata, permission strings and
timestamps for any path, (3) converts kernel error codes into readable
messages for every invalid input, and (4) serves as a self-documenting
reference for viva preparation on files, directories, descriptors and the
system-call interface.

## O. Conclusion

The project demonstrates, in a single coherent program, how user-space C
code communicates with the Linux kernel: requests enter through the POSIX
API, are validated by the kernel, executed on the file system, and returned
as values or `errno`-tagged errors. Building every feature directly on
`open/read/write/close/lseek/stat/opendir/readdir/mkdir/rmdir` — with strict
checking of every return value — converts abstract OS lecture topics into
observable behaviour, exactly as intended by the course objective.

## P. Future Enhancements

Deliberately excluded from the core to keep the API focus; listed for later
work: file copy and rename, recursive directory listing, file search,
permission modification (`chmod`), symbolic-link information display, disk
usage reporting, command history and an ncurses-based interface.

## Q. Viva Questions and Answers

The full set of 20 viva questions and answers is maintained in the main
[`../README.md`](../README.md#viva-questions-and-answers) (What is a system
call; file descriptors; `open()` vs `fopen()`; `read()`/`write()` semantics
and partial transfers; why `close()` is required; `lseek()`; `stat()` and
inodes; files vs directories; permission bits and umask; `opendir()`/
`readdir()`; `mkdir()`/`rmdir()` and ENOTEMPTY; `errno`; `perror()`;
failure handling; the kernel; and the user→kernel interaction path).
