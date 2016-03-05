#ifndef TERM_H
#define TERM_H

#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
#  include "windows.h"
#endif

// Macros for color string on terminal
// Example: printf(TERM_RED "This is red string." TERM_RESET " Reseted.");
// #if defined(_WIN32) || defined(_WIN64)
// #  define TERM_NRM     
// #  define TERM_RED     
// #  define TERM_GREEN   
// #  define TERM_YELLOW  
// #  define TERM_BLUE    
// #  define TERM_MAGENTA 
// #  define TERM_CYAN    
// #  define TERM_WHITE   
// #  define TERM_RESET  
#if 0
#else
#  define TERM_NRM     "\x1B[0m"
#  define TERM_RED     "\x1B[31m"
#  define TERM_GREEN   "\x1B[32m"
#  define TERM_YELLOW  "\x1B[33m"
#  define TERM_BLUE    "\x1B[34m"
#  define TERM_MAGENTA "\x1B[35m"
#  define TERM_CYAN    "\x1B[36m"
#  define TERM_WHITE   "\x1B[37m"
#  define TERM_RESET   "\033[0m"
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
	TA_RESET     = 0,
	TA_BRIGHT    = 1,
	TA_DIM       = 2,
	TA_BLINK     = 3,
	TA_UNDERLINE = 4,
	TA_REVERSE   = 7,
	TA_HIDDEN    = 8,
} TermAttr;

typedef enum {
	TC_BLACK   = 0,
	TC_RED     = 1,
	TC_GREEN   = 2,
	TC_YELLOW  = 3,
	TC_BLUE    = 4,
	TC_MAGENTA = 5,
	TC_CYAN    = 6,
	TC_WHITE   = 7,
} TermColor;

int
term_acprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...);

int
term_cprintf(TermColor fg, TermColor bg, char const* fmt, ...);

int
term_aceprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...);

int
term_ceprintf(TermColor fg, TermColor bg, char const* fmt, ...);

#endif

