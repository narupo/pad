#include <make/make.h>

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
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    errstack_t *errstack;
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
        "    cap make [file] [file-arguments]...\n"
        "\n"
    );
    fflush(stderr);
}

void
makecmd_del(makecmd_t *self) {
    if (!self) {
        return;
    }

    errstack_del(self->errstack);
    free(self);
}

makecmd_t *
makecmd_new(const config_t *config, int argc, char **argv) {
    makecmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = errstack_new();

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
        char path[FILE_NPATH];
        const char *cap_path = self->argv[1];

        if (!solve_cmdline_arg_path(self->config, path, sizeof path, cap_path)) {
            err_error("failed to solve cap path");
            return 1;
        }

        src = file_readcp_from_path(path);
        if (!src) {
            err_error("failed to read from \"%s\"", path);
            return 1;
        }
    }

    char *compiled = compile_argv(
        self->config,
        self->errstack,
        self->argc-1,
        self->argv+1,
        src
    );
    if (!compiled) {
        errstack_pushb(self->errstack, __FILE__, __LINE__, __func__,
            "failed to compile from \"%s\"", (self->argv[1] ? self->argv[1] : "stdin"));
        errstack_trace(self->errstack, stderr);
        fflush(stderr);
        free(src);
        return 1;
    }

    printf("%s", compiled);
    fflush(stdout);

    free(compiled);
    free(src);
    return 0;
}
