#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

/*
 * file_operations.h
 * -----------------
 * Public interface of the FILE OPERATIONS module.
 *
 * This module demonstrates the low-level (unbuffered) POSIX file I/O
 * system calls:  open(), read(), write(), close() and lseek().
 * It also shows how a file copy is assembled from open/read/write
 * and how rename() moves a directory entry atomically in the kernel.
 *
 * Every function is safe to call with any user input: each one performs
 * its own validation, reports errors through the terminal, and NEVER
 * crashes on invalid input.
 */

/* Menu option numbers handled by this module (dispatched from main.c). */
#define MENU_CREATE_FILE   4
#define MENU_READ_FILE     5
#define MENU_WRITE_FILE    6
#define MENU_LSEEK_DEMO    10
#define MENU_COPY_FILE    11
#define MENU_RENAME_FILE  12

/* Menu action: Option 4 - create a new empty file with open(O_CREAT). */
void op_create_file(void);

/* Menu action: Option 5 - display a file's contents with open/read/close. */
void op_read_file(void);

/* Menu action: Option 6 - append user text to a file with open/write/close. */
void op_write_file(void);

/* Menu action: Option 10 - move the file offset with lseek() and read. */
void op_lseek_demo(void);

/* Menu action: Option 11 - copy a file with open/read/write/close. */
void op_copy_file(void);

/* Menu action: Option 12 - rename a file/directory with rename(). */
void op_rename_file(void);

#endif /* FILE_OPERATIONS_H */
