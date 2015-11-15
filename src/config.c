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

static bool
config_create_default_setting_files(Config* self, char const* dirpath) {
    char fpath[NCONFIG_PATH] = {0};

    //! Make directory with permission "drwxr-xr-x"
    if (file_mkdir(dirpath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
        return false;

    //! Create 'config' file
    strappend(fpath, sizeof(fpath), dirpath);
    strappend(fpath, sizeof(fpath), "/config");

    FILE* fout = file_open(fpath, "wb");
    if (!fout)
        die("Failed to open file \"%s\"", fpath);
    fprintf(fout, "source %s\n", source_dirpath);
    fclose(fout);

    return true;
}

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
    char const* dirpath = buffer_getc(self->config_dirpath);

    if (!file_is_exists(dirpath)) {
        config_create_default_setting_files(self, dirpath);
    }
    else {
    }
    return true;
}

Config*
config_new(void) {
    Config* self = (Config*) calloc(1, sizeof(Config));
    if (!self)
        goto fail_0;

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

