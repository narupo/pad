#include "modules/commands/home.h"

struct homecmd {
    config_t *config;
    cmdargs_t *cmdargs;
};

void
homecmd_del(homecmd_t *self) {
    if (self) {
        config_del(self->config);
        cmdargs_del(self->cmdargs);
        free(self);
    }
}

homecmd_t *
homecmd_new(config_t *move_config, cmdargs_t *move_cmdargs) {
    homecmd_t *self = mem_ecalloc(1, sizeof(*self));
    self->config = move_config;
    self->cmdargs = move_cmdargs;
    return self;
}

int
homecmd_run(homecmd_t *self) {
    int argc = cmdargs_get_argc(self->cmdargs);
    char **argv = cmdargs_get_argv(self->cmdargs);

    if (argc < 2) {
        char line[FILE_NPATH];
        file_readline(line, sizeof line, self->config->var_home_path);
        printf("%s\n", line);
        return 0;
    }

    char newhome[FILE_NPATH];
    file_solve(newhome, sizeof newhome, argv[1]);
    if (!file_isdir(newhome)) {
        err_die("%s is not a directory", newhome);
    }

    file_writeline(newhome, self->config->var_home_path);
    file_writeline(newhome, self->config->var_cd_path);

    return 0;
}