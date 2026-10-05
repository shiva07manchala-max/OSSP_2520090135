#ifndef NAVIGATION_H
#define NAVIGATION_H

/*
 * navigation.h
 * ------------
 * Public interface of the NAVIGATION module.
 *
 * This module demonstrates how a running process queries and changes
 * its own current working directory with getcwd() and chdir().
 *
 * Every function is safe to call with any user input: each one performs
 * its own validation, reports errors through the terminal, and NEVER
 * crashes on invalid input.
 */

/* Menu option numbers handled by this module (dispatched from main.c). */
#define MENU_SHOW_CWD      1
#define MENU_CHANGE_DIR    3

/* Menu action: Option 1 - print the current working directory (getcwd). */
void op_show_cwd(void);

/* Menu action: Option 3 - change the current working directory (chdir). */
void op_change_dir(void);

#endif /* NAVIGATION_H */
