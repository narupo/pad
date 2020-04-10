#include "find/arguments_manager.h"

struct arguments_manager {
    cstring_array_t *args;
};

void
argsmgr_del(argsmgr_t *self) {
    if (!self) {
        return;
    }

    cstrarr_del(self->args);
    free(self);
}

argsmgr_t *
argsmgr_new(char *argv[]) {
    argsmgr_t *self = mem_ecalloc(1, sizeof(*self));

    self->args = cstrarr_new();

    for (char **ap = argv; *ap; ++ap) {
        cstrarr_pushb(self->args, *ap);
    }

    return self; 
}

const char *
argsmgr_getc(const argsmgr_t *self, int32_t idx) {
    return cstrarr_getc(self->args, idx);
}

bool
argsmgr_contains_all(const argsmgr_t *self, const char *target) {
    bool contain = true;
    for (int32_t i = 0; i < cstrarr_len(self->args); ++i) {
        const char *arg = cstrarr_getc(self->args, i);
        if (!strstr(target, arg)) {
            contain = false;
            break;
        }
    }

    return contain;
}
