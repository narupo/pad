#include <core/config.h>

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
    self->scope = CAP_SCOPE_LOCAL;
    self->recursion_count = 0;

    strcpy(self->line_encoding, "lf");

    if (!file_solve(self->var_cd_path, sizeof self->var_cd_path, "~/.cap/var/cd")) {
        err_error("failed to create path of cd of variable");
        return false;
    }
    if (!file_solve(self->var_home_path, sizeof self->var_home_path, "~/.cap/var/home")) {
        err_error("failed to create path of home of variable");
        return false;
    }
    if (!file_solve(self->var_editor_path, sizeof self->var_editor_path, "~/.cap/var/editor")) {
        err_error("failed to create path of editor of variable");
        return false;
    }

    if (!file_readline(self->cd_path, sizeof self->cd_path, self->var_cd_path)) {
        err_error("failed to read line from cd of variable");
        return false;
    }
    pop_tail_slash(self->cd_path);

    if (!file_readline(self->home_path, sizeof self->home_path, self->var_home_path)) {
        err_error("failed to read line from home of variable");
        return false;
    }
    pop_tail_slash(self->home_path);

    if (!file_readline(self->editor, sizeof self->editor, self->var_editor_path)) {
        err_error("failed to read line from editor of variable");
        return false;
    }

    if (!file_solve(self->codes_dir_path, sizeof self->codes_dir_path, "~/.cap/codes")) {
        err_error("failed to solve path for snippet codes directory path");
        return false;
    }

    // standard libraries
    if (!file_solve(self->std_lib_dir_path, sizeof self->std_lib_dir_path, "~/.cap/stdlib")) {
        err_error("failed to solve path for standard libraries directory");
        return false;
    }

    return self;
}
