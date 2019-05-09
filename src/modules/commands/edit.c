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
        {0},
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

editcmd_t *
editcmd_read_editor(editcmd_t *self) {
    self->editor[0] = '\0';
    if (!file_readline(self->editor, sizeof self->editor, self->config->var_editor_path)) {
        return NULL;
    }
    if (!strlen(self->editor)) {
        return NULL;
    }
    return self;
}

editcmd_t *
editcmd_create_open_fname(editcmd_t *self, const char *fname) {
    char path[FILE_NPATH];
    const char *srcpath;

    if (fname[0] == '/') {
        srcpath = self->config->var_home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        srcpath = self->config->var_cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        srcpath = self->config->var_home_path;
    } else {
        err_die("impossible. invalid scope");
        return NULL;
    }

    if (!file_readline(path, sizeof path, srcpath)) {
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

    if (!editcmd_read_editor(self)) {
        err_die("not found editor. please setting with 'cap editor' command");
        return 1;
    }

    cstr_app(self->cmdline, sizeof self->cmdline, self->editor);
    if (fname) {
        if (!editcmd_create_open_fname(self, fname)) {
            err_die("failed to create open file name");
            return 2;
        }
        cstr_app(self->cmdline, sizeof self->cmdline, " ");
        cstr_app(self->cmdline, sizeof self->cmdline, self->open_fname);
    }

    safesystem(self->cmdline, SAFESYSTEM_EDIT);

    return 0;
}
