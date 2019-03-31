#include "command.h"

struct command {
    cmdargs_t *cmdargs;
};

void
command_del(struct command_t *self) {
    if (self) {
        free(self);
    }
}

struct command_t *
command_new(cmdargs_t *move_cmdargs) {
    struct command_t *self = cap_ecalloc(1, sizeof(*self));

    self->cmdargs = move_cmdargs;

    return self;
}
