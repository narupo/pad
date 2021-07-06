#include <pad/lib/term.h>

#ifdef PAD_TERM__WINDOWS
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
    case PAD_TERM__BLACK:   break;
    case PAD_TERM__RED:     attr |= FOREGROUND_RED; break;
    case PAD_TERM__GREEN:   attr |= FOREGROUND_GREEN; break;
    case PAD_TERM__BLUE:    attr |= FOREGROUND_BLUE; break;
    case PAD_TERM__YELLOW:  attr |= FOREGROUND_RED | FOREGROUND_GREEN; break;
    case PAD_TERM__MAGENTA: attr |= FOREGROUND_RED | FOREGROUND_BLUE; break;
    case PAD_TERM__CYAN:    attr |= FOREGROUND_GREEN | FOREGROUND_BLUE; break;
    case PAD_TERM__GRAY:    attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
    case PAD_TERM__WHITE:   attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
    case PAD_TERM__DEFAULT: break;
    }
    
    switch (bg) {
    case PAD_TERM__BLACK:   break;
    case PAD_TERM__RED:     attr |= BACKGROUND_RED; break;
    case PAD_TERM__GREEN:   attr |= BACKGROUND_GREEN; break;
    case PAD_TERM__BLUE:    attr |= BACKGROUND_BLUE; break;
    case PAD_TERM__YELLOW:  attr |= BACKGROUND_RED | BACKGROUND_GREEN; break;
    case PAD_TERM__MAGENTA: attr |= BACKGROUND_RED | BACKGROUND_BLUE; break;
    case PAD_TERM__CYAN:    attr |= BACKGROUND_GREEN | BACKGROUND_BLUE; break;
    case PAD_TERM__GRAY:    attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
    case PAD_TERM__WHITE:   attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY; break;
    case PAD_TERM__DEFAULT: break;
    }
    
    switch (opt) {
    case PAD_TERM__BRIGHT:
        attr |= FOREGROUND_INTENSITY;
        break;
    case PAD_TERM__UNDER: attr |= COMMON_LVB_UNDERSCORE; break;
    case PAD_TERM__REVERSE: attr |= COMMON_LVB_REVERSE_VIDEO; break;
    }

    SetConsoleTextAttribute(handle, attr);
}
#endif

int
PadTerm_CFPrintf(FILE *fout, int fg, int bg, int opt, const char *fmt, ...) {
#ifdef PAD_TERM__WINDOWS
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
    case PAD_TERM__NULL: break;
    case PAD_TERM__BLACK:   fprintf(fout, "\x1b[30m"); break;
    case PAD_TERM__RED:     fprintf(fout, "\x1b[31m"); break;
    case PAD_TERM__GREEN:   fprintf(fout, "\x1b[32m"); break;
    case PAD_TERM__YELLOW:  fprintf(fout, "\x1b[33m"); break;
    case PAD_TERM__BLUE:    fprintf(fout, "\x1b[34m"); break;
    case PAD_TERM__MAGENTA: fprintf(fout, "\x1b[35m"); break;
    case PAD_TERM__CYAN:    fprintf(fout, "\x1b[36m"); break;
    case PAD_TERM__GRAY:    fprintf(fout, "\x1b[37m"); break;
    case PAD_TERM__WHITE:   fprintf(fout, "\x1b[37m"); break;
    case PAD_TERM__DEFAULT: fprintf(fout, "\x1b[39m"); break;
    }

    switch (bg) {
    case PAD_TERM__NULL: break;
    case PAD_TERM__BLACK:   fprintf(fout, "\x1b[40m"); break;
    case PAD_TERM__RED:     fprintf(fout, "\x1b[41m"); break;
    case PAD_TERM__GREEN:   fprintf(fout, "\x1b[42m"); break;
    case PAD_TERM__YELLOW:  fprintf(fout, "\x1b[43m"); break;
    case PAD_TERM__BLUE:    fprintf(fout, "\x1b[44m"); break;
    case PAD_TERM__MAGENTA: fprintf(fout, "\x1b[45m"); break;
    case PAD_TERM__CYAN:    fprintf(fout, "\x1b[46m"); break;
    case PAD_TERM__GRAY:    fprintf(fout, "\x1b[47m"); break;
    case PAD_TERM__WHITE:   fprintf(fout, "\x1b[47m"); break;
    case PAD_TERM__DEFAULT: fprintf(fout, "\x1b[49m"); break;
    }

    switch (opt) {
    case PAD_TERM__NULL: break;
    case PAD_TERM__UNDER:   fprintf(fout, "\x1b[4m"); break;
    case PAD_TERM__BRIGHT:  fprintf(fout, "\x1b[1m"); break;
    case PAD_TERM__REVERSE: fprintf(fout, "\x1b[7m"); break;
    case PAD_TERM__DEFAULT: fprintf(fout, "\x1b[0m"); break;
    }

    va_list ap;
    va_start(ap, fmt);
    int len = vfprintf(fout, fmt, ap);
    va_end(ap);
    
    // reset
    fprintf(fout, "\x1b[39m");
    fprintf(fout, "\x1b[49m");
    fprintf(fout, "\x1b[0m");
    fflush(fout);

    return len;
#endif
}
