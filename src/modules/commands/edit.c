#include "modules/commands/edit.h"

struct opts {
    bool is_global;
};

struct editcmd {
    config_t *config;
    struct opts opts;
    int argc;
    char **argv;
    char editor[1024];
    char cmdline[2048];
    char open_fname[1024];
};

editcmd_t *
editcmd_parse_opts(editcmd_t *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"global", no_argument, 0, 'g'},
        {},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hg", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': /* Help */ break;
        case 'g': self->opts.is_global = true; break;
        case '?':
        default:
            err_error("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        err_error("failed to parse option");
        return NULL;
    }

    return self;
}

void
editcmd_del(editcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

editcmd_t *
editcmd_new(config_t *move_config, int argc, char **move_argv) {
    editcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!editcmd_parse_opts(self)) {
        err_die("failed to parse options");
    }

    return self;
}

char *
editcmd_create_resouce_path(editcmd_t *self, char *path, uint32_t pathsz, int scope) {
    char srcpath[FILE_NPATH];

    if (scope == CAP_SCOPE_LOCAL) {
        if (!file_readline(srcpath, sizeof srcpath, self->config->var_cd_path)) {
            err_die("failed to read line from cd of variable");
        }
    } else if (scope == CAP_SCOPE_GLOBAL) {
        if (!file_readline(srcpath, sizeof srcpath, self->config->var_home_path)) {
            err_die("failed to read line from home of variable");
        }
    } else {
        err_die("impossible. invalid scope");
        return NULL;
    }

    if (!file_solvefmt(path, pathsz, "%s/.caprc", srcpath)) {
        err_die("failed to solve path");
        return NULL;
    }

    if (!file_exists(path)) {
        return NULL;
    }

    return path;
}

editcmd_t *
editcmd_read_editor(editcmd_t *self, int scope) {
    // parse resource file
    char path[FILE_NPATH];

    if (!editcmd_create_resouce_path(self, path, sizeof path, scope)) {
        return NULL;
    }

    char *content = file_readcp_from_path(path);
    if (!content) {
        err_die("failed to read content from \"%s\"", path);
        return NULL;
    }

    editcmd_t *ret = self;
    tokenizer_t *tkr = tkr_new();
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();

    if (!tkr_parse(tkr, content)) {
        ret = NULL;
        goto fail;
    }

    if (!ast_parse(ast, tkr_get_tokens(tkr))) {
        ret = NULL;
        goto fail;
    }

    ast_traverse(ast, ctx);

    const dict_t *confmap = ctx_getc_confmap(ctx);
    const dict_item_t *item = dict_getc(confmap, "editor");
    if (!item) {
        ret = NULL;
        goto fail;
    }

    cstr_copy(self->editor, sizeof self->editor, item->value);

fail:
    free(content);
    tkr_del(tkr);
    ast_del(ast);
    ctx_del(ctx);
    return ret;
}

editcmd_t *
editcmd_create_open_fname(editcmd_t *self, const char *fname) {
    char path[FILE_NPATH];
    if (!file_readline(path, sizeof path, self->config->var_cd_path)) {
        return NULL;
    }

    if (!file_solvefmt(self->open_fname, sizeof self->open_fname, "%s/%s", path, fname)) {
        return NULL;
    }

    return self;
}

int
editcmd_run(editcmd_t *self) {
    const char *fname = NULL;
    if (self->argc >= 2) {
        fname = self->argv[optind];
    }

    if (self->opts.is_global) {
        if (!editcmd_read_editor(self, CAP_SCOPE_GLOBAL)) {
            err_die("not found editor. please setting at resource file");
            return 1;
        }        
    } else {
        if (!editcmd_read_editor(self, CAP_SCOPE_LOCAL)) {
            // not found at local, next find at global
            if (!editcmd_read_editor(self, CAP_SCOPE_GLOBAL)) {
                // editor is not setting
                err_die("not found editor. please setting at resource file");
                return 1;
            }
        }        
    }

    cstr_cat(self->cmdline, sizeof self->cmdline, self->editor);
    if (fname) {
        if (!editcmd_create_open_fname(self, fname)) {
            err_die("failed to create open file name");
            return 2;
        }        
        cstr_cat(self->cmdline, sizeof self->cmdline, " ");
        cstr_cat(self->cmdline, sizeof self->cmdline, self->open_fname);
    }

    safesystem(self->cmdline, SAFESYSTEM_EDIT);

    return 0;
}

