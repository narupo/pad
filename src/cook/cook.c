#include <cook/cook.h>

/**
 * structure of command
 */
struct cookcmd {
    const config_t *config;
    int argc;
    char **argv;
    errstack_t *errstack;
};

void
cookcmd_del(cookcmd_t *self) {
    if (!self) {
        return;
    }

    errstack_del(self->errstack);
    free(self);
}

cookcmd_t *
cookcmd_new(const config_t *config, int argc, char **argv) {
    cookcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = errstack_new();

    return self;
}

int
cookcmd_run(cookcmd_t *self) {
    int result = make_from_args(
        self->config,
        self->errstack,
        self->argc,
        self->argv,
        false  // look me!
    );
    if (result != 0) {
        errstack_trace(self->errstack, stderr);
    }

    return result;
}
