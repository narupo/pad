#include <editor/editor.h>

/**
 * structure of options
 */
struct opts {
    bool is_help;
};

/**
 * structure of command
 */
struct editorcmd {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    char editor[FILE_NPATH];
};

/**
 * show usage of command
 *
 * @param[in] self pointer to editorcmd_t
 */
static void
editorcmd_show_usage(editorcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap editor [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * parse options
 *
 * @param[in] self pointer to editorcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
editorcmd_parse_opts(editorcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case '?':
        default:
            err_die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
editorcmd_del(editorcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

editorcmd_t *
editorcmd_new(const config_t *config, int argc, char **argv) {
    editorcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!editorcmd_parse_opts(self)) {
        editorcmd_del(self);
        return NULL;
    }

    return self;
}

int
editorcmd_show_editor(editorcmd_t *self) {
    self->editor[0] = '\0';
    if (!file_readline(self->editor, sizeof self->editor, self->config->var_editor_path)) {
        err_error("failed to read editor from editor of variable");
        return 1;
    }
    if (strlen(self->editor)) {
        puts(self->editor);
    }
    return 0;
}

int
editorcmd_set_editor(editorcmd_t *self) {
    const char *editor = self->argv[self->optind];
    if (!file_writeline(editor, self->config->var_editor_path)) {
        err_error("failed to write editor into editor of variable");
        return 1;
    }
    return 0;
}

int
editorcmd_run(editorcmd_t *self) {
    if (self->opts.is_help) {
        editorcmd_show_usage(self);
        return 0;
    }

    if (self->argc < self->optind+1) {
        editorcmd_show_editor(self);
        return 0;
    }

    return editorcmd_set_editor(self);
}
