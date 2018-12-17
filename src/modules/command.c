#include "command.h"

struct command {
    int argc;
    char **argv;
};

void
command_del(struct command *self) {
    if (self) {
        free(self);
    }
}

struct command *
command_new(int argc, char *argv[]) {
    struct command *self = cap_ecalloc(1, sizeof(*self));

    self->argc = argc;
    self->argv = argv;

    return self;
}
