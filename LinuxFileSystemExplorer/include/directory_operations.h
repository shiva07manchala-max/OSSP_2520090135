#ifndef DIRECTORY_OPERATIONS_H
#define DIRECTORY_OPERATIONS_H

/*
 * directory_operations.h
 * ----------------------
 * Public interface of the DIRECTORY OPERATIONS module.
 *
 * This module demonstrates directory handling with opendir()/readdir()/
 * closedir(), and directory create/remove with mkdir()/rmdir().
 *
 * Every function is safe to call with any user input: each one performs
 * its own validation, reports errors through the terminal, and NEVER
 * crashes on invalid input.
 */

/* Menu option numbers handled by this module (dispatched from main.c). */
#define MENU_LIST_DIR      2
#define MENU_CREATE_DIR    7
#define MENU_REMOVE_DIR    8

/* Menu action: Option 2 - list entries of a directory (opendir/readdir). */
void op_list_directory(void);

/* Menu action: Option 7 - create a new directory (mkdir). */
void op_create_directory(void);

/* Menu action: Option 8 - remove an EMPTY directory (rmdir). */
void op_remove_directory(void);

#endif /* DIRECTORY_OPERATIONS_H */
