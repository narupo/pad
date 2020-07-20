#include <alias/alias.h>

struct opts {
    bool is_help;
    bool is_global;
    bool is_desc;
};

struct alcmd {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    almgr_t *almgr;
    int32_t key_colors[3];
    int32_t value_colors[3];
    int32_t desc_colors[3];
};

static alcmd_t *
alcmd_parse_opts(alcmd_t *self) {
    // Parse options
    static const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"global", required_argument, 0, 'g'},
        {"description", no_argument, 0, 'd'},
        {0},
    };
    static const char *shortopts = "hgd";

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
        case 'h': self->opts.is_help = true; break;
        case 'g': self->opts.is_global = true; break;
        case 'd': self->opts.is_desc = true; break;
        case '?':
        default: err_die("unknown option"); break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
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
        "    -h, --help        show usage.\n"
        "    -g, --global      scope to global.\n"
        "    -d, --description show description of alias.\n"
        "\n"
    );
    fflush(stdout);
}

void
alcmd_del(alcmd_t *self) {
    if (!self) {
        return;
    }

    almgr_del(self->almgr);
    free(self);
}

alcmd_t *
alcmd_new(const config_t *config, int argc, char **argv) {
    alcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->almgr = almgr_new(self->config);
    self->key_colors[0] = TERM_GREEN;
    self->key_colors[1] = TERM_BLACK;
    self->key_colors[2] = TERM_BRIGHT;
    self->value_colors[0] = TERM_CYAN;
    self->value_colors[1] = TERM_BLACK;
    self->value_colors[2] = TERM_BRIGHT;
    self->desc_colors[0] = TERM_RED;
    self->desc_colors[1] = TERM_BLACK;
    self->desc_colors[2] = TERM_BRIGHT;

    if (!alcmd_parse_opts(self)) {
        err_die("failed to parse options");
        return NULL;
    }

    return self;
}

static alcmd_t *
alcmd_load_alias_list_by_opts(alcmd_t* self) {
    if (self->opts.is_global) {
        if (!almgr_load_alias_list(self->almgr, CAP_SCOPE_GLOBAL)) {
            if (almgr_has_error(self->almgr)) {
                err_error(almgr_get_error_detail(self->almgr));
            }
            return NULL;
        }
    } else {
        if (!almgr_load_alias_list(self->almgr, CAP_SCOPE_LOCAL)) {
            if (almgr_has_error(self->almgr)) {
                err_error(almgr_get_error_detail(self->almgr));
            }
            return NULL;
        }
    }
    return self;
}

static const char *
alcmd_getc_value(alcmd_t *self, const char *key) {
    const context_t *ctx = almgr_getc_context(self->almgr);
    const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
    return alinfo_getc_value(alinfo, key);
}

static void
padline(FILE *fout, int32_t len) {
    for (int32_t i = 0; i < len; ++i) {
        fputc(' ', fout);
    }
}

static void
print_key_val_desc(const alcmd_t *self, FILE *fout, bool print_color, int32_t keymaxlen, int32_t valmaxlen, const char *key, const char *val, const char *desc) {
    if (!print_color) {
        printf("%-*s    %-*s    %s\n", keymaxlen, key, valmaxlen, val, desc);
        return;
    }

    int32_t difkeylen = keymaxlen - strlen(key);
    int32_t difvallen = valmaxlen - strlen(val);

    term_cfprintf(fout, self->key_colors[0], self->key_colors[1], self->key_colors[2], "%s", key);
    padline(fout, difkeylen + 4);
    term_cfprintf(fout, self->value_colors[0], self->value_colors[1], self->value_colors[2], "%s", val);
    padline(fout, difvallen + 4);
    term_cfprintf(fout, self->desc_colors[0], self->desc_colors[1], self->desc_colors[2], "%s\n", desc);
}

static void
print_key_val(const alcmd_t *self, FILE *fout, bool print_color, int32_t keymaxlen, const char *key, const char *val) {
    if (!print_color) {
        printf("%-*s    %s\n", keymaxlen, key, val);
        return;
    }

    int32_t difkeylen = keymaxlen - strlen(key);

    term_cfprintf(fout, self->key_colors[0], self->key_colors[1], self->key_colors[2], "%s", key);
    padline(fout, difkeylen + 4);
    term_cfprintf(fout, self->value_colors[0], self->value_colors[1], self->value_colors[2], "%s\n", val);
}

static int
alcmd_show_list(alcmd_t *self) {
    const context_t *ctx = almgr_getc_context(self->almgr);
    const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
    const dict_t *key_val_map = alinfo_getc_key_value_map(alinfo);
    int keymaxlen = 0;
    int valmaxlen = 0;

#undef max
#define max(a, b) (a > b ? a : b);

    for (int i = 0; i < dict_len(key_val_map); ++i) {
        const dict_item_t *item = dict_getc_index(key_val_map, i);
        if (!item) {
            continue;
        }
        keymaxlen = max(strlen(item->key), keymaxlen);
        valmaxlen = max(strlen(item->value), valmaxlen);
    }

    FILE *fout = stdout;
    bool print_color = isatty(file_get_no(fout));

    for (int i = 0; i < dict_len(key_val_map); ++i) {
        const dict_item_t *kv_item = dict_getc_index(key_val_map, i);
        if (!kv_item) {
            continue;
        }

        const char *desc = alinfo_getc_desc(alinfo, kv_item->key);
        if (self->opts.is_desc && desc) {
            char disp_desc[128] = {0};
            trim_first_line(disp_desc, sizeof disp_desc, desc);

            print_key_val_desc(
                self,
                fout,
                print_color,
                keymaxlen,
                valmaxlen,
                kv_item->key,
                kv_item->value,
                disp_desc
            );
        } else {
            print_key_val(
                self,
                fout,
                print_color,
                keymaxlen,
                kv_item->key,
                kv_item->value
            );
        }
    }
    fflush(stdout);

    return 0;
}

static int
alcmd_show_alias_value(alcmd_t *self) {
    const char *key = self->argv[self->optind];
    const char *value = alcmd_getc_value(self, key);
    if (!value) {
        err_error("not found alias \"%s\"", key);
        return 1;
    }

    puts(value);
    fflush(stdout);
    
    return 0;
}

int
alcmd_show_desc_of_alias(alcmd_t *self) {
    const context_t *ctx = almgr_getc_context(self->almgr);
    const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
    const char *key = self->argv[self->optind];
    const char *desc = alinfo_getc_desc(alinfo, key);
    if (!desc) {
        return 0;
    }

    puts(desc);
    fflush(stdout);

    return 0;
}

int
alcmd_run(alcmd_t *self) {
    if (self->opts.is_help) {
        alcmd_show_usage(self);
        return 0;
    }

    if (!alcmd_load_alias_list_by_opts(self)) {
        return 1;
    }

    if (self->argc - self->optind == 0) {
        return alcmd_show_list(self);
    }

    if (self->opts.is_desc) {
        return alcmd_show_desc_of_alias(self);
    }

    return alcmd_show_alias_value(self);
}
