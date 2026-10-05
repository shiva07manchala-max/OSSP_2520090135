#define _POSIX_C_SOURCE 200809L

/*
 * utils.c
 * -------
 * Implementation of the shared utilities used by every module.
 *
 * Contains:
 *   - overflow-proof line input (fgets based - gets() is NEVER used,
 *     because gets() cannot be made safe: it has no length limit)
 *   - strict integer parsing (strtol based)
 *   - colour handling that is safe when output is redirected to a file
 *     (escape codes are suppressed unless stdout is a terminal)
 *   - unified [SUCCESS] / [ERROR] / [NOTICE] messages
 *   - system-call error reporting with strerror(errno) and perror()
 */

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>     /* isatty() - POSIX, declares STDOUT_FILENO */

/* ----------------------------------------------------------------------
 * Colour support
 * ----------------------------------------------------------------------
 * ANSI escape sequences are the portable way to colour terminal text on
 * Linux.  They are only sent when stdout is an interactive terminal
 * (checked with isatty()).  If the program output is piped into a file
 * the codes would appear as garbage such as ^[[31m, so they are turned
 * off automatically.  The NO_COLOR convention is honoured as well.
 */
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_RED     "\033[31m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_CYAN    "\033[36m"

static int g_colors_enabled = 0;   /* set once by ui_init() */

/* Wrap text in an ANSI colour when colours are enabled, else as-is. */
static const char *c(const char *code)
{
    return g_colors_enabled ? code : "";
}

/* ----------------------------------------------------------------------
 * Safe input
 * ---------------------------------------------------------------------- */

char *read_line(char *buf, size_t buf_size)
{
    size_t len;

    if (buf == NULL || buf_size < 2) {
        return NULL;
    }

    /* fgets() reads at most (buf_size - 1) characters and always adds a
     * NUL terminator, so the buffer can never overflow.  gets() would
     * overflow - it is removed from modern C standards. */
    if (fgets(buf, (int)buf_size, stdin) == NULL) {
        return NULL;              /* EOF or read error */
    }

    len = strlen(buf);

    /* If the line did not fit, the '\n' is still in stdin: discard the
     * rest of the line so it cannot be mistaken for the next answer. */
    if (len > 0 && buf[len - 1] != '\n') {
        int ch;
        while ((ch = getchar()) != EOF && ch != '\n') {
            /* drain */
        }
    }

    /* Remove the trailing newline. */
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    /* Remove a trailing '\r' too, so input files with Windows line
     * endings behave the same as Linux input. */
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\r') {
        buf[len - 1] = '\0';
    }

    return buf;
}

char *trim(char *s)
{
    char *end;

    if (s == NULL) {
        return NULL;
    }

    /* Skip leading whitespace. */
    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return s;                 /* string was all spaces */
    }

    /* Cut trailing whitespace by placing a NUL after the last visible
     * character. */
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';

    return s;
}

char *prompt_line(const char *prompt, char *buf, size_t buf_size)
{
    printf("%s", prompt);
    fflush(stdout);               /* make sure the prompt is visible
                                     before we block on input */

    if (read_line(buf, buf_size) == NULL) {
        return NULL;              /* EOF - caller treats this as cancel */
    }

    return trim(buf);
}

int parse_int(const char *s, int *out)
{
    char *endptr;
    long value;

    if (s == NULL || out == NULL || *s == '\0') {
        return 0;                 /* empty or missing argument */
    }

    errno = 0;                    /* strtol() signals overflow via errno */
    value = strtol(s, &endptr, 10);

    if (errno == ERANGE) {
        return 0;                 /* number too big / too small for long */
    }
    if (endptr == s) {
        return 0;                 /* no digits at all ("abc") */
    }
    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr)) {
            return 0;             /* stray characters after the number */
        }
        endptr++;
    }
    if (value < INT_MIN || value > INT_MAX) {
        return 0;                 /* outside int range */
    }

    *out = (int)value;
    return 1;
}

/* ----------------------------------------------------------------------
 * Output helpers
 * ---------------------------------------------------------------------- */

void ui_init(void)
{
    const char *no_color = getenv("NO_COLOR");

    /* Colours only for interactive terminals, never for piped output,
     * and never when the user sets the NO_COLOR variable. */
    if (no_color == NULL && isatty(STDOUT_FILENO)) {
        g_colors_enabled = 1;
    } else {
        g_colors_enabled = 0;
    }
}

void print_banner(const char *title)
{
    const int width = 60;
    int pad;
    int i;

    printf("\n%s%s", c(ANSI_BOLD), c(ANSI_CYAN));
    for (i = 0; i < width; i++) {
        putchar('=');
    }
    printf("\n");

    pad = width - (int)strlen(title) - 2;
    if (pad < 1) {
        pad = 1;
    }
    printf(" %s", title);
    for (i = 0; i < pad; i++) {
        putchar(' ');
    }
    printf("\n");

    for (i = 0; i < width; i++) {
        putchar('=');
    }
    printf("%s\n", c(ANSI_RESET));
}

void print_welcome(void)
{
    printf("%s%s", c(ANSI_BOLD), c(ANSI_CYAN));
    printf("============================================================\n");
    printf("              LINUX FILE SYSTEM EXPLORER\n");
    printf("                 USING SYSTEM CALLS\n");
    printf("============================================================\n");
    printf("%s", c(ANSI_RESET));
    printf(" Language    : C (POSIX / Linux system calls)\n");
    printf("\n");
    printf(" This program talks DIRECTLY to the Linux kernel through\n");
    printf(" open() read() write() close() lseek() stat()\n");
    printf(" opendir() readdir() mkdir() rmdir()\n");
}

void print_menu(const char *cwd)
{
    printf("\n%s%s------------------------------------------------------------%s\n",
           c(ANSI_BOLD), c(ANSI_CYAN), c(ANSI_RESET));
    printf("Current Directory: %s\n", cwd);
    printf("%s%s------------------------------------------------------------%s\n",
           c(ANSI_BOLD), c(ANSI_CYAN), c(ANSI_RESET));
    /*
     * Options are ALWAYS printed in strict numerical order (1 -> 13)
     * so the menu reads top-to-bottom exactly like the numbers run.
     * Small section labels are kept for clarity but never break the
     * ascending order.
     */
    printf("%s NAVIGATION%s\n", c(ANSI_BOLD), c(ANSI_RESET));
    printf("    1. Show Current Directory\n");
    printf("    2. List Directory Contents\n");
    printf("    3. Change Directory\n");
    printf("%s FILE OPERATIONS%s\n", c(ANSI_BOLD), c(ANSI_RESET));
    printf("    4. Create File\n");
    printf("    5. Read File\n");
    printf("    6. Write to File\n");
    printf("%s DIRECTORY OPERATIONS%s\n", c(ANSI_BOLD), c(ANSI_RESET));
    printf("    7. Create Directory\n");
    printf("    8. Remove Directory\n");
    printf("%s FILE INFORMATION%s\n", c(ANSI_BOLD), c(ANSI_RESET));
    printf("    9. File Information\n");
    printf("   10. File Offset / Random Access\n");
    printf("%s ADVANCED FILE OPERATIONS%s\n", c(ANSI_BOLD), c(ANSI_RESET));
    printf("   11. Copy File\n");
    printf("   12. Rename (Move) File\n");
    printf("   13. Exit\n");
    printf("%s%s------------------------------------------------------------%s\n",
           c(ANSI_BOLD), c(ANSI_CYAN), c(ANSI_RESET));
}

void msg_success(const char *text)
{
    printf("%s[SUCCESS]%s %s\n", c(ANSI_GREEN), c(ANSI_RESET), text);
}

void msg_error(const char *text)
{
    printf("%s[ERROR]%s %s\n", c(ANSI_RED), c(ANSI_RESET), text);
}

void msg_notice(const char *text)
{
    printf("%s[NOTICE]%s %s\n", c(ANSI_YELLOW), c(ANSI_RESET), text);
}

void msg_plain(const char *text)
{
    printf("%s\n", text);
}

void print_separator(void)
{
    printf("------------------------------------------------------------\n");
}

/* ----------------------------------------------------------------------
 * System-call error reporting
 * ----------------------------------------------------------------------
 * When a system call fails it returns -1 (or NULL) and stores a small
 * integer error code in the global variable 'errno'.  strerror(errno)
 * converts that code into a human readable string such as
 * "No such file or directory".  perror() does the same job but writes
 * to stderr and prefixes the message with the text we supply.
 */

void print_errno(const char *custom_text)
{
    /* strerror() returns a pointer to a static string owned by the C
     * library - safe to print directly. */
    printf("%s[ERROR]%s %s\n", c(ANSI_RED), c(ANSI_RESET), custom_text);
    printf("Reason: %s\n", strerror(errno));
}

void print_perror(const char *custom_text)
{
    /* perror() writes "custom_text: reason" to stderr. */
    perror(custom_text);
}
