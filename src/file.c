#include "file.h"

FILE*
file_open(char const* name, char const* mode) {
    return fopen(name, mode);
}

int
file_close(FILE* fp) {
    return fclose(fp);
}


