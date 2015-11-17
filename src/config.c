#include "config.h"

enum {
    NCONFIG_PATH = 256,
};

struct Config {
    Buffer* config_dirpath;
    Buffer* source_dirpath;
};

/**
 *
 */
static bool
config_create_default_config_file(Config const* self) {
    char fpath[NCONFIG_PATH] = {0};

    //! Make file path
    snprintf(fpath, sizeof(fpath), "%s/config", buffer_getc(self->config_dirpath));

    //! Create new file
    FILE* fout = file_open(fpath, "wb");
    if (!fout) {
        return false;
    }
    fprintf(fout, "source /tmp/cap\n");
    fclose(fout);

    return true;
}

/**
 *
 */
static bool
config_create_default_setting_files(Config const* self) {
    //! Make directory with permission "drwxr-xr-x"
    if (file_mkdir(buffer_getc(self->config_dirpath), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        return false;
    }

    //! Create 'config' file
    if (!config_create_default_config_file(self)) {
        return false;
    }

    return true;
}

/**
 *
 */
static FILE*
_open_config_file(Config const* self, char const* basename, char const* mode) {
    char fpath[NCONFIG_PATH] = {0};

    snprintf(fpath, sizeof(fpath), "%s/%s", buffer_getc(self->config_dirpath), basename);
    
    return file_open(fpath, mode);
}

/**
 *
 */
void
config_delete(Config* self) {
    if (self) {
        buffer_delete(self->config_dirpath);
        buffer_delete(self->source_dirpath);
        free(self);
    }
}

static bool
_load_setting_files(Config* self) {
    //! Parse config file
    FILE* fin = _open_config_file(self, "config", "rb");
    if (!fin) {
        WARN("Failed to open config file");
        goto fail_0;
    }
    Buffer* buf = buffer_new();
    if (!buf) {
        WARN("Failed to allocate buffer");
        goto fail_1;
    }

    //! Parse start
    char const delim = ' ';

    buffer_clear(self->source_dirpath);

    for (; buffer_getline(buf, fin); ) {
        char const* line = buffer_getc(buf);

        if (strncmp(line, "source", 6) == 0) {  //! TODO: magic number
            char const* p = strchr(line, delim);
            ++p;

            char* spath = file_make_solve_path(p);
            if (!spath) {
                WARN("Failed to solve path");
                goto fail_2;
            }

            buffer_clear(self->source_dirpath);
            buffer_copy_str(self->source_dirpath, spath);
            free(spath);

            if (!self->source_dirpath) {
                WARN("Failed to allocate buffer");
                goto fail_2;
            }
        }
    }

    //! Check parse results
    if (buffer_empty(self->source_dirpath)) {
        WARN("Not found source directory path");

        //! Fix
        if (!buffer_copy_str(self->source_dirpath, "/tmp")) {
            WARN("Failed to copy string");
            goto fail_2;
        }
        config_save(self);
    }

    //! Done
    fclose(fin);
    buffer_delete(buf);
    return true;

fail_2:
    buffer_delete(buf);

fail_1:
    fclose(fin);

fail_0:
    return false;
}

bool
config_load_setting(Config* self, char const* config_dirpath) {
    //! Make cap's config directory path
    char* spath = file_make_solve_path(config_dirpath);
    if (!spath) {
        WARN("Failed to make solve path");
        goto fail;
    }

    buffer_clear(self->config_dirpath);
    if (!buffer_copy_str(self->config_dirpath, spath)) {
        WARN("Failed to copy string");
        free(spath);
        goto fail;
    }
    free(spath);

    //! Check files
    if (!file_is_exists(buffer_getc(self->config_dirpath))) {
        if (!config_create_default_setting_files(self)) {
            WARN("Failed to config create default setting files");
            goto fail;
        }
    }

    //! Load
    if (!_load_setting_files(self)) {
        WARN("Failed to config load setting files.");
        goto fail;
    }

    return true;

fail:
    return false;
}

bool
config_save(Config const* self) {
    //! Open config file
    FILE* fout = _open_config_file(self, "config", "wb");
    if (!fout) {
        WARN("Failed to open config file");
        goto fail_0;
    }

    fprintf(fout, "source %s\n", buffer_getc(self->source_dirpath));

    fclose(fout);
    return true;

fail_0: 
    return false;
}

Config*
config_new(void) {
    Config* self = (Config*) calloc(1, sizeof(Config));
    if (!self) {
        WARN("Failed to allocate memory");
        goto fail_self;
    }

    if (!(self->config_dirpath = buffer_new())) {
        WARN("Failed to construct buffer");
        goto fail_config;
    }

    if (!(self->source_dirpath = buffer_new())) {
        WARN("Failed to construct buffer");
        goto fail_source;
    }

    //! Load settings from files
    if (!config_load_setting(self, "~/.cap")) {
        WARN("Failed to load setting");
        goto fail_load;
    }

    return self;

fail_load:
    buffer_delete(self->source_dirpath);

fail_source:
    buffer_delete(self->config_dirpath);

fail_config:
    free(self);

fail_self:
    return NULL;
}

char*
config_make_path_from_base(Config const* self, char const* basename) {
    if (!basename) {
        WARN("Invalid arguments");
        goto fail_0;
    }
    size_t namelen = strlen(basename);
    size_t pathsize = buffer_length(self->source_dirpath) + 1 + namelen + 1;
    char* path = (char*) malloc(sizeof(char) * pathsize);
    if (!path) {
        WARN("Failed to allocate memory");
        goto fail_1;
    }

    snprintf(path, pathsize, "%s/%s", buffer_getc(self->source_dirpath), basename);

    return path;

fail_1:
    ;
fail_0:
    return NULL;
}

char const*
config_dirpath(Config const* self) {
    return buffer_getc(self->config_dirpath);
}

char const*
config_source_dirpath(Config const* self) {
    return buffer_getc(self->source_dirpath);
}

bool
config_set_source_dirpath(Config* self, char const* srcdirpath) {
    char* spath = file_make_solve_path(srcdirpath);
    if (!spath) {
        WARN("Failed to make solve path");
        return false;
    }

    if (!buffer_copy_str(self->source_dirpath, spath)) {
        WARN("Failed to copy string");
        free(spath);
        return false;
    }

    free(spath);
    return true;
}

