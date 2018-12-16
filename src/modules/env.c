#include "modules/env.h"

struct env {
    char version[32];
};

struct env *
env_new(void) {
    struct env *self = cap_ecalloc(1, sizeof(*self));
    return self;
}

void
env_del(struct env *self) {
    if (self) {
        free(self);
    }
}

void
env_set(struct env *self, const char *key, const char *val) {
    if (!strcmp(key, "CAP_VERSION")) {
        snprintf(self->version, sizeof self->version, "%s", val);
    } else {
        cap_die("not found environment key \"%s\"", key);
    }
}

const char *
env_get(const struct env *self, const char *key) {
    if (!strcmp(key, "CAP_VERSION")) {
        return self->version;
    } else {
        cap_die("not found environment key \"%s\"", key);
    }
}
