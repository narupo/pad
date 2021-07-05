/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <pad/core/util.h>

void
Pad_FreeArgv(int argc, char *argv[]) {
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            free(argv[i]);
        }
        free(argv);
    }
}

void
Pad_ShowArgv(int argc, char *argv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = [%s]\n", i, argv[i]);
    }
}

int
Pad_RandRange(int min, int max) {
    return min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
}

int
Pad_SafeSystem(const char *cmdline, int option) {
    if (option & PAD_SAFESYSTEM_UNSAFE) {
        return system(cmdline);
    }

#ifdef PAD_WINDOWS
    int flag = 0;
    if (option & PAD_SAFESYSTEM_EDIT) {
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

    if (option & PAD_SAFESYSTEM_EDIT) {
        // case of edit command, to not wait exit of child process
        return 0;
    }

    if (option & PAD_SAFESYSTEM_DETACH) {
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
    PadCL *cl = PadCL_New();
    if (!PadCL_ParseStrOpts(cl, cmdline, 0)) {
        err_error("failed to parse command line \"%s\"", cmdline);
        PadCL_Del(cl);
        return -1;
    }

    int argc = PadCL_Len(cl);
    char **argv = PadCL_EscDel(cl);
    if (!argv) {
        err_error("failed to Pad_Escape and delete of clk");
        return -1;
    }

    int status = 0;

    switch (fork()) {
    default: // parent
        Pad_FreeArgv(argc, argv);
        wait(&status);
        break;
    case 0: // child
        if (execv(argv[0], argv) == -1) {
            err_error("failed to Pad_SafeSystem");
            Pad_FreeArgv(argc, argv);
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
Pad_ArgsByOptind(int argc, char *argv[], int optind) {
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
Pad_TrimFirstLine(char *dst, int32_t dstsz, const char *text) {
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
Pad_CompileArgv(const PadConfig *config, PadErrStack *errstack, int argc, char *argv[], const char *src) {
    PadTkr *tkr = PadTkr_New(PadTkrOpt_New());
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);
    PadOpts *opts = PadOpts_New();

    if (!PadOpts_Parse(opts, argc, argv)) {
        if (errstack) {
            PadErrStack_PushBack(errstack, NULL, 0, NULL, 0, "failed to compile argv. failed to parse options");
        }
        return NULL;
    }

    PadTkr_Parse(tkr, src);
    if (PadTkr_HasErrStack(tkr)) {
        if (errstack) {
            const PadErrStack *es = PadTkr_GetcErrStack(tkr);
            PadErrStack_ExtendBackOther(errstack, es);
        }
        return NULL;
    }

    PadAst_Clear(ast);
    PadAst_MoveOpts(ast, opts);
    opts = NULL;

    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    if (PadAst_HasErrs(ast)) {
        if (errstack) {
            const PadErrStack *es = PadAst_GetcErrStack(ast);
            PadErrStack_ExtendBackOther(errstack, es);
        }
        return NULL;
    }

    PadTrv_Trav(ast, ctx);
    if (PadAst_HasErrs(ast)) {
        if (errstack) {
            const PadErrStack *es = PadAst_GetcErrStack(ast);
            PadErrStack_ExtendBackOther(errstack, es);
        }
        return NULL;
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);

    string_t *buf = str_new();
    str_app(buf, PadCtx_GetcStdoutBuf(ctx));
    str_app(buf, PadCtx_GetcStderrBuf(ctx));

    PadCtx_Del(ctx);
    PadGC_Del(gc);

    return str_esc_del(buf);
}

void
Pad_ClearScreen(void) {
#ifdef PAD_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

static char *
read_path_var_from_resource(const PadConfig *config, const char *rcpath) {
    char *src = file_readcp_from_path(rcpath);

    PadTkr *tkr = PadTkr_New(PadTkrOpt_New());
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);
    PadOpts *opts = PadOpts_New();

    PadTkr_Parse(tkr, src);
    free(src);
    src = NULL;
    if (PadTkr_HasErrStack(tkr)) {
        err_error("%s", PadTkr_GetcFirstErrMsg(tkr));
        return NULL;
    }

    PadAst_Clear(ast);
    PadAst_MoveOpts(ast, opts);
    opts = NULL;

    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    if (PadAst_HasErrs(ast)) {
        err_error("%s", PadAst_GetcFirstErrMsg(ast));
        return NULL;
    }

    PadTrv_Trav(ast, ctx);
    if (PadAst_HasErrs(ast)) {
        err_error("%s", PadAst_GetcFirstErrMsg(ast));
        return NULL;
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);

    PadObjDict *varmap = PadCtx_GetVarmapAtGlobal(ctx);
    const PadObjDictItem *item = PadObjDict_Getc(varmap, "PATH");
    if (!item) {
        PadCtx_Del(ctx);
        PadGC_Del(gc);
        return NULL;
    }

    PadCtx_PopNewlineOfStdoutBuf(ctx);
    printf("%s", PadCtx_GetcStdoutBuf(ctx));
    fflush(stdout);

    const char *s = uni_getc_mb(item->value->unicode);
    char *path = cstr_dup(s);
    if (!path) {
        PadCtx_Del(ctx);
        PadGC_Del(gc);
        return NULL;        
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);

    return path;
}

cstring_array_t *
Pad_SplitToArray(const char *str, int ch) {
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
    return Pad_SplitToArray(path, ':');
}

cstring_array_t *
Pad_PushFrontArgv(int argc, char *argv[], const char *front) {
    cstring_array_t *arr = cstrarr_new();

    cstrarr_pushb(arr, front);

    for (int32_t i = 0; i < argc; ++i) {
        cstrarr_pushb(arr, argv[i]);
    }

    return mem_move(arr);
}

char *
Pad_Escape(char *dst, int32_t dstsz, const char *src, const char *target) {
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
Pad_IsDotFile(const char *path) {
    return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

char *
Pad_PopTailSlash(char *path) {
    int32_t pathlen = strlen(path);
#ifdef PAD_WINDOWS
    if (pathlen == 3 && path[2] == '\\') {
        return path;
    } else {
        return path_Pad_PopTailSlash(path);
    }
#else
    if (pathlen == 1 && path[0] == '/') {
        return path;
    } else {
        return path_Pad_PopTailSlash(path);
    }
#endif
}
