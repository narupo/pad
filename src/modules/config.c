#include "modules/config.h"

void
config_del(config_t *self) {
    if (self) {
        free(self);
    }
}

config_t *
config_new(void) {
    config_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}
