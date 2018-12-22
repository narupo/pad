#include "cmdargs.h"

struct cmdargs {
    int argc;
    char **argv; // store dynamic allocate memory
};

void
cmdargs_del(cmdargs_t *self) {
    if (self) {
        freeargv(self->argc, self->argv);
        free(self);
    }
}

cmdargs_t *
cmdargs_new(void) {
    cmdargs_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

cmdargs_t *
cmdargs_parse(cmdargs_t *self, int app_argc, char *app_argv[]) {
    if (app_argc < 2) {
        return self;
    }

    cstring_array_t *args = cstrarr_new();

    for (int i = 1; i < app_argc; ++i) {
        cstrarr_push(args, app_argv[i]);
    }
    cstrarr_move(args, NULL);

    freeargv(self->argc, self->argv);

    self->argc = cstrarr_len(args)-1;
    self->argv = cstrarr_escdel(args);

    return self;
}

const char *
cmdargs_get_cmdname(const cmdargs_t *self) {
    if (self->argc <= 0) {
        return NULL;
    }

    return self->argv[0];
}
