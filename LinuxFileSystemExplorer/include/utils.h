#ifndef UTILS_H
#define UTILS_H

/*
 * utils.h
 * -------
 * Public interface of the UTILITIES module.
 *
 * Everything that is shared by the other modules lives here:
 *   - safe, overflow-proof line input          (read_line)
 *   - safe integer conversion                  (parse_int)
 *   - colour helpers (auto-disabled when piped, NO_COLOR honoured)
 *   - unified [SUCCESS] / [ERROR] / [NOTICE] messages
 *   - system-call error reporting              (print_errno, print_error)
 *   - section banners / screen header
 */

#include <stddef.h>

/* ---------- Buffer size limits (protect against overflow) ---------- */

#define PATH_BUF_SIZE  4096   /* Linux PATH_MAX - roomy for any path       */
#define LINE_BUF_SIZE  1024   /* one line of user input                    */
#define NAME_BUF_SIZE   256   /* one filename / directory name             */
#define IO_BUF_SIZE    4096   /* chunk size used by read()/write() loops   */

/* ---------- Safe input ------------------------------------------------ */

/* Read one line from stdin into buf (at most buf_size bytes including
 * the NUL).  The trailing '\n' is removed.  Overflowing input is safely
 * discarded.  Returns buf on success, NULL on EOF/error. */
char *read_line(char *buf, size_t buf_size);

/* Trim leading/trailing spaces, tabs and CR/LF from a string in place.
 * Returns the (possibly moved) pointer to the first non-space char. */
char *trim(char *s);

/* Print "prompt> " style prompt (no newline), flush, read one line,
 * trim it, and return the trimmed pointer into buf.
 * Returns NULL only on EOF (treated as "cancel" by callers). */
char *prompt_line(const char *prompt, char *buf, size_t buf_size);

/* Convert a whole string to an int, strictly.  Returns 1 on success and
 * stores the value in *out.  Returns 0 on: empty string, stray
 * characters, overflow.  Used for menu choices and lseek() offsets. */
int parse_int(const char *s, int *out);

/* ---------- Output helpers ------------------------------------------- */

/* Initialize colours: enabled only if stdout is a terminal and the
 * NO_COLOR environment variable is not set.  Called once from main(). */
void ui_init(void);

/* Section banner, e.g.  ================= FILE OPERATIONS ============= */
void print_banner(const char *title);

/* Draw the start-up logo exactly as in the project specification. */
void print_welcome(void);

/* Draw the main menu and the current working directory. */
void print_menu(const char *cwd);

/* ---------- Unified message helpers ----------------------------------- */

/* [SUCCESS] message in green (when colours are on). */
void msg_success(const char *text);

/* [ERROR] message in red (when colours are on). */
void msg_error(const char *text);

/* [NOTICE] message in yellow (when colours are on). */
void msg_notice(const char *text);

/* Plain line (no prefix). */
void msg_plain(const char *text);

/* ---------- System-call error reporting ------------------------------- */

/* Print "[ERROR] <custom text>" followed by the errno reason:
 *      [ERROR] Unable to open file.
 *      Reason: No such file or directory.
 * Implemented with strerror(errno) as required by the specification. */
void print_errno(const char *custom_text);

/* Same idea but prints perror()-style output on stderr:
 *      <custom_text>: No such file or directory
 * Kept to demonstrate perror() alongside strerror() (viva question 17). */
void print_perror(const char *custom_text);

/* Print a horizontal separator line of dashes. */
void print_separator(void);

#endif /* UTILS_H */
