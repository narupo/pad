#include "sh/sh.h"

enum {
    LINE_BUFFER_SIZE = 1024 * 10,
};

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct sh {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    char line_buf[LINE_BUFFER_SIZE];
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to shcmd_t
 */
static void
shcmd_show_usage(shcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap sh [options]...\n"
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
 * @param[in] self pointer to shcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
shcmd_parse_opts(shcmd_t *self) {
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
shcmd_del(shcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

shcmd_t *
shcmd_new(const config_t *config, int argc, char **argv) {
    shcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!shcmd_parse_opts(self)) {
        shcmd_del(self);
        return NULL;
    }

    return self;
}

int
shcmd_run(shcmd_t *self) {
    for (;;) {
        printf("%s$ ", self->config->cd_path);
        fflush(stdout);

        self->line_buf[0] = '\0';
        if (file_getline(self->line_buf, sizeof self->line_buf, stdin) == EOF) {
            break;
        }

        printf("%s\n", self->line_buf);
    }

    return 0;
}
