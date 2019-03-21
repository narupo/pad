#include "modules/commands/alias.h"

struct opts {
    bool ishelp;
};

struct alcmd {
    config_t *config;
    int argc;
    char **argv;
    struct opts opts;
};

static alcmd_t *
alcmd_parse_opts(alcmd_t *self) {
    // Parse options
    static const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        // {"fname", required_argument, 0, 'f'},
        {},
    };
    static const char *shortopts = "h";

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.ishelp = true; break;
        case '?':
        default: err_die("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        err_die("Failed to parse option");
        return NULL;
    }

    return self;
}

static void
alcmd_show_usage(const alcmd_t *self) {
    fprintf(stdout, "Usage:\n"
        "\n"
        "    cap alias [name] [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage.\n"
        "\n"
        );
    fflush(stdout);
}

void
alcmd_del(alcmd_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        free(self);
    }
}

alcmd_t *
alcmd_new(config_t *move_config, int argc, char **move_argv) {
    alcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (alcmd_parse_opts(self) == NULL) {
        err_die("failed to parse options");
        return NULL;
    }

    return self;
}

int
alcmd_show_list(const alcmd_t *self) {
    almgr_t *almgr = almgr_new(self->config);

    if (almgr_load_alias_list(almgr, CAP_SCOPE_LOCAL) == NULL) {
        if (almgr_load_alias_list(almgr, CAP_SCOPE_GLOBAL) == NULL) {
            if (almgr_has_error(almgr)) {
                err_error(almgr_get_error_detail(almgr));
            }
            almgr_del(almgr);
            return 1;
        }
    }

    const almap_t *almap = almgr_getc_almap(almgr);
    for (int i = 0; i < almap_len(almap); ++i) {
        const alias_t *alias = almap_getc_index(almap, i);
        if (alias == NULL) {
            continue;
        }
        printf("%s %s\n", alias->key, alias->value);
    }
    fflush(stdout);

    almgr_del(almgr);
    return 0;
}

int
alcmd_run(alcmd_t *self) {
    if (self->opts.ishelp) {
        alcmd_show_usage(self);
        return 0;
    }

    return alcmd_show_list(self);
}
