#include "modules/commands/hub/hub.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct hubcmd {
    config_t *config;
    int argc;
    int cmd_argc;
    int optind;
    char **argv;
    char **cmd_argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to hubcmd_t
 */
static void
hubcmd_show_usage(hubcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Cap's Hub command is manage server and client\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap hub [command] [options]...\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    runserver    Run server\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    Show usage\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to hubcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
hubcmd_parse_opts(hubcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"fname", required_argument, 0, 'f'},
        {0},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hf:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'f': printf("%s\n", optarg); break;
        case '?':
        default:
            err_die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
hubcmd_del(hubcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    freeargv(self->cmd_argc, self->cmd_argv);
    free(self);
}

hubcmd_t *
hubcmd_distribute_args(hubcmd_t *self, int argc, char **argv) {
    distribute_args_t dargs = {0};
    if (!distribute_args(&dargs, argc, argv)) {
        return NULL;
    }

    self->argc = dargs.argc;
    self->argv = dargs.argv;
    self->cmd_argc = dargs.cmd_argc;
    self->cmd_argv = dargs.cmd_argv;
    return self;
}

hubcmd_t *
hubcmd_new(config_t *move_config, int argc, char **move_argv) {
    hubcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;

    if (!hubcmd_distribute_args(self, argc, move_argv)) {
        hubcmd_del(self);
        return NULL;
    }
    freeargv(argc, move_argv);

    if (!hubcmd_parse_opts(self)) {
        hubcmd_del(self);
        return NULL;
    }

    return self;
}

int
hubcmd_run_command(hubcmd_t *self) {
    const char *cmd = self->cmd_argv[0];
    if (cstr_eq(cmd, "runserver")) {
        puts(cmd);
    }
    return 0;
}

int
hubcmd_run(hubcmd_t *self) {
    if (self->cmd_argc == 0) {
        hubcmd_show_usage(self);
        return 0;
    }

    return hubcmd_run_command(self);
}
