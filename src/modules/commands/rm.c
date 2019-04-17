#include "modules/commands/rm.h"

extern int opterr;
extern int optind;

struct opts {
    bool is_help;
    bool is_recursive;
};

struct rmcmd {
    config_t *config;
    int argc;
    char **argv;
    int optind;
    struct opts opts;
};

void
rmcmd_parse_opts(rmcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"recursive", no_argument, 0, 'r'},
        {},
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hr", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'r': self->opts.is_recursive = true; break;
        case '?':
        default: err_die("unknown option"); break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
    }

    self->optind = optind;
}

void
rmcmd_del(rmcmd_t *self) {
    if (!self) {
        return;
    }
    freeargv(self->argc, self->argv);
    config_del(self->config);
    free(self);
}

rmcmd_t *
rmcmd_new(config_t *move_config, int argc, char **move_argv) {
    rmcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    rmcmd_parse_opts(self);

    return self;
}

void
rmcmd_show_usage(rmcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "   cap rm [path] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "   -h, --help         show usage\n"
        "   -r, --recursive    remove directories and their contents recursively\n"
        "\n"
    );
    fflush(stderr);
}

int
rmcmd_rmr(rmcmd_t *self) {
    puts("TODO");
    return 0;
}

int
rmcmd_rm(rmcmd_t *self) {
    char parpath[FILE_NPATH];
    char path[FILE_NPATH];

    if (!file_readline(parpath, sizeof parpath, self->config->var_cd_path)) {
        err_error("failed to read line from cd of variable");
        return 1;
    }

    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        if (!file_solvefmt(path, sizeof path, "%s/%s", parpath, argpath)) {
            err_error("failed to solve path");
            return 2;
        }

        if (file_remove(path) != 0) {
            err_error("failed to remove \"%s\"", path);
            return 3;
        }
    }

    return 0;
}

int
rmcmd_run(rmcmd_t *self) {
    if (self->argc < self->optind+1) {
        rmcmd_show_usage(self);
        return 0;
    }

    if (self->opts.is_help) {
        rmcmd_show_usage(self);
        return 0;        
    }

    if (self->opts.is_recursive) {
        return rmcmd_rmr(self);
    } else {
        return rmcmd_rm(self);
    }

    return 0; // impossible
}
