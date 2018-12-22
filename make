#!/bin/bash
gcc -std=c11 -I$HOME/shared/cap/src -c \
    src/app.c \
    src/lib/memory.c \
    src/lib/error.c \
    src/lib/file.c \
    src/lib/string.c \
    src/lib/cstring_array.c \
    src/modules/util.c \
    src/modules/cmdargs.c \
    src/modules/commands/homecmd.c \

mv ./*.o build/
gcc -std=c11 -o build/cap ./build/*.o

