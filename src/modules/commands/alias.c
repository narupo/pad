#include "modules/commands/alias.h"

struct opts {
    bool ishelp;
    bool isall;
};

struct alcmd {
    config_t *config;
    int argc;
    char **argv;
    struct opts opts;
};

void
alcmd_del(alcmd_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        free(self);
    }
}

alcmd_t *
alcmd_new(config_t *move_config, int argc, char **move_argv) {
    alcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    return self;
}

int
alcmd_run(alcmd_t *self) {
    // load home/.caprc
    puts("alias");
}
