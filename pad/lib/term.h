#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
# define PAD_TERM__WINDOWS
#endif

#ifdef PAD_TERM__WINDOWS
# include <windows.h>
# include <wincon.h>
#endif

enum {
    PAD_TERM__NULL,
    
    // fg, bg
    PAD_TERM__BLACK,
    PAD_TERM__RED,
    PAD_TERM__GREEN,
    PAD_TERM__YELLOW,
    PAD_TERM__BLUE,
    PAD_TERM__MAGENTA,
    PAD_TERM__CYAN,
    PAD_TERM__GRAY,
    PAD_TERM__WHITE,
    PAD_TERM__DEFAULT,

    // opt
    PAD_TERM__UNDER,
    PAD_TERM__BRIGHT,
    PAD_TERM__REVERSE,
};

#define PadTerm_CPrintf(fg, bg, opt, fmt, ...) PadTerm_CFPrintf(stdout, fg, bg, opt, fmt, __VA_ARGS__)

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
PadTerm_CFPrintf(FILE *fout, int fg, int bg, int opt, const char *fmt, ...);

