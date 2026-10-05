#define _POSIX_C_SOURCE 200809L

/*
 * navigation.c
 * ------------
 * NAVIGATION module.
 *
 * Demonstrates how a process queries and changes its own CURRENT
 * WORKING DIRECTORY:
 *
 *   getcwd() - "get current working directory": the kernel copies the
 *              absolute path of this process's working directory into
 *              our buffer.  Returns NULL on failure (buffer too small,
 *              current directory deleted, ...) and sets errno.
 *
 *   chdir()  - "change directory": asks the kernel to make the given
 *              path the process's working directory.  Relative paths
 *              (and all relative filename lookups that follow) are then
 *              resolved from there.  Returns 0 on success, -1 on error
 *              with errno explaining why (ENOENT no such directory,
 *              ENOTDIR a path component is a file, EACCES no permission).
 *
 * IMPORTANT viva point: chdir() changes the directory of the PROCESS,
 * never of the shell that started it - each process has its own working
 * directory stored by the kernel.
 */

#include "navigation.h"
#include "utils.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>     /* getcwd(), chdir() */

/* ----------------------------------------------------------------------
 * Option 1 - Show Current Directory (getcwd)
 * ---------------------------------------------------------------------- */
void op_show_cwd(void)
{
    char cwd[PATH_BUF_SIZE];

    print_banner("CURRENT DIRECTORY");

    /* POSIX.1-2001 getcwd(buf, size): kernel writes the absolute path +
     * NUL into buf.  We pass the real size so it can never overflow. */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        print_errno("Unable to get current directory.");
        return;
    }

    printf("Current Directory:\n %s\n", cwd);
    msg_success("Current directory displayed.");
}

/* ----------------------------------------------------------------------
 * Option 3 - Change Directory (chdir)
 * ---------------------------------------------------------------------- */
void op_change_dir(void)
{
    char path[PATH_BUF_SIZE];
    char cwd[PATH_BUF_SIZE];

    print_banner("NAVIGATION - CHANGE DIRECTORY");

    if (prompt_line("Enter directory path: ", path, sizeof(path)) == NULL) {
        msg_notice("Cancelled (no input).");
        return;
    }
    if (path[0] == '\0') {
        msg_error("Path cannot be empty.");
        return;
    }

    if (chdir(path) != 0) {
        print_errno("ERROR: Unable to access directory.");
        msg_notice("Tip: use an absolute path like /home/user/Documents.");
        return;
    }

    /* Verify + display where we are now (also proves chdir worked). */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        print_errno("Directory changed, but getcwd() failed.");
        return;
    }

    msg_success("Directory changed successfully.");
    printf("Now in: %s\n", cwd);
}
