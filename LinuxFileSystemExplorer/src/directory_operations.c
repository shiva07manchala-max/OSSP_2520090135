#define _POSIX_C_SOURCE 200809L

/*
 * directory_operations.c
 * ----------------------
 * DIRECTORY OPERATIONS module.
 *
 * Demonstrates:
 *   opendir()  - open a directory stream (like open(), but for a
 *                directory); returns DIR* or NULL on error.
 *   readdir()  - return entries one at a time as 'struct dirent'.  The
 *                field d_ino is the inode number and d_name is the file
 *                name.  d_type may be DT_UNKNOWN on some filesystems,
 *                so portable code calls stat() when d_type is unknown.
 *   closedir() - close the directory stream (never leak it).
 *   mkdir()    - create a new directory with the given permissions
 *                (filtered by the umask).  0 on success, -1 on error.
 *   rmdir()    - remove a directory; the kernel refuses (ENOTEMPTY)
 *                unless it contains nothing but "." and "..".
 */

#include "directory_operations.h"
#include "metadata.h"       /* lstat_and_type() - shared file-type helper */
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>   /* mkdir(), stat()            */
#include <sys/types.h>  /* dev_t, ino_t, mode_t ...   */
#include <dirent.h>     /* opendir(), readdir(), closedir(), dirent  */
#include <unistd.h>     /* getcwd() */

/* Table rows are aligned by name using this width. */
#define NAME_COL_WIDTH 32

/* ----------------------------------------------------------------------
 * Option 2 - List Directory Contents (opendir + readdir + closedir)
 * ---------------------------------------------------------------------- */
void op_list_directory(void)
{
    char path[PATH_BUF_SIZE];
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    print_banner("DIRECTORY OPERATIONS - LIST CONTENTS");

    if (prompt_line("Enter directory path (Enter = current): ",
                    path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        if (getcwd(path, sizeof(path)) == NULL) {
            print_errno("Unable to get current directory.");
            return;
        }
    }

    dir = opendir(path);

    if (dir == NULL) {
        /* errno explains: ENOENT missing dir, ENOTDIR path is a file,
         * EACCES permission denied. */
        print_errno("ERROR: Unable to open directory.");
        return;
    }

    printf("Contents of: %s\n", path);
    print_separator();
    printf("%-*s %s\n", NAME_COL_WIDTH, "NAME", "TYPE");
    print_separator();

    /* readdir() returns one entry per call and NULL at the end (or on
     * error - errno tells which).  "." and ".." are real directory
     * entries; we skip them for a clean listing. */
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        char fullpath[PATH_BUF_SIZE];
        char type_buf[16];   /* must fit "DIRECTORY" + NUL */

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* Build "directory/entry" so we can stat() the entry even if
         * d_type is DT_UNKNOWN.  snprintf() returns the length the full
         * string WOULD have needed; if that does not fit, the entry is
         * skipped instead of silently truncated. */
        int needed = snprintf(fullpath, sizeof(fullpath), "%s/%s",
                              path, entry->d_name);
        if (needed < 0 || (size_t)needed >= sizeof(fullpath)) {
            msg_notice("Entry path too long - skipped.");
            continue;
        }

        if (lstat_and_type(fullpath, type_buf, sizeof(type_buf)) == 0) {
            snprintf(type_buf, sizeof(type_buf), "?    ");
        }

        printf("%-*s %s\n", NAME_COL_WIDTH, entry->d_name, type_buf);
        count++;
    }

    if (errno != 0) {
        /* readdir() signaled an error, not end-of-directory. */
        print_errno("Error while reading directory entries.");
        closedir(dir);
        return;
    }

    print_separator();
    printf("Total entries: %d\n", count);
    print_separator();

    if (closedir(dir) != 0) {
        print_errno("Warning: closedir() failed.");
        return;
    }
    msg_success("Directory listed successfully.");
}

/* ----------------------------------------------------------------------
 * Option 7 - Create Directory (mkdir)
 * ----------------------------------------------------------------------
 * mkdir(path, mode) creates a directory; the permission bits are the
 * requested mode filtered by the process umask, and "." / ".." entries
 * are created automatically by the kernel.
 */
void op_create_directory(void)
{
    char path[NAME_BUF_SIZE];

    print_banner("DIRECTORY OPERATIONS - CREATE DIRECTORY");

    if (prompt_line("Enter directory name: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Directory name cannot be empty.");
        return;
    }

    /* 0755 = rwxr-xr-x (owner full, others read+traverse). */
    if (mkdir(path, 0755) != 0) {
        print_errno("Unable to create directory.");
        return;
    }

    msg_success("Directory created successfully.");
}

/* ----------------------------------------------------------------------
 * Option 8 - Remove Directory (rmdir)
 * ---------------------------------------------------------------------- */
void op_remove_directory(void)
{
    char path[NAME_BUF_SIZE];

    print_banner("DIRECTORY OPERATIONS - REMOVE DIRECTORY");

    if (prompt_line("Enter directory name: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Directory name cannot be empty.");
        return;
    }

    /* Safety filter (defence in depth): rmdir() itself is safe, but we
     * politely refuse to even try on a few dangerous names. */
    if (strcmp(path, "/") == 0 || strcmp(path, ".") == 0 ||
        strcmp(path, "..") == 0) {
        msg_error("Refusing to remove \"/\", \".\" or \"..\".");
        return;
    }

    if (rmdir(path) != 0) {
        print_errno("Unable to remove directory.");
        return;
    }

    msg_success("Directory removed successfully.");
}
