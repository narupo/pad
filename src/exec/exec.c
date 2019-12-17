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

/*
cl_t *
execcmd_solve_cl(execcmd_t *self, const cl_t *cl) {
    cl_t *dstcl = cl_new();

    if (!cl_len(cl)) {
        return dstcl;
    }

    const char *first = cl_getc(cl, 0);
    char path[FILE_NPATH];
    if (!symlink_follow_path(self->config, path, sizeof path, first)) {
        execcmd_set_error(self, "failed to follow path");
        return NULL;
    }

    cl_push(dstcl, path);

    for (int32_t i = 1; i < cl_len(cl); ++i) {
        const char *el = cl_getc(cl, i);
        cl_push(dstcl, el);
    }

    return dstcl;
}

execcmd_t *
execcmd_exec_first(execcmd_t *self) {
    if (cmdline_len(self->cmdline) != 1) {
        return NULL;
    }

    const cmdline_object_t *obj = cmdline_getc(self->cmdline, 0);
    if (!obj) {
        execcmd_set_error(self, "object is null. do not execute first");
        return NULL;
    }

    if (obj->type != CMDLINE_OBJECT_TYPE_CMD) {
        execcmd_set_error(self, "invalid object type. do not execute first");
        return NULL;
    }

    cl_t *cl = execcmd_solve_cl(self, obj->cl);
    if (!cl) {
        execcmd_set_error(self, "failed to solve command line");
        return NULL;
    }

    char *cmdline = cl_to_string(cl);
    cl_del(cl);

    printf("app[%s] cmdline[%s]\n", self->config->app_path, cmdline);
    // safesystem(cmdline, SAFESYSTEM_DEFAULT);

    free(cmdline);
    return self;
}
*/

execcmd_t *
execcmd_exec_first(execcmd_t *self) {
    const cmdline_object_t *first = cmdline_getc(self->cmdline, 0);
    const char *cmd = str_getc(first->command);
    safesystem(cmd, SAFESYSTEM_UNSAFE);
    return self;
}

execcmd_t *
execcmd_exec_all_win(execcmd_t *self) {
    return self;
}

execcmd_t *
execcmd_exec_all_unix(execcmd_t *self) {
    int stdinno = dup(STDIN_FILENO);
    int stdoutno = dup(STDOUT_FILENO);
    int exit_code = 0;

    for (int32_t i = 0; i < cmdline_len(self->cmdline); i += 2) {
        const cmdline_object_t *obj = cmdline_getc(self->cmdline, i);
        const cmdline_object_t *ope = cmdline_getc(self->cmdline, i+1);

        if (ope && ope->type == CMDLINE_OBJECT_TYPE_AND) {
            const char *cmd = str_getc(obj->command);
            int status = safesystem(cmd, SAFESYSTEM_UNSAFE);
            exit_code = WEXITSTATUS(status);            
            if (exit_code != 0) {
                break;
            }
        } else {
            int fd[2] = {0};
            if (pipe(fd) != 0) {
                execcmd_set_error(self, "failed to create pipe");
                return NULL;
            }

            pid_t pid = fork();
            switch (pid) {
            default: { // parent
                close(fd[READ]);
                if (ope) {
                    dup2(fd[WRITE], STDOUT_FILENO);
                    close(fd[WRITE]);
                } else {
                    close(fd[WRITE]);
                }

                const char *cmd = str_getc(obj->command);
                int status = safesystem(cmd, SAFESYSTEM_UNSAFE);
                exit_code = WEXITSTATUS(status);
                (void) &exit_code;

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
                close(fd[READ]);
                close(fd[WRITE]);
                return NULL;
            } break;
            }
        }
    }

    close(stdinno);
    close(stdoutno);
done:
    return self; // impossible
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
    return execcmd_exec_all_unix(self);
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

char *
unescape_cl(char *dst, int32_t dstsz, const char *escaped) {
    char *dp = dst;
    char *dend = dst + dstsz-1;

    for (const char *p = escaped; *p && dp < dend; ++p) {
        if (*p == '\\') {
            // pass
        } else {
            *dp++ = *p;
        }
    }

    *dp = '\0';
    return dst;
}

int
execcmd_run(execcmd_t *self) {
    if (self->argc - self->optind == 0 ||
        self->opts.is_help) {
        execcmd_show_usage(self);
        return 1;
    }

    for (int32_t i = self->optind; i < self->argc; ++i) {
        const char *escaped_cltxt = self->argv[i];
        char cltxt[1024];
        unescape_cl(cltxt, sizeof cltxt, escaped_cltxt);

        if (!execcmd_exec(self, cltxt)) {
            err_error(self->what);
            return 1;
        }
    }

    return 0;
}
