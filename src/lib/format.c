#include <lib/format.h>

char *
fmt_capitalize_text(char *dst, size_t dstsz, const char *text) {
    int m = 0;
    char *dp = dst;
    char *dpend = dst + (dstsz-1);
    const char *p = text;

    for (; *p && dp < dpend; ) {
        char c = *p++;
        switch (m) {
        case 0: // first
            if (isspace(c)) {
                *dp++ = c;
            } else {
                if (isalpha(c)) {
                    *dp++ = toupper(c);
                } else {
                    *dp++ = c;
                }
                m = 10;
            }
            break;
        case 10: // found character
            if (c == '.') {
                *dp++ = c;
                m = 20;
            } else {
                *dp++ = c;
            }
            break;
        case 20: // found '.'
            if (isalpha(c)) {
                *dp++ = toupper(c);
                m = 10;
            } else {
                *dp++ = c;
            }
            break;
        }
    }

    *dp = '\0';
    return dst;
}
