#pragma once

enum {
    ALIAS_KEY_SIZE = 256,
    ALIAS_VAL_SIZE = 512,
};

struct alias {
    char key[ALIAS_KEY_SIZE];
    char value[ALIAS_VAL_SIZE];
};

typedef struct alias alias_t;

