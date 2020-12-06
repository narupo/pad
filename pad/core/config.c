#include <pad/core/config.h>

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

char *
pop_tail_slash(char *path);

config_t *
config_init(config_t *self) {
    strcpy(self->line_encoding, "lf");

    // standard libraries
    if (!file_solve(self->std_lib_dir_path, sizeof self->std_lib_dir_path, "~/.cap/stdlib")) {
        err_error("failed to solve path for standard libraries directory");
        return false;
    }

    return self;
}
