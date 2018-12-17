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

    for (int i = 1; i < app_argc; ++i) {
        cstrarr_push(args, app_argv[i]);
    }
    cstrarr_move(args, NULL);

    freeargv(self->argc, self->argv);

    self->argc = cstrarr_len(args)-1;
    self->argv = cstrarr_escdel(args);

    showargv(self->argc, self->argv);

    return self;
}
