#ifndef TERM_H
#define TERM_H

#include "define.h"
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdbool.h>

#if defined(_CAP_WINDOWS)
#  include "windows.h"
#endif

/**
 * Wrapper of fflush(stdout)
 */
void
term_flush(void);

/**
 * Wrapper of fflush(stderr)
 */
void
term_eflush(void);

/**
 * Wrapper of fputc(c, stdout)
 */
int
term_putc(int ch);

/**
 * Wrapper of fputc(c, stderr)
 */
int
term_eputc(int ch);

/**
 * Wrapper of fprintf(stdout, fmt, ...)
 */
int
term_printf(char const* fmt, ...);

/**
 * Wrapper of fprintf(stderr, fmt, ...)
 */
int
term_eprintf(char const* fmt, ...);

/**
 * Wrapper of fprintf(stdout, fmt, ...)
 * And print newline
 */
int
term_putsf(char const* fmt, ...);

/**
 * Wrapper of fprintf(stderr, fmt, ...)
 * And print newline
 */
int
term_eputsf(char const* fmt, ...);

/********************
* term color family *
********************/

typedef enum {
	TA_BRIGHT    = 1,
	TA_DIM       = 2,
	TA_BLINK     = 3,
	TA_UNDERLINE = 4,
	TA_REVERSE   = 7,
	TA_HIDDEN    = 8,
} TermAttr;

typedef enum {
	TC_DEFAULT = -1,
	TC_BLACK   = 0,
	TC_RED     = 1,
	TC_GREEN   = 2,
	TC_YELLOW  = 3,
	TC_BLUE    = 4,
	TC_MAGENTA = 5,
	TC_CYAN    = 6,
	TC_WHITE   = 7,
} TermColor;

/**
 * Wrapper of fprintf(3) with colors
 *
 * @param[out] fout pointer to FILE for destination
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return     success to number of length of write
 * @return     failed to under of zero
 */
int
term_cfprintf(FILE* fout, TermColor fg, TermColor bg, char const* fmt, ...);

/**
 * Wrapper of fprintf(3) with colors
 *
 * @param[out] fout pointer to FILE for destination
 * @param[in]  attr attribute
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return     success to number of length of write
 * @return     failed to under of zero
 */
int
term_acfprintf(FILE* fout, TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...);

/**
 * Wrapper of fprintf(stdout) with attribute and colors
 *
 * @param[in]  attr attribute
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return
 */
int
term_acprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...);

/**
 * Wrapper of fprintf(stdout) with colors
 *
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return
 */
int
term_cprintf(TermColor fg, TermColor bg, char const* fmt, ...);

/**
 * Wrapper of fprintf(stderr) with attribute and colors
 *
 * @param[in]  attr attribute
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return
 */
int
term_aceprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...);

/**
 * Wrapper of fprintf(stderr) with colors
 *
 * @param[in]  fg   fore ground color
 * @param[in]  bg   back ground color
 * @param[in]  fmt  string of format
 * @param[in]  ...  arguments of format
 *
 * @return
 */
int
term_ceprintf(TermColor fg, TermColor bg, char const* fmt, ...);

#endif
