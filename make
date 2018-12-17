#!/bin/bash
gcc -std=c11 -I$HOME/shared/cap/src -o build/cap \
    src/app.c \
    src/lib/memory.c \
    src/lib/error.c \
    src/lib/file.c \
    src/modules/util.c \
    src/modules/env.c \

