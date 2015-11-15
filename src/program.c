#include "program.h"

struct Program {
    char const* name;
    int argc;
    char** argv;
};

void
program_delete(Program* self) {
    if (self) {
        free(self);
    }
}

Program*
program_new(int argc, char* argv[]) {
    Program* self = (Program*) calloc(1, sizeof(Program));
    if (!self)
        goto fail_0;

    self->name = argv[0];
    self->argc = argc;
    self->argv = argv;

    return self;

fail_0:
    return NULL;
}

char const*
program_name(Program const* self) {
    return self->name;
}

