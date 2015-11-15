#include "config.h"

enum {
    NCONFIG_PATH = 256,
};

struct Config {
    Buffer* config_dirpath;
    Buffer* source_dirpath;
};

static char const* config_dirpath = "~/.cap";
static char const* source_dirpath = "/tmp/cap";

void
config_delete(Config* self) {
    if (self) {
        buffer_delete(self->config_dirpath);
        buffer_delete(self->source_dirpath);
        free(self);
    }
}

Config*
config_new(void) {
    Config* self = (Config*) calloc(1, sizeof(Config));
    if (!self)
        goto fail_0;

    // Make dir path
    self->config_dirpath = buffer_new_str(config_dirpath);
    if (!self->config_dirpath)
        goto fail_1;

    self->source_dirpath = buffer_new_str(source_dirpath);
    if (!self->source_dirpath)
        goto fail_2;

    return self;

fail_2:
    buffer_delete(self->config_dirpath);
fail_1:
    free(self);
fail_0:
    return NULL;
}

char*
config_make_file_path(Config const* self, char const* name) {
    if (!name)
        goto fail_0;

    size_t namelen = strlen(name);
    size_t pathsize = buffer_length(self->config_dirpath) + 1 + namelen + 1;
    char* path = (char*) malloc(sizeof(char) * pathsize);
    if (!path)
        goto fail_1;

    snprintf(path, pathsize, "%s/%s", buffer_getc(self->config_dirpath), name);

    return path;

fail_1:
    ;
fail_0:
    return NULL;
}

