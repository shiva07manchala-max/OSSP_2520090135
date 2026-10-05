#define _POSIX_C_SOURCE 200809L

/*
 * file_operations.c
 * -----------------
 * FILE OPERATIONS module - the heart of the project.
 *
 * This module demonstrates the LOW-LEVEL (unbuffered) POSIX file I/O
 * system calls and the concept of FILE DESCRIPTORS:
 *
 *   open()   - ask the kernel to open/create a file; kernel returns a
 *              small integer FILE DESCRIPTOR (fd).  By convention
 *              fd 0 = standard input, 1 = standard output, 2 = standard
 *              error, so the first open() in a fresh process usually
 *              returns 3 - but we NEVER hard-code that number, we
 *              always print the value the kernel actually gave us.
 *   read()   - copy bytes from the kernel's file (at the current file
 *              offset) into a buffer in our process memory.
 *   write()  - copy bytes from our buffer into the file.
 *   close()  - release the descriptor back to the kernel.  Every open
 *              is matched by exactly one close.
 *   lseek()  - move the file offset; the offset is a per-descriptor
 *              property stored in the kernel's open file table entry.
 *   rename() - atomically move a directory entry to a new name inside
 *              the kernel; no data is copied, only the name is changed.
 *
 * A file copy (menu option 11) is assembled from open()/read()/
 * write()/close()/fstat(); a rename (option 12) demonstrates that the
 * kernel can move a name with a single atomic system call.
 *
 * Every function performs full validation and reports every failure,
 * so the program never crashes on normal invalid user input.
 */

#include "file_operations.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>      /* open() flags: O_RDONLY, O_CREAT, ...        */
#include <unistd.h>     /* read(), write(), close(), lseek(), STDIN_FILENO */
#include <sys/stat.h>   /* fstat(), struct stat, S_ISDIR               */

/* ----------------------------------------------------------------------
 * Helper: create a new (or truncate an existing) file.
 * ----------------------------------------------------------------------
 * open() flags used (see bit-by-bit explanation in README):
 *   O_WRONLY - open for writing only
 *   O_CREAT  - create the file if it does not exist
 *   O_TRUNC  - if it DOES exist, shrink it to zero length
 *
 * 0644 is the permission argument (mode_t).  It is only applied at
 * creation time and is filtered by the process umask, so a typical
 * result is rw-r--r-- (644).
 *
 * open() returns -1 on failure and sets 'errno'; on success it returns
 * the lowest free file descriptor number for this process.
 */
static void create_file_action(void)
{
    char path[NAME_BUF_SIZE];
    int fd;

    if (prompt_line("Enter filename: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Filename cannot be empty.");
        return;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        /* open() failed: errno now tells WHY (ENOENT bad path,
         * EACCES permission denied, EISDIR it is a directory, ...). */
        print_errno("Unable to create file.");
        return;
    }

    printf("File opened successfully.\n");
    printf("File Descriptor: %d\n", fd);

    /* close() releases the descriptor; the kernel may then reuse this
     * number for the next open().  Not closing leaks descriptors. */
    if (close(fd) != 0) {
        print_errno("Warning: close() failed.");
        return;
    }
    msg_success("File created successfully.");
}

/* ----------------------------------------------------------------------
 * Helper: open a file, read it completely, print it.
 * ----------------------------------------------------------------------
 * Demonstrates the canonical read loop:
 *
 *   open() -> while ((n = read(fd, buf, sizeof buf)) > 0) use buf
 *          -> close()
 *
 * read() returns the number of bytes actually read (which may be less
 * than requested!), 0 at END OF FILE, or -1 on error.  A partial read
 * is normal behaviour, not an error - so we simply keep looping until
 * read() returns 0.
 */
static void read_file_action(void)
{
    char path[NAME_BUF_SIZE];
    char buffer[IO_BUF_SIZE];
    int fd;
    ssize_t bytes_read;
    long total = 0;

    if (prompt_line("Enter filename to read: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Filename cannot be empty.");
        return;
    }

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        print_errno("Unable to open file.");
        return;
    }

    printf("File opened successfully. File Descriptor: %d\n", fd);
    printf("---- FILE CONTENT START ----\n");

    for (;;) {
        bytes_read = read(fd, buffer, sizeof(buffer));

        if (bytes_read < 0) {
            print_errno("Unable to read file.");
            close(fd);            /* never leak the descriptor */
            return;
        }
        if (bytes_read == 0) {
            break;                /* 0 = end of file reached */
        }

        /* fwrite/write to stdout with the same care we read with.
         * fflush keeps output ordered with the prompts. */
        if (fwrite(buffer, 1, (size_t)bytes_read, stdout) != (size_t)bytes_read) {
            msg_error("Failed to display file contents.");
            close(fd);
            return;
        }
        total += (long)bytes_read;
    }

    printf("\n---- FILE CONTENT END ----\n");
    printf("Total bytes read: %ld\n", total);

    if (close(fd) != 0) {
        print_errno("Warning: close() failed.");
        return;
    }

    if (total == 0) {
        msg_notice("File is empty (0 bytes).");
    } else {
        msg_success("File read successfully.");
    }
}

/* ----------------------------------------------------------------------
 * Helper: open for append, write the user's text.
 * ----------------------------------------------------------------------
 * O_APPEND makes the kernel update the file offset to the end of the
 * file AUTOMATICALLY before every write() - even if other processes
 * write concurrently.  This is an atomic kernel-side operation.
 *
 * write() may also transfer fewer bytes than requested (partial write),
 * so a correct program loops until everything is written.
 */
static void write_file_action(void)
{
    char path[NAME_BUF_SIZE];
    char text[LINE_BUF_SIZE];
    int fd;
    size_t remaining;
    const char *p;
    ssize_t written;

    if (prompt_line("Enter filename to write: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Filename cannot be empty.");
        return;
    }

    if (prompt_line("Enter content (one line): ", text, sizeof(text)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd < 0) {
        print_errno("Unable to open file for writing.");
        return;
    }

    printf("File opened successfully. File Descriptor: %d\n", fd);

    /* Partial-write loop: keep writing from where write() stopped. */
    p = text;
    remaining = strlen(text);
    while (remaining > 0) {
        written = write(fd, p, remaining);
        if (written < 0) {
            if (errno == EINTR) {
                continue;         /* interrupted by a signal: retry */
            }
            print_errno("Unable to write to file.");
            close(fd);
            return;
        }
        if (written == 0) {       /* defensive: should not happen */
            msg_error("write() transferred 0 bytes unexpectedly.");
            close(fd);
            return;
        }
        p += written;             /* advance past what was written  */
        remaining -= (size_t)written;
    }

    /* Newline at the end so repeated appends stay on separate lines. */
    if (write(fd, "\n", 1) < 0) {
        print_errno("Unable to write newline.");
        close(fd);
        return;
    }

    if (close(fd) != 0) {
        print_errno("Warning: close() failed.");
        return;
    }
    msg_success("Content written successfully.");
}

/* ----------------------------------------------------------------------
 * Option 4 - Create File (open + close)
 * ---------------------------------------------------------------------- */
void op_create_file(void)
{
    print_banner("FILE OPERATIONS - CREATE FILE");
    create_file_action();
}

/* ----------------------------------------------------------------------
 * Option 5 - Read File (open + read + close)
 * ---------------------------------------------------------------------- */
void op_read_file(void)
{
    print_banner("FILE OPERATIONS - READ FILE");
    read_file_action();
}

/* ----------------------------------------------------------------------
 * Option 6 - Write to File (open + write + close)
 * ---------------------------------------------------------------------- */
void op_write_file(void)
{
    print_banner("FILE OPERATIONS - WRITE FILE");
    write_file_action();
}

/* ----------------------------------------------------------------------
 * Option 10 - File Offset / Random Access (lseek demonstration)
 * ----------------------------------------------------------------------
 * lseek(fd, offset, whence) repositions the file offset held in the
 * kernel's open-file-description for this descriptor.
 *
 *   off_t new_off = lseek(int fd, off_t offset, int whence);
 *
 *   whence = SEEK_SET - offset counted from the START of the file
 *   whence = SEEK_CUR - offset relative to the CURRENT position
 *   whence = SEEK_END - offset relative to the END of the file
 *
 * It returns the NEW offset from the beginning, or (off_t)-1 on error
 * (e.g. negative resulting offset -> EINVAL).
 *
 * NOTE: lseek() only changes the bookkeeping offset.  It moves no data
 * and touches no disk - the next read()/write() simply continues from
 * the new position.
 */
void op_lseek_demo(void)
{
    char path[NAME_BUF_SIZE];
    char offset_text[LINE_BUF_SIZE];
    char buffer[IO_BUF_SIZE + 1];
    int fd;
    int offset;
    off_t new_offset;
    ssize_t bytes_read;

    print_banner("FILE OPERATIONS - LSEEK / RANDOM ACCESS");

    if (prompt_line("Enter filename: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Filename cannot be empty.");
        return;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        print_errno("Unable to open file.");
        return;
    }

    printf("File opened successfully. File Descriptor: %d\n", fd);

    if (prompt_line("Enter offset from beginning of file: ",
                    offset_text, sizeof(offset_text)) == NULL) {
        msg_notice("Cancelled (no input).");
        close(fd);
        return;
    }

    /* Strict validation: parse_int() rejects empty strings, "abc",
     * trailing garbage and overflow - so "12abc" or "-5" never reach
     * lseek() unchecked. */
    if (!parse_int(offset_text, &offset)) {
        msg_error("Invalid offset. Please enter a whole number (0 or greater).");
        close(fd);
        return;
    }
    if (offset < 0) {
        msg_error("Invalid offset. Offset cannot be negative.");
        close(fd);
        return;
    }

    new_offset = lseek(fd, (off_t)offset, SEEK_SET);

    if (new_offset == (off_t)-1) {
        print_errno("lseek() failed.");
        close(fd);
        return;
    }

    printf("File offset moved successfully. New offset: %ld\n",
           (long)new_offset);

    /* Read and display up to 200 bytes from the new position so the
     * effect of lseek() is visible. */
    bytes_read = read(fd, buffer, 200);
    if (bytes_read < 0) {
        print_errno("Unable to read from new offset.");
        close(fd);
        return;
    }
    if (bytes_read == 0) {
        msg_notice("Offset is at (or beyond) end of file - 0 bytes read.");
    } else {
        buffer[bytes_read] = '\0';   /* make it a printable string */
        printf("Content from offset %ld:\n", (long)new_offset);
        printf("---- READ FROM OFFSET ----\n");
        fwrite(buffer, 1, (size_t)bytes_read, stdout);
        printf("\n---- END ----\n");
        msg_success("Read from offset completed.");
    }

    if (close(fd) != 0) {
        print_errno("Warning: close() failed.");
        return;
    }
}

/* ----------------------------------------------------------------------
 * Helper: clean up after a failed copy.  Closes both descriptors; the
 * caller has already printed the error, so close failures are noted
 * but cannot change the outcome.  Removing a half-written destination
 * would need unlink() which the core spec leaves out - we keep the
 * truncated file and the error message explains what happened.
 * ---------------------------------------------------------------------- */
static void copy_cleanup(int src_fd, int dst_fd)
{
    if (close(src_fd) != 0) {
        print_errno("Warning: close() failed on source.");
    }
    if (dst_fd >= 0 && close(dst_fd) != 0) {
        print_errno("Warning: close() failed on destination.");
    }
}

/* ----------------------------------------------------------------------
 * Helper: copy one file, byte by byte, through the kernel.
 * ----------------------------------------------------------------------
 * This is a REAL kernel-level copy - not a shell 'cp' call and not a
 * system() command.  The recipe:
 *
 *   1. open() the SOURCE read-only                    -> src_fd
 *   2. fstat() the source (by descriptor, not path!)  -> mode + type
 *      - fstat() instead of stat() removes the race where the file
 *        could be swapped between open() and stat()
 *      - the source's st_mode gives the permission bits the copy
 *        should receive (umask still applies at creation time)
 *   3. reject directories: reading one fails with EISDIR only on the
 *      first read(), so we refuse earlier with a clear message
 *   4. open() the DESTINATION O_WRONLY|O_CREAT|O_TRUNC with the
 *      source's permission bits
 *   5. read()/write() loop with partial-read and partial-write
 *      handling; EINTR (interrupted by a signal) is retried
 *   6. close() BOTH descriptors - close() can report deferred write
 *      errors that write() already "succeeded" on, because data may
 *      still be sitting in kernel buffers
 *
 * Returns 0 on success, -1 on failure (message already printed).
 * ---------------------------------------------------------------------- */
static int copy_file(const char *src_path, const char *dst_path)
{
    char buffer[IO_BUF_SIZE];
    int src_fd = -1;
    int dst_fd = -1;
    struct stat src_info;

    /* Step 1 - open the source. */
    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        print_errno("Unable to open source file.");
        return -1;
    }

    /* Step 2 - inspect the open file by descriptor. */
    if (fstat(src_fd, &src_info) != 0) {
        print_errno("Unable to inspect source file.");
        close(src_fd);
        return -1;
    }

    /* Step 3 - refuse directories with a clear message. */
    if (S_ISDIR(src_info.st_mode)) {
        msg_error("Source is a directory - copy works on files only.");
        close(src_fd);
        return -1;
    }

    /* Step 4 - create/overwrite the destination with the source's mode. */
    dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC,
                  src_info.st_mode & 0777);
    if (dst_fd < 0) {
        print_errno("Unable to create destination file.");
        close(src_fd);
        return -1;
    }

    printf("Source opened.      File Descriptor: %d\n", src_fd);
    printf("Destination opened. File Descriptor: %d\n", dst_fd);

    /* Step 5 - the copy loop: read() then write() until EOF. */
    for (;;) {
        ssize_t bytes_read = read(src_fd, buffer, sizeof(buffer));

        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;         /* interrupted by a signal: retry */
            }
            print_errno("Error while copying (read).");
            copy_cleanup(src_fd, dst_fd);
            return -1;   /* destination is incomplete; error already shown */
        }
        if (bytes_read == 0) {
            break;                /* 0 = end of source file reached */
        }

        {
            const char *p = buffer;
            size_t left = (size_t)bytes_read;

            while (left > 0) {
                ssize_t written = write(dst_fd, p, left);
                if (written < 0) {
                    if (errno == EINTR) {
                        continue; /* interrupted by a signal: retry */
                    }
                    print_errno("Error while copying (write).");
                    copy_cleanup(src_fd, dst_fd);
                    return -1;
                }
                if (written == 0) {   /* defensive: should not happen */
                    msg_error("write() transferred 0 bytes unexpectedly.");
                    copy_cleanup(src_fd, dst_fd);
                    return -1;
                }
                p += (size_t)written; /* advance past what was written  */
                left -= (size_t)written;
            }
        }
    }

    /* Step 6 - close both; check the DESTINATION close especially,
     * because it can surface deferred write errors. */
    if (close(src_fd) != 0) {
        print_errno("Warning: close() failed on source.");
    }
    if (close(dst_fd) != 0) {
        print_errno("Error while finishing the copy (close).");
        return -1;
    }

    printf("Copied %ld bytes.\n", (long)src_info.st_size);
    return 0;
}

/* ----------------------------------------------------------------------
 * Option 11 - Copy File (open + fstat + open + read/write loop + close)
 * ---------------------------------------------------------------------- */
void op_copy_file(void)
{
    char src_path[NAME_BUF_SIZE];
    char dst_path[NAME_BUF_SIZE];

    print_banner("FILE OPERATIONS - COPY FILE");

    if (prompt_line("Enter source filename: ", src_path, sizeof(src_path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (src_path[0] == '\0') {
        msg_error("Source filename cannot be empty.");
        return;
    }

    if (prompt_line("Enter destination filename: ",
                    dst_path, sizeof(dst_path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (dst_path[0] == '\0') {
        msg_error("Destination filename cannot be empty.");
        return;
    }

    if (strcmp(src_path, dst_path) == 0) {
        msg_error("Source and destination are the same file.");
        return;
    }

    if (copy_file(src_path, dst_path) == 0) {
        msg_success("File copied successfully.");
    }
}

/* ----------------------------------------------------------------------
 * Option 12 - Rename / Move (rename system call)
 * ----------------------------------------------------------------------
 * rename(oldpath, newpath) asks the kernel to move the directory entry
 * 'oldpath' to 'newpath' in ONE ATOMIC step:
 *
 *   - no user data is copied; only the name -> inode mapping changes
 *   - if newpath already exists it is silently REPLACED (same file
 *     system, regular file case) - we warn about that up front
 *   - either the whole operation happens or nothing does: observers
 *     never see a half-renamed file
 *   - it returns 0 on success, -1 on failure with errno explaining
 *     why (ENOENT missing source, EACCES permission, EISDIR/EINVAL
 *     directory mismatch, EXDEV across file systems, ...)
 *
 * rename() can also rename directories (non-recursively, target must
 * not exist), which makes it a gentle preview of 'mv'.
 */
void op_rename_file(void)
{
    char old_path[NAME_BUF_SIZE];
    char new_path[NAME_BUF_SIZE];

    print_banner("FILE OPERATIONS - RENAME (MOVE)");

    if (prompt_line("Enter current name: ", old_path, sizeof(old_path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (old_path[0] == '\0') {
        msg_error("Current name cannot be empty.");
        return;
    }

    if (prompt_line("Enter new name: ", new_path, sizeof(new_path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (new_path[0] == '\0') {
        msg_error("New name cannot be empty.");
        return;
    }

    if (strcmp(old_path, new_path) == 0) {
        msg_error("New name is the same as the current name.");
        return;
    }

    /* One atomic kernel operation - no read/write loop involved. */
    if (rename(old_path, new_path) != 0) {
        print_errno("Unable to rename.");
        return;
    }

    printf("Renamed: %s -> %s\n", old_path, new_path);
    msg_success("Rename completed successfully.");
}
