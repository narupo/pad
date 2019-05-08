#include "modules/commands/touch.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct touchcmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to touchcmd_t
 */
static void
touchcmd_show_usage(touchcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap touch [options...]\n"
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
 * @param[in] self pointer to touchcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
touchcmd_parse_opts(touchcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };
    
    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
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
touchcmd_del(touchcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

touchcmd_t *
touchcmd_new(config_t *move_config, int argc, char **move_argv) {
    touchcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!touchcmd_parse_opts(self)) {
        touchcmd_del(self);
        return NULL;
    }

    return self;
}

static int
touchcmd_touch(touchcmd_t *self) {
    const char *argpath = self->argv[self->optind];
    const char *org;
    char path[FILE_NPATH];

    if (argpath[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        err_die("impossible. invalid state in touch");
    }

    if (!file_solvefmt(path, sizeof path, "%s/%s", org, argpath)) {
        err_error("failed to solve path by \"%s\"", argpath);
        return 1;
    }

    if (!file_trunc(path)) {
        err_error("failed to truncate file");
        return 1;
    }

    return 0;
}

int
touchcmd_run(touchcmd_t *self) {
    if (self->argc < self->optind+1) {
        touchcmd_show_usage(self);
        return 1;
    }

    if (self->opts.is_help) {
        touchcmd_show_usage(self);
        return 1;        
    }

    return touchcmd_touch(self);
}
