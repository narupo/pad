#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <stdint.h>

int
main(void) {
    setlocale(LC_CTYPE, "");
    const char32_t *s2 = U"日本語";
    wprintf(U"%ls\n%lc\n", s2, s2[0]);
    fflush(stdout);
    return 0;
}