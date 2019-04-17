#include "modules/commands/alias.h"

struct opts {
    bool ishelp;
    bool isglobal;
};

struct alcmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    almgr_t *almgr;
};

static alcmd_t *
alcmd_parse_opts(alcmd_t *self) {
    // Parse options
    static const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"global", required_argument, 0, 'g'},
        {},
    };
    static const char *shortopts = "hg";

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
        case 'g': self->opts.isglobal = true; break;
        case '?':
        default: err_die("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        err_die("Failed to parse option");
        return NULL;
    }

    self->optind = optind;

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
        "    -g, --global  scope to global.\n"
    );
    fflush(stdout);
}

void
alcmd_del(alcmd_t *self) {
    if (self) {
        almgr_del(self->almgr);
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
    self->almgr = almgr_new(self->config);

    if (alcmd_parse_opts(self) == NULL) {
        err_die("failed to parse options");
        return NULL;
    }

    return self;
}

static alcmd_t *
alcmd_load_alias_list_by_opts(alcmd_t* self) {
    if (self->opts.isglobal) {
        if (almgr_load_alias_list(self->almgr, CAP_SCOPE_GLOBAL) == NULL) {
            if (almgr_has_error(self->almgr)) {
                err_error(almgr_get_error_detail(self->almgr));
            }
            almgr_del(self->almgr);
            return NULL;
        }
    } else {
        if (almgr_load_alias_list(self->almgr, CAP_SCOPE_LOCAL) == NULL) {
            if (almgr_has_error(self->almgr)) {
                err_error(almgr_get_error_detail(self->almgr));
            }
            almgr_del(self->almgr);
            return NULL;
        }
    }
    return self;
}

static const char *
alcmd_getc_value(alcmd_t *self, const char *key) {
    const context_t *ctx = almgr_getc_context(self->almgr);
    const dict_t *almap = ctx_getc_almap(ctx);
    const dict_item_t *item = dict_getc(almap, key);
    if (item == NULL) {
        return NULL;
    }
    return item->value;
}

static int
alcmd_show_list(alcmd_t *self) {
    const context_t *ctx = almgr_getc_context(self->almgr);
    const dict_t *almap = ctx_getc_almap(ctx);
    int keymaxlen = 0;

#undef max
#define max(a, b) (a > b ? a : b);

    for (int i = 0; i < dict_len(almap); ++i) {
        const dict_item_t *alias = dict_getc_index(almap, i);
        if (alias == NULL) {
            continue;
        }
        keymaxlen = max(strlen(alias->key), keymaxlen);
    }

#undef max

    for (int i = 0; i < dict_len(almap); ++i) {
        const dict_item_t *alias = dict_getc_index(almap, i);
        if (alias == NULL) {
            continue;
        }
        printf("%-*s %s\n", keymaxlen, alias->key, alias->value);
    }
    fflush(stdout);

    return 0;
}

static int
alcmd_show_alias_value(alcmd_t *self, const char *key) {
    const char *value = alcmd_getc_value(self, key);
    if (value == NULL) {
        err_error("not found alias \"%s\"", key);
        return 1;
    }
    puts(value);
    fflush(stdout);
    return 0;
}

int
alcmd_run(alcmd_t *self) {
    if (self->opts.ishelp) {
        alcmd_show_usage(self);
        return 0;
    }

    if (alcmd_load_alias_list_by_opts(self) == NULL) {
        return 1;
    }

    if (self->argc - self->optind >= 1) {
        return alcmd_show_alias_value(self, self->argv[self->optind]);
    }

    return alcmd_show_list(self);
}
