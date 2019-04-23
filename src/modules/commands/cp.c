#include "modules/commands/cp.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct cpcmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to cpcmd_t
 */
static void
cpcmd_show_usage(cpcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap cp [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to cpcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
cpcmd_parse_opts(cpcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"fname", required_argument, 0, 'f'},
        {},
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
cpcmd_del(cpcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self);
}

cpcmd_t *
cpcmd_new(config_t *move_config, int argc, char **move_argv) {
    cpcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!cpcmd_parse_opts(self)) {
        cpcmd_del(self);
        return NULL;
    }

    return self;
}

int
cpcmd_run(cpcmd_t *self) {
    puts("cpcmd");
    return 0;
}
