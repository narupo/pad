/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include <core/util.h>

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

bool
isoutofhome(const char *varhome, const char *pth) {
    char hm[FILE_NPATH];
    if (!file_readline(hm, sizeof hm, varhome)) {
        err_error("invalid environment variable of varhome");
        return true;
    }

    char home[FILE_NPATH];
    char path[FILE_NPATH];

    if (!file_solve(home, sizeof home, hm) ||
        !file_solve(path, sizeof path, pth)) {
        return true;
    }

    if (!file_exists(path)) {
        return true;
    }

    size_t homelen = strlen(home);
    if (strncmp(home, path, homelen)) {
        return true;
    }

    return false;
}

bool
is_out_of_home(const char *homepath, const char *argpath) {
    char home[FILE_NPATH];
    char path[FILE_NPATH];

    if (!file_solve(home, sizeof home, homepath) ||
        !file_solve(path, sizeof path, argpath)) {
        return true;
    }

    if (!file_exists(path)) {
        return true;
    }

    size_t homelen = strlen(home);
    if (strncmp(home, path, homelen)) {
        return true;
    }

    return false;
}

bool
is_out_of_home_no_exists(const char *homepath, const char *argpath) {
    char home[FILE_NPATH];
    char path[FILE_NPATH];

    if (!file_solve(home, sizeof home, homepath) ||
        !file_solve(path, sizeof path, argpath)) {
        return true;
    }

    size_t homelen = strlen(home);
    if (strncmp(home, path, homelen)) {
        return true;
    }

    return false;
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

const char *
get_origin(const config_t *config, const char *cap_path) {
    if (cap_path[0] == '/') {
        return config->home_path;
    } else if (config->scope == CAP_SCOPE_LOCAL) {
        return config->cd_path;
    } else if (config->scope == CAP_SCOPE_GLOBAL) {
        return config->home_path;
    }
    err_die("impossible. invalid state in get origin");
    return NULL;
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
compile_argv(const config_t *config, int argc, char *argv[], const char *src) {
    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);
    opts_t *opts = opts_new();

    if (!opts_parse(opts, argc-1, argv+1)) {
        err_error("failed to compile argv. failed to parse options");
        return NULL;
    }

    tkr_parse(tkr, src);
    if (tkr_has_error(tkr)) {
        err_error("%s", tkr_get_error_detail(tkr));
        return NULL;
    }

    ast_move_opts(ast, opts);
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_error(ast)) {
        err_error("%s", ast_get_error_detail(ast));
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_error(ast)) {
        err_error("%s", ast_get_error_detail(ast));
        return NULL;        
    }

    tkr_del(tkr);
    ast_del(ast);

    char *compiled = cstr_edup(ctx_getc_buf(ctx));
    ctx_del(ctx);
    gc_del(gc);

    return compiled;
}

void
clear_screen(void) {
#ifdef _CAP_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

/**
 * Show snippet code by fname
 *
 * @param[in] *config reference to config
 * @param[in] *fname  snippet file name
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 *
 * @return success to true
 * @return failed to false
 */
static bool
show_snippet(const config_t *config, const char *fname, int argc, char **argv) {
    char path[FILE_NPATH];
    if (!file_solvefmt(path, sizeof path, "%s/%s", config->codes_dir_path, fname)) {
        err_error("failed to solve path for snippet file");
        return false;
    }

    char *content = file_readcp_from_path(path);
    if (!content) {
        err_error("failed to read from snippet \"%s\"", fname);
        return false;
    }

    char *compiled = compile_argv(config, argc, argv, content);
    if (!compiled) {
        err_error("failed to compile snippet");
        free(content);
        return false;
    }

    printf("%s", compiled);
    fflush(stdout);

    free(content);
    free(compiled);

    return true;
}

int
execute_snippet(const config_t *config, const char *name, int argc, char **argv) {
    file_dir_t *dir = file_diropen(config->codes_dir_path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", config->codes_dir_path);
        return 1;
    }

    bool found = false;

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *fname = file_dirnodename(node);
        if (is_dot_file(fname)) {
            continue;
        }

        if (cstr_eq(fname, name)) {
            found = true;
            if (!show_snippet(config, fname, argc, argv)) {
                file_dirclose(dir);
                return 1;
            }
        }
    }

    file_dirclose(dir);
    return found ? 0 : -1;
}

char *
solve_cmdline_arg_path(const config_t *config, char *dst, int32_t dstsz, const char *caps_arg_path) {
    if (caps_arg_path[0] == ':') {
        if (!file_solve(dst, dstsz, caps_arg_path+1)) {
            return NULL;
        }
    } else {
        char tmp[FILE_NPATH*2];
        const char *org = get_origin(config, caps_arg_path);

        const char *path = caps_arg_path;
        if (caps_arg_path[0] == '/') {
            path = caps_arg_path + 1;
        }

        snprintf(tmp, sizeof tmp, "%s/%s", org, path);
        if (!symlink_follow_path(config, dst, dstsz, tmp)) {
            return NULL;
        }
    }

    return dst;
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
