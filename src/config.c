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

bool
config_load_settings(Config* self) {
    //! Create cap's config directory at user's home
    char const* dirpath = buffer_getc(self->config_dirpath);

    if (!file_is_exists(dirpath)) {
        if (file_mkdir(dirpath, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
            return false;
    }
    return true;
}

Config*
config_new(void) {
    Config* self = (Config*) calloc(1, sizeof(Config));
    if (!self)
        goto fail_0;

    //! Make Buffers

    //! Make cap's config directory path
    self->config_dirpath = buffer_new_str(config_dirpath);
    if (!self->config_dirpath)
        goto fail_1;

    //! Make cap's source directory path
    self->source_dirpath = buffer_new_str(source_dirpath);
    if (!self->source_dirpath)
        goto fail_2;

    //! Load settings from files
    if (!config_load_settings(self))
        goto fail_3;

    return self;

fail_3:
    buffer_delete(self->source_dirpath);
fail_2:
    buffer_delete(self->config_dirpath);
fail_1:
    free(self);
fail_0:
    return NULL;
}

char*
config_make_path_from_base(Config const* self, char const* name) {
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

