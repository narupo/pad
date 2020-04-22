#include <cat/cat.h>

/**
 * Structure of options
 */
struct opts {
    bool ishelp;
    int indent;
    int tabspaces;
    bool istab;
    bool ismake;
};

/**
 * Structure of command
 */
struct catcmd {
    const config_t *config;
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
        {"make", no_argument, 0, 'm'},
        {0},
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
        int cur = getopt_long(self->argc, self->argv, "hi:T:tm", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.ishelp = true; break;
        case 'i': self->opts.indent = atoi(optarg); break;
        case 'T': self->opts.tabspaces = atoi(optarg); break;
        case 't': self->opts.istab = true; break;
        case 'm': self->opts.ismake = true; break;
        case '?':
        default:
            err_die("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        return NULL;
    }

    self->optind = optind;

    return self;
}

/**
 * Destruct command
 *
 * @param[in] pointer to allocate memory of command
 */
void
catcmd_del(catcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

/**
 * Construct command
 *
 * @param[in] config pointer to config with move semantics
 * @param[in] argc   number of arguments
 * @param[in] argv   pointer to string array with move semantics
 * @return pointer to allocate memory of command
 */
catcmd_t *
catcmd_new(const config_t *config, int argc, char **argv) {
    catcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!catcmd_parse_opts(self)) {
        err_error("failed to parse options");
        catcmd_del(self);
        return NULL;
    }

    return self;
}

/**
 * Make file path for concatenate
 * 
 * @param[in] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *cap_path string of cap_path
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
static char *
catcmd_makepath(catcmd_t *self, char *dst, size_t dstsz, const char *cap_path) {
    const char *org = get_origin(self->config, cap_path);

    char drtpath[FILE_NPATH];
    snprintf(drtpath, sizeof drtpath, "%s/%s", org, cap_path);

    if (!symlink_follow_path(self->config, dst, dstsz, drtpath)) {
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
catcmd_setindent(catcmd_t *self, char *buf, size_t bufsize) {
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
 * Concatenate fin to fout
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
            if (!catcmd_setindent(self, str, sizeof str)) {
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
 * Show usage and exit from proccess
 *
 * @param[in] self
 */
static void
catcmd_usage(const catcmd_t *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap cat [options] [files]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -i, --indent     indent spaces.\n"
        "    -T, --tabspaces  number of tab spaces.\n"
        "    -t, --tab        tab indent mode.\n"
        "    -m, --make       with make.\n"
        "\n"
        "Examples:\n"
        "\n"
        "    $ cap cat f - g\n"
        "    $ cap cat\n"
        "\n"
    );
}

/**
 * Read content from stream
 *
 * @param[in] *self pointer to catcmd_t
 * @param[in] *fin pointer to FILE of source
 *
 * @return pointer to content
 */
static string_t *
catcmd_read_stream(catcmd_t *self, FILE *fin) {
    string_t *buf = str_new();

    for (;;) {
        int c = fgetc(fin);
        if (c == EOF) {
            break;
        }
        str_pushb(buf, c);
    }

    return buf;
}

/**
 * Write buffer at stream
 *
 * @param[in] *self pointer to catcmd_t
 * @param[in] *fout pointer to FILE of destination
 * @param[in] *buf pointer to buffer
 *
 * @return success to true, failed to false
 */
static bool
catcmd_write_stream(catcmd_t *self, FILE *fout, const string_t *buf) {
    bool ret = true;
    string_t *out = str_new();
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);
    const char *p = str_getc(buf);

    if (self->opts.ismake) {
        tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
        ast_t *ast = ast_new(self->config);

        tkr_parse(tkr, str_getc(buf));
        if (tkr_has_error(tkr)) {
            err_error("%s", tkr_get_error_detail(tkr));
            ret = false;
            goto fail;
        }

        cc_compile(ast, tkr_get_tokens(tkr));
        if (ast_has_error(ast)) {
            err_error("%s", ast_get_error_detail(ast));
            ret = false;
            goto fail;
        }

        trv_traverse(ast, ctx);
        if (ast_has_error(ast)) {
            err_error("%s", ast_get_error_detail(ast));
            ret = false;
            goto fail;        
        }

        tkr_del(tkr);
        ast_del(ast);

        p = ctx_getc_stdout_buf(ctx);
    }

    // set indent
    int m = 0;
    for (; *p; ) {
        char c = *p++;

        switch (m) {
        case 0: { // Indent mode
            char str[100] = {0};
            if (!catcmd_setindent(self, str, sizeof str)) {
                return false;
            }

            for (int i = 0; i < self->opts.indent; ++i) {
                str_app(out, str);
            }

            str_pushb(out, c);
            if (c != '\n') {
                m = 1;
            }
        } break;
        case 1: { // Stream mode
            str_pushb(out, c);
            if (c == '\n') {
                m = 0;
            }
        } break;
        }
    }

    printf("%s", str_getc(out));
    fflush(fout);

fail:
    ctx_del(ctx);
    gc_del(gc);
    str_del(out);
    return ret;
}

/**
 * Read content of file of path
 *
 * @param[in] *self pointer to catcmd_t
 * @param[in] *path path of file
 *
 * @return pointer to string_t
 */
static string_t *
catcmd_read_file(catcmd_t *self, const char *path) {
    FILE *fin = fopen(path, "rb");
    if (fin == NULL) {
        return NULL;
    }

    string_t *buf = catcmd_read_stream(self, fin);    

    if (fclose(fin) < 0) {
        str_del(buf);
        return NULL;
    }

    return buf;
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
        string_t *stdinbuf = catcmd_read_stream(self, stdin);
        catcmd_write_stream(self, stdout, stdinbuf);
        str_del(stdinbuf);
        return 0;
    }

    int ret = 0;
    for (int i = self->optind; i < self->argc; ++i) {
        const char *name = self->argv[i];

        if (strcmp(name, "-") == 0) {
            string_t *stdinbuf = catcmd_read_stream(self, stdin);
            catcmd_write_stream(self, stdout, stdinbuf);
            str_del(stdinbuf);
            continue;
        }
        
        char path[FILE_NPATH];
        if (!catcmd_makepath(self, path, sizeof path, name)) {
            ++ret;
            err_error("failed to make path by \"%s\"", name);
            continue;
        }

        string_t *filebuf = catcmd_read_file(self, path);
        if (!filebuf) {
            ++ret;
            err_error("failed to read file from \"%s\"", path);
            continue;
        }
        catcmd_write_stream(self, stdout, filebuf);
        str_del(filebuf);
    }

    return ret;
}
