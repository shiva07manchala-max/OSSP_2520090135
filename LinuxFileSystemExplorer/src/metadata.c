/*
 * Feature-test macro: under strict -std=c11 the compiler hides POSIX
 * extensions (lstat, readlink, S_ISSOCK ...).  Asking for POSIX.1-2008
 * exposes exactly the interfaces this project teaches.
 */
#define _POSIX_C_SOURCE 200809L

/*
 * metadata.c
 * ----------
 * FILE METADATA module - one of the most important demonstrations.
 *
 * Demonstrates stat() / lstat() and 'struct stat':
 *
 *   stat()     - asks the kernel for the inode data of a path.  If the
 *                path is a symbolic link, stat() FOLLOWS the link and
 *                describes the target.
 *   lstat()    - same, but describes the LINK ITSELF when the path is a
 *                symlink (the 'l' = link).  Used in directory listing so
 *                a link is shown as LINK, not as its target's type.
 *
 * struct stat fields used here:
 *   st_size  - file size in bytes (meaningless for directories)
 *   st_ino   - inode number in the filesystem
 *   st_mode  - packed bits: file type (S_IFMT) + permission bits
 *   st_nlink - number of hard links
 *   st_uid / st_gid - numeric owner / group
 *   st_atime - last ACCESS time
 *   st_mtime - last MODIFICATION time
 *   st_ctime - last STATUS CHANGE time (inode change, NOT "creation")
 *
 * File-type decision uses the standard macros:
 *   S_ISREG() S_ISDIR() S_ISLNK() S_ISCHR() S_ISBLK() S_ISFIFO()
 *   S_ISSOCK() - macros, because C has no typeof() to compare st_mode
 *   against S_IFREG etc. directly in a portable switch() statement.
 */

#include "metadata.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>       /* strftime(), localtime()          */
#include <sys/stat.h>   /* stat(), lstat(), struct stat     */
#include <sys/types.h>
#include <unistd.h>     /* readlink() helper for symlinks   */
#include <pwd.h>        /* uid -> user name (getpwuid)      */
#include <grp.h>        /* gid -> group name (getgrgid)     */

/* Row label column width for the aligned information table. */
#define LABEL_WIDTH 16

/* ----------------------------------------------------------------------
 * Shared helper (declared in metadata.h, used by directory listing too)
 *
 * lstat() the path and produce a short printable type tag:
 *   REG   DIR   LINK   CHR   BLK   FIFO  SOCK  ?
 * Returns 1 on success, 0 on failure (message already printed).
 * ---------------------------------------------------------------------- */
int lstat_and_type(const char *path, char *type_buf, int type_buf_size)
{
    struct stat file_stat;

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    /* lstat() = stat() but does NOT follow a final symbolic link. */
    if (lstat(path, &file_stat) != 0) {
        return 0;   /* caller decides how to report (listing continues) */
    }

    if (S_ISREG(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "FILE");
    } else if (S_ISDIR(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "DIRECTORY");
    } else if (S_ISLNK(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "SYMLINK");
    } else if (S_ISCHR(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "CHAR DEV");
    } else if (S_ISBLK(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "BLOCK DEV");
    } else if (S_ISFIFO(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "FIFO/PIPE");
    } else if (S_ISSOCK(file_stat.st_mode)) {
        snprintf(type_buf, (size_t)type_buf_size, "SOCKET");
    } else {
        snprintf(type_buf, (size_t)type_buf_size, "?");
    }
    return 1;
}

/* ----------------------------------------------------------------------
 * Helper: build "rw-r--r--" style permission string from st_mode.
 *
 * The low 9 bits of st_mode are three rwx triples:
 *   user (owner):  S_IRUSR S_IWUSR S_IXUSR
 *   group:         S_IRGRP S_IWGRP S_IXGRP
 *   others:        S_IROTH S_IWOTH S_IXOTH
 * Leading character shows the file type: d / l / c / b / p / s / -.
 * ---------------------------------------------------------------------- */
static void format_permissions(mode_t mode, char *out, size_t out_size)
{
    char type_char;

    if (S_ISDIR(mode)) {
        type_char = 'd';
    } else if (S_ISLNK(mode)) {
        type_char = 'l';
    } else if (S_ISCHR(mode)) {
        type_char = 'c';
    } else if (S_ISBLK(mode)) {
        type_char = 'b';
    } else if (S_ISFIFO(mode)) {
        type_char = 'p';
    } else if (S_ISSOCK(mode)) {
        type_char = 's';
    } else {
        type_char = '-';
    }

    snprintf(out, out_size, "%c%c%c%c%c%c%c%c%c%c",
             type_char,
             (mode & S_IRUSR) ? 'r' : '-',
             (mode & S_IWUSR) ? 'w' : '-',
             (mode & S_IXUSR) ? 'x' : '-',
             (mode & S_IRGRP) ? 'r' : '-',
             (mode & S_IWGRP) ? 'w' : '-',
             (mode & S_IXGRP) ? 'x' : '-',
             (mode & S_IROTH) ? 'r' : '-',
             (mode & S_IWOTH) ? 'w' : '-',
             (mode & S_IXOTH) ? 'x' : '-');
}

/* ----------------------------------------------------------------------
 * Helper: format a struct stat timestamp as "YYYY-MM-DD HH:MM:SS".
 *
 * localtime() converts the raw time_t (seconds since the epoch,
 * 1970-01-01 00:00:00 UTC) into calendar fields for the local zone.
 * A 64-byte buffer is ample for this fixed-format string; truncation
 * is impossible for this format, but snprintf still guards the copy.
 * ---------------------------------------------------------------------- */
static void format_time(time_t raw_time, char *out, size_t out_size)
{
    struct tm *time_info;

    time_info = localtime(&raw_time);
    if (time_info == NULL) {
        snprintf(out, out_size, "(invalid time)");
        return;
    }
    strftime(out, out_size, "%Y-%m-%d %H:%M:%S", time_info);
}

/* ----------------------------------------------------------------------
 * Helper: turn uid/gid numbers into names (fall back to the number).
 * getpwuid()/getgrgid() read /etc/passwd and /etc/group.
 * ---------------------------------------------------------------------- */
static void format_owner(uid_t uid, char *out, size_t out_size)
{
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        snprintf(out, out_size, "%s", pw->pw_name);
    } else {
        snprintf(out, out_size, "%u", (unsigned)uid);
    }
}

static void format_group(gid_t gid, char *out, size_t out_size)
{
    struct group *gr = getgrgid(gid);

    if (gr != NULL) {
        snprintf(out, out_size, "%s", gr->gr_name);
    } else {
        snprintf(out, out_size, "%u", (unsigned)gid);
    }
}

/* ----------------------------------------------------------------------
 * Option 9 - File Information / Metadata (stat + lstat)
 * ---------------------------------------------------------------------- */
void op_file_info(void)
{
    char path[NAME_BUF_SIZE];
    char perms[16];
    char time_buf[64];
    char owner[128];
    char group[128];
    const char *file_type;
    struct stat file_stat;

    print_banner("FILE INFORMATION (STAT)");

    if (prompt_line("Enter file or directory name: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("File name cannot be empty.");
        return;
    }

    /* FIRST try lstat(): it reports the link itself for symlinks. */
    if (lstat(path, &file_stat) != 0) {
        /* Second chance: if lstat failed on a dangling symlink we would
         * already have the answer, so a stat() retry is unnecessary.
         * Report the kernel's reason and stop. */
        print_errno("Unable to get file information.");
        return;
    }

    /* Human-readable file type via the standard mode macros. */
    if (S_ISREG(file_stat.st_mode)) {
        file_type = "Regular File";
    } else if (S_ISDIR(file_stat.st_mode)) {
        file_type = "Directory";
    } else if (S_ISLNK(file_stat.st_mode)) {
        file_type = "Symbolic Link";
    } else if (S_ISCHR(file_stat.st_mode)) {
        file_type = "Character Device";
    } else if (S_ISBLK(file_stat.st_mode)) {
        file_type = "Block Device";
    } else if (S_ISFIFO(file_stat.st_mode)) {
        file_type = "FIFO / Named Pipe";
    } else if (S_ISSOCK(file_stat.st_mode)) {
        file_type = "Socket";
    } else {
        file_type = "Unknown";
    }

    format_permissions(file_stat.st_mode, perms, sizeof(perms));
    format_owner(file_stat.st_uid, owner, sizeof(owner));
    format_group(file_stat.st_gid, group, sizeof(group));

    printf("%-*s: %s\n", LABEL_WIDTH, "File Name", path);
    printf("%-*s: %s\n", LABEL_WIDTH, "File Type", file_type);
    printf("%-*s: %s\n", LABEL_WIDTH, "Permissions", perms);
    printf("%-*s: %ld bytes\n", LABEL_WIDTH, "File Size", (long)file_stat.st_size);
    printf("%-*s: %lu\n", LABEL_WIDTH, "Inode", (unsigned long)file_stat.st_ino);
    printf("%-*s: %d\n", LABEL_WIDTH, "Hard Links", (int)file_stat.st_nlink);
    printf("%-*s: %s (%u)\n", LABEL_WIDTH, "Owner", owner, (unsigned)file_stat.st_uid);
    printf("%-*s: %s (%u)\n", LABEL_WIDTH, "Group", group, (unsigned)file_stat.st_gid);

    format_time(file_stat.st_atime, time_buf, sizeof(time_buf));
    printf("%-*s: %s\n", LABEL_WIDTH, "Last Accessed", time_buf);

    format_time(file_stat.st_mtime, time_buf, sizeof(time_buf));
    printf("%-*s: %s\n", LABEL_WIDTH, "Last Modified", time_buf);

    format_time(file_stat.st_ctime, time_buf, sizeof(time_buf));
    printf("%-*s: %s\n", LABEL_WIDTH, "Status Changed", time_buf);

    /* Symbolic links: show WHERE they point using readlink(). */
    if (S_ISLNK(file_stat.st_mode)) {
        char target[PATH_BUF_SIZE];
        ssize_t len = readlink(path, target, sizeof(target) - 1);

        if (len >= 0) {
            target[len] = '\0';
            printf("%-*s: -> %s\n", LABEL_WIDTH, "Points To", target);
        } else {
            print_errno("Could not read symbolic link target.");
        }
    }

    msg_success("File information displayed.");
}
