#include "modules/alias_manager.h"

struct alias_manager {
    const config_t *config;
};

void
almgr_del(almgr_t *self) {
    if (self) {
        free(self);
    }
}

almgr_t *
almgr_new(const config_t *config) {
    almgr_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;

    return self;
}

almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope) {
    strcpy(dst, "run bin/date-line");
    return self;
}
