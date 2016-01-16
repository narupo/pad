#ifndef TERM_H
#define TERM_H

#include <stdio.h>
#include <stdarg.h>

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

#endif

