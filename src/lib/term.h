#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
# define TERM_WINDOWS
#endif

enum {
    TERM_NULL,
    
    // fg, bg
    TERM_BLACK,
    TERM_RED,
    TERM_GREEN,
    TERM_YELLOW,
    TERM_BLUE,
    TERM_MAGENTA,
    TERM_CYAN,
    TERM_GRAY,
    TERM_WHITE,
    TERM_DEFAULT,

    // opt
    TERM_UNDER,
    TERM_BRIGHT,
    TERM_REVERSE,
};

#define term_cprintf(fg, bg, opt, fmt, ...) term_cfprintf(stdout, fg, bg, opt, fmt, __VA_ARGS__)

/**
 * print format with color at stream
 *
 * @param[in] *fout destination stream
 * @param[in] fg    fore ground color
 * @param[in] bg    back ground color
 * @param[in] opt   optional attributes
 * @param[in] *fmt  format
 * @param[in] ...   arguments
 *
 * @return print length
 */
int 
term_cfprintf(FILE *fout, int fg, int bg, int opt, const char *fmt, ...);

