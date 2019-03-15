#include "modules/commands/cat.h"

struct opts {
    bool ishelp;
    int indent;
    int tabspaces;
    bool istab;
};

struct catcmd {
    config_t *config;
    int argc;
    char **argv;
    struct opts opts;
    int optind;
};

/**
 * Parse options
 * 
 * @param[in] *self   
 * @param[in] argc    
 * @param[in] *argv[] 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
catcmd_t *
catcmd_parse_opts(catcmd_t *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"indent", required_argument, 0, 'i'},
        {"tabspaces", required_argument, 0, 'T'},
        {"tab", no_argument, 0, 't'},
        {},
    };

    self->opts = (struct opts){
        .ishelp = false,
        .indent = 0,
        .tabspaces = 4,
        .istab = false,
    };
    opterr = 0;
    optind = 0;

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hi:T:t", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.ishelp = true; break;
        case 'i': self->opts.indent = atoi(optarg); break;
        case 'T': self->opts.tabspaces = atoi(optarg); break;
        case 't': self->opts.istab = true; break;
        case '?':
        default: return NULL; break;
        }
    }

    if (self->argc < optind) {
        return NULL;
    }

    self->optind = optind;

    return self;
}

void
catcmd_del(catcmd_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        free(self);
    }
}

catcmd_t *
catcmd_new(config_t *move_config, int argc, char **move_argv) {
    catcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!catcmd_parse_opts(self)) {
        err_error("failed to parse options");
        catcmd_del(self);
        return NULL;
    }

    return self;
}

/**
 * Make file path for catenate
 * 
 * @param[in] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *cdpath string of cd path
 * @param[in] *name string of name
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
static char *
catcmd_makepath(catcmd_t *self, char *dst, size_t dstsz, const char *cdpath, const char *name) {
    if (!file_solvefmt(dst, dstsz, "%s/%s", cdpath, name)) {
        return NULL;
    }

    if (isoutofhome(self->config->var_home_path, dst)) {
        return NULL;
    }

    if (file_isdir(dst)) {
        return NULL;
    }

    return dst;
}

/**
 * Set indent characters.
 *
 * @param[in] *opts options
 * @param[out] *buf buffer
 * @param[in] bufsize buffer size
 *
 * @return success to true
 * @return failed to false
 */
static bool
setindent(catcmd_t *self, char *buf, size_t bufsize) {
    if (self->opts.istab) {
        buf[0] = '\t';
        buf[1] = '\0';
    } else {
        if (self->opts.tabspaces >= bufsize-1) {
            return false;
        }

        memset(buf, ' ', self->opts.tabspaces);
        buf[self->opts.tabspaces] = '\0';
    }

    return true;
}

/**
 * Catenate fin to fout
 * 
 * @param[in] *fout destination stream
 * @param[in] *fin source stream
 * 
 * @return success to number of zero
 * @return failed to not a number of zero
 */
static int
catcmd_catstream(catcmd_t *self, FILE *fout, FILE *fin) {
    int m = 0;

    for (;;) {
        int c = fgetc(fin);
        if (c == EOF) {
            break;
        }

        switch (m) {
        case 0: { // Indent mode
            char str[100] = {0};
            if (!setindent(self, str, sizeof str)) {
                return 1;
            }

            for (int i = 0; i < self->opts.indent; ++i) {
                fprintf(fout, "%s", str);
            }

            fputc(c, fout);
            if (c != '\n') {
                m = 1;
            }
        } break;
        case 1: { // Stream mode
            fputc(c, fout);
            if (c == '\n') {
                m = 0;
            }
        } break;
        }
    }

    return 0;
}

/**
 * Catenate file content to fout
 * 
 * @param[in] *fout destination buffer
 * @param[in] *path file path
 * 
 * @return success to true
 * @return failed to false
 */
static bool
catcmd_catfile(catcmd_t *self, FILE *fout, const char *path) {
    FILE *fin = fopen(path, "rb");
    if (!fin) {
        return false;
    }

    if (catcmd_catstream(self, fout, fin) != 0) {
        fclose(fin);
        return false;
    }
    
    if (fclose(fin) < 0) {
        return false;
    }

    return true;
}

/**
 * Show usage and exit from proccess
 *
 * @param[in] self
 */
static void
catcmd_usage(const catcmd_t *self) {
    fprintf(stderr,
        "Usage: cap cat [options] [files]\n"
        "\n"
        "    Concatenate files.\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -i, --indent     indent spaces.\n"
        "    -T, --tabspaces  number of tab spaces.\n"
        "    -t, --tab        tab indent mode.\n"
        "\n"
        "Examples:\n"
        "\n"
        "    $ cap cat f - g\n"
        "    $ cap cat\n"
        "\n"
    );
}

/**
 * Execute command
 *
 * @param[in] self
 * @return success to number of 0
 * @return failed to number of other of 0
 */
int
catcmd_run(catcmd_t *self) {
    if (self->opts.ishelp) {
        catcmd_usage(self);
        return 0;
    }

    if (self->argc - self->optind + 1 < 2) {
        return catcmd_catstream(self, stdout, stdin);
    }

    char cdpath[FILE_NPATH];
    if (self->config->scope == CAP_SCOPE_LOCAL) {
        if (!file_readline(cdpath, sizeof cdpath, self->config->var_cd_path)) {
            err_die("need environment variable of cd");
        }
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        if (!file_readline(cdpath, sizeof cdpath, self->config->var_home_path)) {
            err_die("need environment variable of home");
        }
    } else {
        err_error("invalid scope \"%s\"", self->config->scope);
        return 1;
    }

    int ret = 0;
    for (int i = self->optind; i < self->argc; ++i) {
        const char *name = self->argv[i];

        if (strcmp(name, "-") == 0) {
            catcmd_catstream(self, stdout, stdin);
            continue;
        }
        
        char path[FILE_NPATH];
        if (!catcmd_makepath(self, path, sizeof path, cdpath, name)) {
            ++ret;
            err_error("failed to make path by '%s'", name);
            continue;
        }

        if (!catcmd_catfile(self, stdout, path)) {
            ++ret;
            err_error("failed to catenate of '%s'", path);
            continue;
        }
    }

    return ret;
}
