#include "find/find.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct find {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to findcmd_t
 */
static void
findcmd_show_usage(findcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap find [options]...\n"
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
 * @param[in] self pointer to findcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
findcmd_parse_opts(findcmd_t *self) {
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
findcmd_del(findcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

findcmd_t *
findcmd_new(const config_t *config, int argc, char **argv) {
    findcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!findcmd_parse_opts(self)) {
        findcmd_del(self);
        return NULL;
    }

    return self;
}

int
findcmd_run(findcmd_t *self) {
    puts("find");
    return 0;
}
