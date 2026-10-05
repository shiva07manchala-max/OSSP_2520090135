#ifndef METADATA_H
#define METADATA_H

/*
 * metadata.h
 * ----------
 * Public interface of the FILE METADATA module.
 *
 * This module demonstrates stat() (follows symbolic links) and lstat()
 * (does not follow symbolic links).  The kernel describes every file
 * through 'struct stat', which contains the file type, size, inode
 * number, permission bits and three timestamps (atime, mtime, ctime).
 *
 * Every function is safe to call with any user input: it performs its
 * own validation, reports errors through the terminal, and NEVER
 * crashes on invalid input.
 */

/* Menu option number handled by this module (dispatched from main.c). */
#define MENU_FILE_INFO     9

/* Menu action: Option 9 - show full metadata of a file (stat/lstat). */
void op_file_info(void);

/* Helper (also used by directory_operations.c while listing):
 * Ask the user for a filename, lstat() it, and store a printable file
 * type string ("REG  ", "DIR  ", "LINK ", ...) into type_buf.
 * Returns 1 on success, 0 if the name is empty or lstat() failed
 * (an error message has already been printed in that case). */
int lstat_and_type(const char *path, char *type_buf, int type_buf_size);

#endif /* METADATA_H */
