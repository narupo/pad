#include <make/make.h>

/**
 * Structure of command
 */
struct makecmd {
    const config_t *config;
    int argc;
    char **argv;
    errstack_t *errstack;
};

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
make_from_args(
    const config_t *config,
    errstack_t *errstack,
    int argc,
    char *argv[],
    bool solve_path
) {
    bool use_stdin = false;
    if (argc < 2) {
        use_stdin = true;
    } else {
        if (argv[1] && argv[1][0] == '-') {
            use_stdin = true;
        }
    }

    char *src = NULL;

    if (use_stdin) {
        src = file_readcp(stdin);
        if (!src) {
            errstack_pushb(errstack, "failed to read from stdin");
            return 1;
        }
    } else {
        char path[FILE_NPATH];
        const char *cap_path = argv[1];

        if (solve_path) {
            if (!solve_cmdline_arg_path(config, path, sizeof path, cap_path)) {
                errstack_pushb(errstack, "failed to solve cap path");
                return 1;
            }            
        } else {
            cstr_copy(path, sizeof path, cap_path);
        }

        src = file_readcp_from_path(path);
        if (!src) {
            errstack_pushb(errstack, "failed to read from \"%s\"", path);
            return 1;
        }
    }

    char *compiled = compile_argv(
        config,
        errstack,
        argc - 1,
        argv + 1,
        src
    );
    if (!compiled) {
        errstack_pushb(
            errstack,
            "failed to compile from \"%s\"",
            (argv[1] ? argv[1] : "stdin")
        );
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

int
makecmd_run(makecmd_t *self) {
    int result = make_from_args(
        self->config,
        self->errstack,
        self->argc,
        self->argv,
        true
    );
    if (result != 0) {
        errstack_trace(self->errstack, stderr);
    }

    return result;
}
