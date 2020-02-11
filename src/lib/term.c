#include "lib/term.h"

#ifdef TERM_WINDOWS
static WORD
get_attr(HANDLE handle) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(handle, &info)) {
        return -1;
    }

    return info.wAttributes;
}

static void
set_color(HANDLE handle, int fg, int bg, int opt) {
    WORD attr = 0;

    switch (fg) {
    case TERM_BLACK:
        if (opt == TERM_BRIGHT) {
            attr |= 8;
        }
        break;
    case TERM_RED:
        if (opt == TERM_BRIGHT) {
            attr |= 12;
        } else {
            attr |= 4;
        }
        break;
    case TERM_GREEN:
        if (opt == TERM_BRIGHT) {
            attr |= 10;
        } else {
            attr |= 2;
        }
        break;
    case TERM_BLUE:
        if (opt == TERM_BRIGHT) {
            attr |= 9;
        } else {
            attr |= 1;
        }
        break;
    case TERM_YELLOW: 
        if (opt == TERM_BRIGHT) {
            attr |= 14;
        } else {
            attr |= 6;
        }
        break;
    case TERM_MAGENTA:
        if (opt == TERM_BRIGHT) {
            attr |= 13;
        } else {
            attr |= 5;
        }
        break;
    case TERM_CYAN:
        if (opt == TERM_BRIGHT) {
            attr |= 11;
        } else {
            attr |= 3;
        }
        break;
    case TERM_GRAY:
        if (opt == TERM_BRIGHT) {
            attr |= 7;
        } else {
            attr |= 8;
        }
        break;
    case TERM_WHITE:
        if (opt == TERM_BRIGHT) {
            attr |= 7;
        } else {
            attr |= 15;
        }
        break;
    case TERM_DEFAULT: break;
    }
    
    switch (bg) {
    case TERM_BLACK:   break;
    case TERM_RED:     attr |= BACKGROUND_RED; break;
    case TERM_GREEN:   attr |= BACKGROUND_GREEN; break;
    case TERM_BLUE:    attr |= BACKGROUND_BLUE; break;
    case TERM_YELLOW:  attr |= BACKGROUND_RED | BACKGROUND_GREEN; break;
    case TERM_MAGENTA: attr |= BACKGROUND_RED | BACKGROUND_BLUE; break;
    case TERM_CYAN:    attr |= BACKGROUND_BLUE | BACKGROUND_INTENSITY; break;
    case TERM_GRAY:    attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
    case TERM_WHITE:   attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY; break;
    case TERM_DEFAULT: break;
    }
    
    switch (opt) {
    case TERM_BRIGHT:
        attr |= FOREGROUND_INTENSITY;
        break;
    case TERM_UNDER: attr |= COMMON_LVB_UNDERSCORE; break;
    case TERM_REVERSE: attr |= COMMON_LVB_REVERSE_VIDEO; break;
    }

    SetConsoleTextAttribute(handle, attr);
}
#endif

int
term_cfprintf(FILE *fout, int fg, int bg, int opt, const char *fmt, ...) {
#ifdef TERM_WINDOWS
    va_list ap;
    va_start(ap, fmt);

    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attr = get_attr(hstdout);
    set_color(hstdout, fg, bg, opt);
    int len = vfprintf(fout, fmt, ap);
    SetConsoleTextAttribute(hstdout, attr);

    va_end(ap);
    return len;
#else
    switch (fg) {
    case TERM_NULL: break;
    case TERM_BLACK:   fprintf(fout, "\x1b[30m"); break;
    case TERM_RED:     fprintf(fout, "\x1b[31m"); break;
    case TERM_GREEN:   fprintf(fout, "\x1b[32m"); break;
    case TERM_YELLOW:  fprintf(fout, "\x1b[33m"); break;
    case TERM_BLUE:    fprintf(fout, "\x1b[34m"); break;
    case TERM_MAGENTA: fprintf(fout, "\x1b[35m"); break;
    case TERM_CYAN:    fprintf(fout, "\x1b[36m"); break;
    case TERM_GRAY:    fprintf(fout, "\x1b[37m"); break;
    case TERM_WHITE:   fprintf(fout, "\x1b[37m"); break;
    case TERM_DEFAULT: fprintf(fout, "\x1b[39m"); break;
    }

    switch (bg) {
    case TERM_NULL: break;
    case TERM_BLACK:   fprintf(fout, "\x1b[40m"); break;
    case TERM_RED:     fprintf(fout, "\x1b[41m"); break;
    case TERM_GREEN:   fprintf(fout, "\x1b[42m"); break;
    case TERM_YELLOW:  fprintf(fout, "\x1b[43m"); break;
    case TERM_BLUE:    fprintf(fout, "\x1b[44m"); break;
    case TERM_MAGENTA: fprintf(fout, "\x1b[45m"); break;
    case TERM_CYAN:    fprintf(fout, "\x1b[46m"); break;
    case TERM_GRAY:    fprintf(fout, "\x1b[47m"); break;
    case TERM_WHITE:   fprintf(fout, "\x1b[47m"); break;
    case TERM_DEFAULT: fprintf(fout, "\x1b[49m"); break;
    }

    switch (opt) {
    case TERM_NULL: break;
    case TERM_UNDER:   fprintf(fout, "\x1b[4m"); break;
    case TERM_BRIGHT:  fprintf(fout, "\x1b[1m"); break;
    case TERM_REVERSE: fprintf(fout, "\x1b[7m"); break;
    case TERM_DEFAULT: fprintf(fout, "\x1b[0m"); break;
    }

    va_list ap;
    va_start(ap, fmt);
    int len = vfprintf(fout, fmt, ap);
    va_end(ap);
    
    // reset
    fprintf(fout, "\x1b[39m");
    fprintf(fout, "\x1b[49m");
    fprintf(fout, "\x1b[0m");

    return len;
#endif
}
