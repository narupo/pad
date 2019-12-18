#include "snippet/snippet.h"

/**
 * Structure of command
 */
struct snptcmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to snptcmd_t
 */
static void
snptcmd_show_usage(snptcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Save or show snippet codes.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap snippet [commands] [arguments]\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    add     add snippet code\n"
        "    ls      show list of snippets\n"
        "    clear   clear all snippet codes\n"
        "\n"
    );
    fflush(stderr);
}

void
snptcmd_del(snptcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

snptcmd_t *
snptcmd_new(config_t *move_config, int argc, char **move_argv) {
    snptcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    return self;
}

static int
snptcmd_show_files(snptcmd_t *self) {
    file_dir_t *dir = file_diropen(self->config->codes_dir_path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *name = file_dirnodename(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        puts(name);
        file_dirnodedel(node);
    }

    file_dirclose(dir);
    return 0;
}

static int
snptcmd_add(snptcmd_t *self) {
    if (self->argc < 3) {
        err_error("failed to add snippet. need file name");
        return 1;
    }

    const char *name = self->argv[2];
    char path[FILE_NPATH];

    if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        err_error("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fout = file_open(path, "wb");
    if (!fout) {
        err_error("failed to open snippet \"%s\"", name);
        return 1;
    }

    for (int c; (c = fgetc(stdin)) != EOF; ) {
        fputc(c, fout);
    }

    fflush(fout);
    file_close(fout);
    return 0;
}

static int 
snptcmd_show(snptcmd_t *self) {
    if (self->argc < 2) {
        err_error("failed to show snippet. need file name");
        return 1;
    }

    const char *name = self->argv[1];
    char path[FILE_NPATH];

    if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        err_error("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fin = file_open(path, "rb");
    if (!fin) {
        err_error("failed to open snippet \"%s\"", name);
        return 1;
    }

    char *content = file_readcp(fin);
    if (!content) {
        err_error("failed to read snippet code from \"%s\"", path);
        return 1;
    }

    fclose(fin);

    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();
    opts_t *opts = opts_new();

    if (!opts_parse(opts, self->argc-2, self->argv+2)) {
        err_error("failed to show snippet. failed to parse options");
        return 1;
    }

    tkr_parse(tkr, content);
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
    free(content);

    return 0;
}

static int
snptcmd_clear(snptcmd_t *self) {
    file_dir_t *dir = file_diropen(self->config->codes_dir_path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *name = file_dirnodename(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        char path[FILE_NPATH];
        if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
            err_error("failed to solve path for \"%s\"", name);
            goto fail;
        }

        if (file_remove(path) != 0) {
            err_error("failed to remove file \"%s\"", path);
            goto fail;
        }

        file_dirnodedel(node);
    }

fail:
    file_dirclose(dir);
    return 0;
}

int
snptcmd_run(snptcmd_t *self) {
    if (self->argc < 2) {
        snptcmd_show_usage(self);
        return 0;
    } else if (cstr_eq(self->argv[1], "clear")) {
        return snptcmd_clear(self);
    } else if (cstr_eq(self->argv[1], "ls")) {
        return snptcmd_show_files(self);
    } else if (cstr_eq(self->argv[1], "add")) {
        return snptcmd_add(self);
    }
    return snptcmd_show(self);
}
