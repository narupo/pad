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
#ifdef _CAP_WINDOWS
    string_t *read_buffer;
#endif
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
#if _CAP_WINDOWS
    str_del(self->read_buffer);
#endif
    free(self);
}

execcmd_t *
execcmd_new(config_t *move_config, int argc, char **move_argv) {
    execcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;
    self->cmdline = cmdline_new();
#if _CAP_WINDOWS
    self->read_buffer = str_new();
#endif

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
    const cmdline_object_t *first = cmdline_getc(self->cmdline, 0);
    const char *cmd = str_getc(first->command);
    safesystem(cmd, SAFESYSTEM_UNSAFE);
    return self;
}

#ifdef _CAP_WINDOWS
execcmd_t *
execcmd_create_read_pipe(execcmd_t *self, HANDLE process, HANDLE *read_handle, HANDLE *write_handle) {
    HANDLE tmp_read_handle = NULL;
    
    if (!CreatePipe(&tmp_read_handle, write_handle, NULL, 0)) {
        execcmd_set_error(self, "failed to create pipe");
        return NULL;
    }

    if (!DuplicateHandle(
        process,
        tmp_read_handle,
        process,
        read_handle,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS)) {
        execcmd_set_error(self, "failed to duplicate handle");
        CloseHandle(tmp_read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    if (!CloseHandle(tmp_read_handle)) {
        execcmd_set_error(self, "failed to close handle");
        CloseHandle(*read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    return self;
}

execcmd_t *
execcmd_create_write_pipe(execcmd_t *self, HANDLE process, HANDLE *read_handle, HANDLE *write_handle) {
    HANDLE tmp_write_handle = NULL;
    
    if (!CreatePipe(read_handle, &tmp_write_handle, NULL, 0)) {
        execcmd_set_error(self, "failed to create pipe");
        return NULL;
    }

    if (!DuplicateHandle(
        process,
        tmp_write_handle,
        process,
        write_handle,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS)) {
        execcmd_set_error(self, "failed to duplicate handle");
        CloseHandle(tmp_write_handle);
        CloseHandle(*read_handle);
        return NULL;
    }

    if (!CloseHandle(tmp_write_handle)) {
        execcmd_set_error(self, "failed to close handle");
        CloseHandle(*read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    return self;
}

execcmd_t *
execcmd_pipe(execcmd_t *self, const cmdline_object_t *obj, const cmdline_object_t *ope) {
    HANDLE hs1[2] = {0};
    HANDLE hs2[2] = {0};
    HANDLE process = GetCurrentProcess();

#define close_hs1() { \
        CloseHandle(hs1[0]); \
        CloseHandle(hs1[1]); \
    }
#define close_hs2() { \
        CloseHandle(hs2[0]); \
        CloseHandle(hs2[1]); \
    }
#define close_hs() { \
        close_hs1(); \
        close_hs2(); \
    }

    if (!execcmd_create_read_pipe(self, process, &hs1[0], &hs1[1])) {
        return NULL;
    }

    if (!execcmd_create_write_pipe(self, process, &hs2[0], &hs2[1])) {
        close_hs1();
        return NULL;
    }

    // create process
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hs1[0];
    si.hStdOutput = hs2[1];
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (si.hStdOutput == INVALID_HANDLE_VALUE) {
        execcmd_set_error(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }
    if (si.hStdError == INVALID_HANDLE_VALUE) {
        execcmd_set_error(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }

    LPTSTR cmdline = (LPTSTR) str_getc(obj->command);
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcess(
        NULL,
        cmdline,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        execcmd_set_error(self, "failed to create process");
        return NULL;
    }

    HANDLE child_process = pi.hProcess;
    if (!CloseHandle(pi.hThread)) {
        execcmd_set_error(self, "failed to close thread handle");
        close_hs();
        return NULL;
    }

    CloseHandle(hs1[0]);
    hs1[0] = NULL;

    CloseHandle(hs2[1]);
    hs2[1] = NULL;

    // write to pipe (parent write to child process)
    DWORD nwrite = 0;
    WriteFile(hs1[1], str_getc(self->read_buffer), str_len(self->read_buffer), &nwrite, NULL);
    CloseHandle(hs1[1]);

    // read from pipe (parent read from child process)
    char buf[512];
    DWORD nread = 0;

    str_clear(self->read_buffer);
    for (;;) {
        memset(buf, 0, sizeof buf);
        ReadFile(hs2[0], buf, sizeof buf, &nread, NULL);
        if (nread == 0) {
            break;
        }

        str_app(self->read_buffer, buf);
    }
    CloseHandle(hs2[0]);

    // wait for child process
    DWORD result = WaitForSingleObject(child_process, INFINITE);
    switch (result) {
    default:
        execcmd_set_error(self, "failed to wait");
        close_hs();
        CloseHandle(child_process);
        return NULL;
        break;
    case WAIT_OBJECT_0: // success
        close_hs();
        CloseHandle(child_process);
        if (!ope) {
            printf("%s", str_getc(self->read_buffer));
            fflush(stdout);
        }
        return self;
        break;
    }

    return NULL; // impossible
}

execcmd_t *
execcmd_exec_all_win(execcmd_t *self) {
    for (int32_t i = 0; i < cmdline_len(self->cmdline); i += 2) {
        const cmdline_object_t *obj = cmdline_getc(self->cmdline, i);
        const cmdline_object_t *ope = cmdline_getc(self->cmdline, i+1);

        if (ope && ope->type == CMDLINE_OBJECT_TYPE_AND) {
            int exit_code = safesystem(str_getc(obj->command), SAFESYSTEM_DEFAULT);
            if (exit_code != 0) {
                break;
            }
        } else {
            if (!execcmd_pipe(self, obj, ope)) {
                return NULL;
            }
        }
    }

    return self;
}
#else

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
#endif

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
