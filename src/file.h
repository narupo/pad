#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>

FILE*
file_open(char const* name, char const* mode);

int
file_close(FILE* fp);

#endif

