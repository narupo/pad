#include "modules/commands/homecmd.h"

struct homecmd {
    cmdargs_t *cmdargs;
};

void
homecmd_del(homecmd_t *self) {
    if (self) {
        cmdargs_del(self->cmdargs);
        free(self);
    }
}

homecmd_t *
homecmd_new(cmdargs_t *move_cmdargs) {
    homecmd_t *self = mem_ecalloc(1, sizeof(*self));
    self->cmdargs = move_cmdargs;
    return self;
}

int
homecmd_run(homecmd_t *self) {
    puts("home!");
    return 0;
}