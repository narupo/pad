#include "modules/commands/edit.h"

struct editcmd {
    config_t *config;
    int argc;
    char **argv;
    char editor[1024];
    char cmdline[2048];
    char open_fname[1024];
};

void
editcmd_del(editcmd_t *self) {
    if (self == NULL) {
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

    return self;
}

char *
editcmd_create_resouce_path(editcmd_t *self, char *path, uint32_t pathsz, int scope) {
    char srcpath[FILE_NPATH];

    if (scope == CAP_SCOPE_LOCAL) {
        if (file_readline(srcpath, sizeof srcpath, self->config->var_cd_path) == NULL) {
            err_die("failed to read line from cd of variable");
        }
    } else if (scope == CAP_SCOPE_GLOBAL) {
        if (file_readline(srcpath, sizeof srcpath, self->config->var_home_path) == NULL) {
            err_die("failed to read line from home of variable");
        }
    } else {
        err_die("impossible. invalid scope");
        return NULL;
    }

    if (file_solvefmt(path, pathsz, "%s/.caprc", srcpath) == NULL) {
        err_die("failed to solve path");
        return NULL;
    }

    if (!file_exists(path)) {
        return NULL;
    }

    return path;
}

editcmd_t *
editcmd_read_editor(editcmd_t *self) {
    // parse resource file
    int scope = CAP_SCOPE_LOCAL;
    char path[FILE_NPATH];

    if (editcmd_create_resouce_path(self, path, sizeof path, scope) == NULL) {
        scope = CAP_SCOPE_GLOBAL;
        if (editcmd_create_resouce_path(self, path, sizeof path, scope) == NULL) {
            err_error("failed to create resource path");
            return NULL;
        }
    }

    char *content = file_readcp_from_path(path);
    if (content == NULL) {
        err_die("failed to read content from \"%s\"", path);
        return NULL;
    }

    editcmd_t *ret = self;
    tokenizer_t *tkr = tkr_new();
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();

    if (tkr_parse(tkr, content) == NULL) {
        ret = NULL;
        goto fail;
    }

    if (ast_parse(ast, tkr_get_tokens(tkr)) == NULL) {
        ret = NULL;
        goto fail;
    }

    ast_traverse(ast, ctx);

    const dict_t *confmap = ctx_getc_confmap(ctx);
    const dict_item_t *item = dict_getc(confmap, "editor");
    if (item == NULL) {
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
    if (file_readline(path, sizeof path, self->config->var_cd_path) == NULL) {
        return NULL;
    }

    if (file_solvefmt(self->open_fname, sizeof self->open_fname, "%s/%s", path, fname) == NULL) {
        return NULL;
    }

    return self;
}

int
editcmd_run(editcmd_t *self) {
    const char *fname = NULL;
    if (self->argc >= 2) {
        fname = self->argv[1];
    }

    if (editcmd_read_editor(self) == NULL) {
        err_die("not found editor. please setting at resource file");
        return 1;
    }

    cstr_cat(self->cmdline, sizeof self->cmdline, self->editor);
    if (fname != NULL) {
        if (editcmd_create_open_fname(self, fname) == NULL) {
            err_die("failed to create open file name");
            return 2;
        }        
        cstr_cat(self->cmdline, sizeof self->cmdline, " ");
        cstr_cat(self->cmdline, sizeof self->cmdline, self->open_fname);
    }

    safesystem(self->cmdline);

    return 0;
}
