#include "cmdargs.h"

struct cmdargs {
    int argc;
    char **argv; // store dynamic allocate memory
};

void
cmdargs_del(cmdargs *self) {
    freeargv(self->argc, self->argv);
    free(self);
}

cmdargs *
cmdargs_new(void) {
    cmdargs *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

cmdargs *
cmdargs_parse(cmdargs *self, int app_argc, char *app_argv[]) {
    if (app_argc < 2) {
        return self;
    }

    cstring_array *args = cstrarr_new();


    return self;
}
