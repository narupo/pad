#include "exec/exec.h"

static const int32_t READ = 0;
static const int32_t WRITE = 1;

/**
 * Structure of options
 */
struct opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct exec {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    cmdline_t *cmdline;
    int32_t cmdline_index;
    char what[1024];
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to execcmd_t
 */
static void
execcmd_show_usage(execcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap exec [options]... [command-line]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    Show usage\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to execcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
execcmd_parse_opts(execcmd_t *self) {
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
        int cur = getopt_long(self->argc, self->argv, "hf:", longopts, &optsindex);
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
execcmd_del(execcmd_t *self) {
    if (!self) {
        return;
    }

    config_del(self->config);
    freeargv(self->argc, self->argv);
    cmdline_del(self->cmdline);
    free(self);
}

execcmd_t *
execcmd_new(config_t *move_config, int argc, char **move_argv) {
    execcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;
    self->cmdline = cmdline_new();

    if (!execcmd_parse_opts(self)) {
        execcmd_del(self);
        return NULL;
    }

    return self;
}

void
execcmd_set_error(execcmd_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->what, sizeof self->what, fmt, ap);
    va_end(ap);
}

execcmd_t *
execcmd_exec_first(execcmd_t *self) {
    if (cmdline_len(self->cmdline) != 1) {
        return NULL;
    }

    const cmdline_object_t *obj = cmdline_getc(self->cmdline, 0);
    if (!obj) {
        return NULL;
    }

    if (obj->type != CMDLINE_OBJECT_TYPE_CMD) {
        return NULL;
    }

    const char *cmd = str_getc(obj->command);

    puts(cmd);

    return self;
}

execcmd_t *
execcmd_exec_all_win(execcmd_t *self) {
    return self;
}

execcmd_t *
execcmd_exec_all_unix_pipe_advance(execcmd_t *self) {
    if (self->cmdline_index >= cmdline_len(self->cmdline)) {
        return self; // success
    }

    bool is_last_cmd = cmdline_getc(self->cmdline, self->cmdline_index+1) == NULL;
    int fd[2] = {0};
    int pid = -1;

    if (pipe(fd) != 0) {
        execcmd_set_error(self, "failed to create pipe (%d)", self->cmdline_index);
        return NULL;
    }

    if ((pid = fork()) < 0) {
        execcmd_set_error(self, "failed to fork (%d)", self->cmdline_index);
        close(fd[READ]);
        close(fd[WRITE]);
        return NULL;
    }

    // printf("cmdline index[%d] is_last_cmd[%d]\n", self->cmdline_index, is_last_cmd);

    switch (pid) {
    default: { // parent
        int stdoutno = dup(STDOUT_FILENO);

        const cmdline_object_t *obj = cmdline_getc(self->cmdline, self->cmdline_index);
        if (!obj) {
            execcmd_set_error(self, "command line object is null");
            close(fd[WRITE]);
            dup2(stdoutno, STDOUT_FILENO);
            return NULL;
        }
        // printf("parent command[%s]\n", str_getc(obj->command));

        close(fd[READ]);
        if (is_last_cmd) {
            // this is last command. do not pipe stdout
            close(fd[WRITE]);
        } else {
            dup2(fd[WRITE], STDOUT_FILENO); // connect to stdout
        }

        safesystem(str_getc(obj->command), SAFESYSTEM_DEFAULT);
        fflush(stdout);

        if (!is_last_cmd) {
            dup2(stdoutno, STDOUT_FILENO);
        }

        fflush(stdout);
    } break;
    case 0: { // child
        close(fd[WRITE]);
        int stdinno = dup(STDIN_FILENO);
        dup2(fd[READ], STDIN_FILENO); // connect to stdin

        const cmdline_object_t *ope = cmdline_getc(self->cmdline, self->cmdline_index+1);
        if (!ope) {
            execcmd_set_error(self, "command line operator is null");
            close(fd[READ]);
            dup2(stdinno, STDIN_FILENO);
            return self; // success
        }

        switch (ope->type) {
        default:
            execcmd_set_error(self, "invalid operator type");
            close(fd[READ]);
            dup2(stdinno, STDIN_FILENO);
            return NULL;
            break;
        case CMDLINE_OBJECT_TYPE_PIPE:
            self->cmdline_index += 2;
            execcmd_t *result = execcmd_exec_all_unix_pipe_advance(self);
            close(fd[READ]);
            dup2(stdinno, STDIN_FILENO);
            return result;
            break;
        }
    } break;
    }

    return self;
}

execcmd_t *
do_pipe(execcmd_t *self) {
    int stdinno = dup(STDIN_FILENO);
    int stdoutno = dup(STDOUT_FILENO);

    // printf("%d: do pipe\n", getpid());

    for (int32_t i = 0; i < cmdline_len(self->cmdline); i += 2) {
        // printf("i[%d]\n", i);
        const cmdline_object_t *obj = cmdline_getc(self->cmdline, i);
        const cmdline_object_t *ope = cmdline_getc(self->cmdline, i+1);

        int fd[2] = {0};
        if (pipe(fd) != 0) {
            execcmd_set_error(self, "failed to create pipe");
            return NULL;
        }

        pid_t pid = fork();
        // printf("%d: forked\n", pid);
        switch (pid) {
        default: { // parent
            close(fd[READ]);
            if (ope) {
                dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[WRITE]);
            } else {
                close(fd[WRITE]);
            }
            safesystem(str_getc(obj->command), SAFESYSTEM_DEFAULT);
            dup2(stdoutno, STDOUT_FILENO);
            dup2(stdinno, STDIN_FILENO);
            wait(NULL);
            goto done;
        } break;
        case 0: { // child
            close(fd[WRITE]);
            dup2(fd[READ], STDIN_FILENO);
            close(fd[READ]);
        } break;
        case -1: { // error
            execcmd_set_error(self, "failed to fork");
            return NULL;
        } break;
        }
    }

    close(stdinno);
    close(stdoutno);
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
done:
    return self; // impossible
}

execcmd_t *
execcmd_exec_all_unix_pipe(execcmd_t *self, const cmdline_object_t *lhs, const cmdline_object_t *rhs) {
    int fd[2] = {0};
    int pid = -1;

    if (pipe(fd) != 0) {
        execcmd_set_error(self, "failed to create pipe");
        return NULL;
    }

    if ((pid = fork()) < 0) {
        execcmd_set_error(self, "failed to fork");
        close(fd[READ]);
        close(fd[WRITE]);
        return NULL;
    }

    switch (pid) {
    default: { // parent
        close(fd[READ]);
        dup2(fd[WRITE], 1); // connect to stdout

        safesystem(str_getc(lhs->command), SAFESYSTEM_DEFAULT);
        fflush(stdout);

        dup2(STDOUT_FILENO, 1);
    } break;
    case 0: { // child
        close(fd[WRITE]);
        dup2(fd[READ], 0); // connect to stdin

        safesystem(str_getc(rhs->command), SAFESYSTEM_DEFAULT);
        fflush(stdout);

        self->cmdline_index = 3;
        return execcmd_exec_all_unix_pipe_advance(self);
    } break;
    }

    return self;
}

/**
 * a | b | c
 */
execcmd_t *
execcmd_exec_all_unix(execcmd_t *self) {
    return self;
}

execcmd_t *
execcmd_exec_all(execcmd_t *self) {
    if (cmdline_len(self->cmdline) < 3) {
        execcmd_set_error(self, "too few command line objects");
        return NULL;
    }

#ifdef _CAP_WINDOWS
    return execcmd_exec_all_win(self);
#else
    return do_pipe(self);
#endif
}

execcmd_t *
execcmd_exec(execcmd_t *self, const char *cltxt) {
    if (!self || !cltxt) {
        return NULL;
    }

    if (!cmdline_parse(self->cmdline, cltxt)) {
        execcmd_set_error(self, "failed to parse command line");
        return NULL;
    }

    if (cmdline_len(self->cmdline) == 1) {
        return execcmd_exec_first(self);
    } else {
        return execcmd_exec_all(self);
    }

    return self;
}

int
execcmd_run(execcmd_t *self) {
    if (self->argc - self->optind == 0 ||
        self->opts.is_help) {
        execcmd_show_usage(self);
        return 1;
    }

    for (int32_t i = self->optind; i < self->argc; ++i) {
        const char *cltxt = self->argv[i];
        if (!execcmd_exec(self, cltxt)) {
            err_error(self->what);
            return 1;
        }
    }

    return 0;
}
