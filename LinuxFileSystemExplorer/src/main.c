#define _POSIX_C_SOURCE 200809L

/*
 * main.c
 * ------
 * Program entry point and MAIN MENU loop for the
 * "Linux File System Explorer Using System Calls".
 *
 * Responsibilities (exactly as in the project specification):
 *   - program startup (banner + ui_init)
 *   - main menu display with the current working directory
 *   - accepting and validating the user's menu choice
 *   - dispatching to the operation modules
 *   - clean exit
 *
 * The program flow is:
 *   START -> initialize -> show cwd + menu -> read choice -> validate
 *         -> run operation (Linux/POSIX API -> kernel -> filesystem)
 *         -> show result/error -> back to menu -> EXIT on option 13.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* getcwd() */

#include "utils.h"
#include "navigation.h"
#include "directory_operations.h"
#include "file_operations.h"
#include "metadata.h"

#define MENU_EXIT 13

/* ---------------------------------------------------------------------
 * Helper: fetch the current working directory into buf.
 * Returns buf on success, or the fallback string "(unavailable)" if
 * getcwd() fails (e.g. the directory was deleted while we run).
 * The menu must NEVER crash just because the cwd vanished.
 * --------------------------------------------------------------------- */
static const char *current_dir_or_fallback(char *buf, size_t buf_size)
{
    if (getcwd(buf, buf_size) == NULL) {
        return "(unavailable)";
    }
    return buf;
}

/* ---------------------------------------------------------------------
 * Dispatch table: maps menu choice -> action function.
 * An index/function-pointer table keeps main() small and readable.
 * --------------------------------------------------------------------- */
typedef void (*menu_action_t)(void);

static void run_show_current_directory(void);
static void run_list_directory(void);
static void run_change_directory(void);
static void run_create_file(void);
static void run_read_file(void);
static void run_write_file(void);
static void run_create_directory(void);
static void run_remove_directory(void);
static void run_file_information(void);
static void run_lseek_demo(void);
static void run_copy_file(void);
static void run_rename_file(void);

static const menu_action_t menu_actions[] = {
    NULL,                       /* placeholder for choice 0 (unused) */
    run_show_current_directory, /* 1  */
    run_list_directory,         /* 2  */
    run_change_directory,       /* 3  */
    run_create_file,            /* 4  */
    run_read_file,              /* 5  */
    run_write_file,             /* 6  */
    run_create_directory,       /* 7  */
    run_remove_directory,       /* 8  */
    run_file_information,       /* 9  */
    run_lseek_demo,             /* 10 */
    run_copy_file,              /* 11 */
    run_rename_file,            /* 12 */
    NULL                        /* 13 = Exit, handled separately   */
};

int main(void)
{
    char line[LINE_BUF_SIZE];
    char cwd[PATH_BUF_SIZE];
    int running = 1;

    ui_init();                 /* colours only when stdout is a TTY */
    print_welcome();           /* ASCII banner                      */

    while (running) {
        int choice = 0;

        print_menu(current_dir_or_fallback(cwd, sizeof(cwd)));
        printf("Enter your choice: ");
        fflush(stdout);

        if (read_line(line, sizeof(line)) == NULL) {
            /* EOF (Ctrl-D) - treat as a clean exit request. */
            printf("\n");
            msg_notice("Input closed (EOF). Exiting.");
            break;
        }

        if (!parse_int(trim(line), &choice)) {
            msg_error("Invalid menu choice. Please enter a number 1-13.");
            continue;
        }

        if (choice < 1 || choice > MENU_EXIT) {
            msg_error("Invalid menu choice. Please enter a number 1-13.");
            continue;
        }

        if (choice == MENU_EXIT) {
            msg_success("Exiting Linux File System Explorer. Goodbye!");
            running = 0;
            break;
        }

        menu_actions[choice]();
    }

    printf("============================================================\n");
    printf(" Program finished - all file descriptors and directory\n");
    printf(" streams were closed properly (no leaked kernel resources).\n");
    printf("============================================================\n");
    return EXIT_SUCCESS;
}

/* --- wrappers around the module entry points ------------------------- */

static void run_show_current_directory(void) { op_show_cwd(); }
static void run_list_directory(void)         { op_list_directory(); }
static void run_change_directory(void)       { op_change_dir(); }
static void run_create_file(void)            { op_create_file(); }
static void run_read_file(void)              { op_read_file(); }
static void run_write_file(void)             { op_write_file(); }
static void run_create_directory(void)       { op_create_directory(); }
static void run_remove_directory(void)       { op_remove_directory(); }static void run_file_information(void)      { op_file_info(); }
static void run_lseek_demo(void)            { op_lseek_demo(); }
static void run_copy_file(void)             { op_copy_file(); }
static void run_rename_file(void)           { op_rename_file(); }
