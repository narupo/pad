/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include <pad/core/util.h>

void
freeargv(int argc, char *argv[]) {
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            free(argv[i]);
        }
        free(argv);
    }
}

void
showargv(int argc, char *argv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = [%s]\n", i, argv[i]);
    }
}

int
randrange(int min, int max) {
    return min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
}

int
safesystem(const char *cmdline, int option) {
    if (option & SAFESYSTEM_UNSAFE) {
        return system(cmdline);
    }

#ifdef _CAP_WINDOWS
    int flag = 0;
    if (option & SAFESYSTEM_EDIT) {
        // option for edit command
        flag = CREATE_NEW_CONSOLE;
    } else {
        flag = 0;
    }

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = { sizeof(STARTUPINFO) };

    // Start the child process.
    if (!CreateProcess(NULL, // No module name (use command line)
        (char *) cmdline, // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        flag, // No creation flags
        NULL, // Use parent's environment block
        NULL, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi) // Pointer to PROCESS_INFORMATION structure
    ) {
        err_error("failed to create sub process");
        return 1;
    }

    if (option & SAFESYSTEM_EDIT) {
        // case of edit command, to not wait exit of child process
        return 0;
    }

    if (option & SAFESYSTEM_DETACH) {
        // not wait child process
        return 0;
    }

    // success to fork
    HANDLE child_process = pi.hProcess;
    if (!CloseHandle(pi.hThread)) {
        err_error("failed to close handle");
        return 1;
    }

    // wait for child process
    DWORD r = WaitForSingleObject(child_process, INFINITE);
    switch(r) {
    case WAIT_FAILED:
        err_error("child process was failed");
        return 1;
    case WAIT_ABANDONED:
        err_error("child process was abandoned");
        return 1;
    case WAIT_OBJECT_0: // success
        break;
    case WAIT_TIMEOUT:
        err_error("child process was timeout");
        return 1;
    default:
        return 1;
    }

    // get exit code of child process
    DWORD exit_code;
    if (!GetExitCodeProcess(child_process, &exit_code)) {
        err_error("failed to get exit code of child process");
        return 1;
    }

    return exit_code;

#else
    cl_t *cl = cl_new();
    if (!cl_parse_str_opts(cl, cmdline, 0)) {
        err_error("failed to parse command line \"%s\"", cmdline);
        cl_del(cl);
        return -1;
    }

    int argc = cl_len(cl);
    char **argv = cl_escdel(cl);
    if (!argv) {
        err_error("failed to escape and delete of clk");
        return -1;
    }

    int status = 0;

    switch (fork()) {
    default: // parent
        freeargv(argc, argv);
        wait(&status);
        break;
    case 0: // child
        if (execv(argv[0], argv) == -1) {
            err_error("failed to safesystem");
            freeargv(argc, argv);
            _exit(1);
        }
        break;
    case -1: // error
        err_error("failed to fork");
        return -1;
        break;
    }

    return status;
#endif
}

cstring_array_t *
argsbyoptind(int argc, char *argv[], int optind) {
    cstring_array_t *args = cstrarr_new();

    // DO NOT DELETE FOR DEBUG.
    //
    // printf("argc[%d] optind[%d]\n", argc, optind);
    // for (int i = 0; i < argc; ++i) {
    // 	printf("%d %s\n", i, argv[i]);
    // }

    cstrarr_push(args, argv[0]);
    for (int i = optind; i < argc; ++i) {
        cstrarr_push(args, argv[i]);
    }

    return args;
}

char *
trim_first_line(char *dst, int32_t dstsz, const char *text) {
    if (!dst || !dstsz || !text) {
        return NULL;
    }

    char *dp = dst;
    const char *dend = dst + dstsz - 1; // -1 for final nil

    for (const char *p = text; *p && dp < dend; ++p) {
        if (*p == '\r' && *(p+1) == '\n') {
            break;
        } else if (*p == '\r') {
            break;
        } else if (*p == '\n') {
            break;
        } else {
            *dp++ = *p;
        }
    }

    *dp = '\0';
    return dst;
}

char *
compile_argv(const config_t *config, errstack_t *errstack, int argc, char *argv[], const char *src) {
    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);
    opts_t *opts = opts_new();

    if (!opts_parse(opts, argc, argv)) {
        if (errstack) {
            errstack_pushb(errstack, "failed to compile argv. failed to parse options");
        }
        return NULL;
    }

    tkr_parse(tkr, src);
    if (tkr_has_error_stack(tkr)) {
        if (errstack) {
            const errstack_t *es = tkr_getc_error_stack(tkr);
            errstack_extendb_other(errstack, es);
        }
        return NULL;
    }

    ast_clear(ast);
    ast_move_opts(ast, opts);
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_errors(ast)) {
        if (errstack) {
            const errstack_t *es = ast_getc_error_stack(ast);
            errstack_extendb_other(errstack, es);
        }
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_errors(ast)) {
        if (errstack) {
            const errstack_t *es = ast_getc_error_stack(ast);
            errstack_extendb_other(errstack, es);
        }
        return NULL;
    }

    tkr_del(tkr);
    ast_del(ast);

    string_t *buf = str_new();
    str_app(buf, ctx_getc_stdout_buf(ctx));
    str_app(buf, ctx_getc_stderr_buf(ctx));

    ctx_del(ctx);
    gc_del(gc);

    return str_esc_del(buf);
}

void
clear_screen(void) {
#ifdef _CAP_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

static char *
read_path_var_from_resource(const config_t *config, const char *rcpath) {
    char *src = file_readcp_from_path(rcpath);

    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);
    opts_t *opts = opts_new();

    tkr_parse(tkr, src);
    free(src);
    src = NULL;
    if (tkr_has_error_stack(tkr)) {
        err_error("%s", tkr_getc_first_error_message(tkr));
        return NULL;
    }

    ast_clear(ast);
    ast_move_opts(ast, opts);
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_errors(ast)) {
        err_error("%s", ast_getc_first_error_message(ast));
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_errors(ast)) {
        err_error("%s", ast_getc_first_error_message(ast));
        return NULL;
    }

    tkr_del(tkr);
    ast_del(ast);

    object_dict_t *varmap = ctx_get_varmap_at_global(ctx);
    const object_dict_item_t *item = objdict_getc(varmap, "PATH");
    if (!item) {
        ctx_del(ctx);
        gc_del(gc);
        return NULL;
    }

    ctx_pop_newline_of_stdout_buf(ctx);
    printf("%s", ctx_getc_stdout_buf(ctx));
    fflush(stdout);

    const char *s = uni_getc_mb(item->value->unicode);
    char *path = cstr_edup(s);

    ctx_del(ctx);
    gc_del(gc);

    return path;
}

cstring_array_t *
split_to_array(const char *str, int ch) {
    if (!str) {
        return NULL;
    }

    cstring_array_t *arr = cstrarr_new();
    string_t *s = str_new();

    for (const char *p = str; *p; ++p) {
        if (*p == ch) {
            if (str_len(s)) {
                cstrarr_pushb(arr, str_getc(s));
                str_clear(s);
            }
        } else {
            str_pushb(s, *p);
        }
    }

    if (str_len(s)) {
        cstrarr_pushb(arr, str_getc(s));
    }

    str_del(s);
    return arr;
}

static cstring_array_t *
split_path_var(const char *path) {
    return split_to_array(path, ':');
}

cstring_array_t *
pushf_argv(int argc, char *argv[], const char *front) {
    cstring_array_t *arr = cstrarr_new();

    cstrarr_pushb(arr, front);

    for (int32_t i = 0; i < argc; ++i) {
        cstrarr_pushb(arr, argv[i]);
    }

    return mem_move(arr);
}

char *
escape(char *dst, int32_t dstsz, const char *src, const char *target) {
    if (!dst || !dstsz || !src || !target) {
        return NULL;
    }

    char *dp = dst;
    char *dpend = dst + dstsz - 1;
    const char *p = src;

    for (; dp < dpend && *p; ++dp, ++p) {
        if (strchr(target, *p)) {
            *dp++ = '\\';
            if (dp < dpend) {
                *dp = *p;
            }
        } else {
            *dp = *p;
        }
    }

    *dp = '\0';
    return dst;
}

bool
is_dot_file(const char *path) {
    return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

char *
pop_tail_slash(char *path) {
    int32_t pathlen = strlen(path);
#ifdef _CAP_WINDOWS
    if (pathlen == 3 && path[2] == '\\') {
        return path;
    } else {
        return path_pop_tail_slash(path);
    }
#else
    if (pathlen == 1 && path[0] == '/') {
        return path;
    } else {
        return path_pop_tail_slash(path);
    }
#endif
}
