#include "home/home.h"

struct homecmd {
    const config_t *config;
    int argc;
    char **argv;
};

void
homecmd_del(homecmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

homecmd_t *
homecmd_new(const config_t *config, int argc, char *argv[]) {
    homecmd_t *self = mem_ecalloc(1, sizeof(*self));
    self->config = config;
    self->argc = argc;
    self->argv = argv;
    return self;
}

int
homecmd_run(homecmd_t *self) {
    int argc = self->argc;
    char **argv = self->argv;

    if (argc < 2) {
        char line[FILE_NPATH];
        if (!file_readline(line, sizeof line, self->config->var_home_path)) {
            err_error("failed to read line from home of variable");
            return 1;
        }
        printf("%s\n", line);
        return 0;
    }

    char newhome[FILE_NPATH];
    if (!file_solve(newhome, sizeof newhome, argv[1])) {
        err_error("failed to solve path from \"%s\"", argv[1]);
        return 2;
    }
    if (!file_isdir(newhome)) {
        err_error("%s is not a directory", newhome);
        return 3;
    }

    if (!file_writeline(newhome, self->config->var_home_path)) {
        err_error("failed to write line to home variable");
        return 4;
    }

    if (!file_writeline(newhome, self->config->var_cd_path)) {
        err_error("failed to write line to cd variable");
        return 5;
    }

    return 0;
}