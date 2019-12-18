#include "make/make.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct makecmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to makecmd_t
 */
static void
makecmd_show_usage(makecmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap make [file]\n"
        "\n"
    );
    fflush(stderr);
}

void
makecmd_del(makecmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

makecmd_t *
makecmd_new(config_t *move_config, int argc, char **move_argv) {
    makecmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    return self;
}

int
makecmd_run(makecmd_t *self) {
    bool use_stdin = false;
    if (self->argc < 2) {
        use_stdin = true;
    } else {
        if (self->argv[1] && self->argv[1][0] == '-') {
            use_stdin = true;
        }
    }

    char *src = NULL;

    if (use_stdin) {
        src = file_readcp(stdin);
        if (!src) {
            err_error("failed to read from stdin");
            return 1;
        }
    } else {
        const char *cap_path = self->argv[1];
        const char *org = get_origin(self->config, cap_path);
        char drtpath[FILE_NPATH];
        snprintf(drtpath, sizeof drtpath, "%s/%s", org, cap_path);

        char path[FILE_NPATH];
        if (!symlink_follow_path(self->config, path, sizeof path, drtpath)) {
            err_error("failed to follow path \"%s\"", cap_path);
            return 1;
        }

        src = file_readcp_from_path(path);
        if (!src) {
            err_error("failed to read from \"%s\"", path);
            return 1;
        }        
    }

    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();
    opts_t *opts = opts_new();

    if (!opts_parse(opts, self->argc, self->argv)) {
        err_error("failed to parse options");
        return 1;        
    }

    tkr_parse(tkr, src);
    if (tkr_has_error(tkr)) {
        err_error("failed to parse tokens. %s", tkr_get_error_detail(tkr));
        return 1;
    }

    ast_move_opts(ast, opts);
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_error(ast)) {
        err_error("failed to parse AST. %s", ast_get_error_detail(ast));
        return 1;
    }

    trv_traverse(ast, ctx);
    if (ast_has_error(ast)) {
        err_error("failed to traverse AST. %s", ast_get_error_detail(ast));
        return 1;        
    }

    printf("%s", ctx_getc_buf(ctx));
    fflush(stdout);

    tkr_del(tkr);
    ast_del(ast);
    ctx_del(ctx);
    free(src);
    return 0;
}
