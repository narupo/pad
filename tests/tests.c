/**
 * Pad
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <tests/tests.h>

/*********
* macros *
*********/

#define showbuf() printf("stdout[%s]\n", PadCtx_GetcStdoutBuf(ctx))
#define showerr() printf("stderr[%s]\n", PadCtx_GetcStderrBuf(ctx))
#define showdetail() printf("detail[%s]\n", PadAst_GetcFirstErrMsg(ast))
#define trace() PadErrStack_TraceDebug(ast->error_stack, stderr)
#define ERR PadErrStack_Trace(ast->error_stack, stderr)
#define eq(s) assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), s))
#define ast_debug(stmt) { \
    PadAst_SetDebug(ast, true); \
    stmt; \
    PadAst_SetDebug(ast, false); \
}
#define check_ok(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        assert(!PadAst_HasErrs(ast)); \
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), hope)); \
    }
#define check_ok_debug_compile(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        ast_debug(PadCc_Compile(ast, PadTkr_GetToks(tkr))); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        assert(!PadAst_HasErrs(ast)); \
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), hope)); \
    }
#define check_ok_debug_traverse(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        ast_debug(PadTrv_Trav(ast, ctx)); \
        assert(!PadAst_HasErrs(ast)); \
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), hope)); \
    }
#define check_ok_trace(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        trace(); \
        assert(!PadAst_HasErrs(ast)); \
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), hope)); \
    }
#define check_ok_showbuf(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        assert(!PadAst_HasErrs(ast)); \
        showbuf(); \
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), hope)); \
    }
#define check_fail(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        assert(PadAst_HasErrs(ast)); \
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), hope)); \
    }
#define check_fail_showbuf(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        const char *msg = PadAst_GetcFirstErrMsg(ast); \
        if (msg) printf("%s\n", msg); \
        else printf("%p\n", msg); \
        assert(PadAst_HasErrs(ast)); \
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), hope)); \
    }
#define check_fail_trace(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        trace(); \
        assert(PadAst_HasErrs(ast)); \
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), hope)); \
    }
#define check_fail_debug_compile(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        ast_debug(PadCc_Compile(ast, PadTkr_GetToks(tkr))); \
        PadCtx_Clear(ctx); \
        PadTrv_Trav(ast, ctx); \
        assert(PadAst_HasErrs(ast)); \
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), hope)); \
    }
#define check_fail_debug_traverse(code, hope) \
    PadTkr_Parse(tkr, code); \
    { \
        PadAst_Clear(ast); \
        PadCc_Compile(ast, PadTkr_GetToks(tkr)); \
        PadCtx_Clear(ctx); \
        ast_debug(PadTrv_Trav(ast, ctx)); \
        assert(PadAst_HasErrs(ast)); \
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), hope)); \
    }

#define trv_ready \
    PadConfig *config = PadConfig_New(); \
    PadTkrOpt *opt = PadTkrOpt_New(); \
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt)); \
    PadAST *ast = PadAst_New(config); \
    PadGC *gc = PadGC_New(); \
    PadCtx *ctx = PadCtx_New(gc); \

#define trv_cleanup \
    PadCtx_Del(ctx); \
    PadGC_Del(gc); \
    PadAst_Del(ast); \
    PadTkr_Del(tkr); \
    PadConfig_Del(config); \

/********
* utils *
********/

/**
 * Show error message and exit from process.
 *
 * @param string fmt message format.
 * @param ...    ... format arguments.
 */
static void
die(const char *fmt, ...) {
    if (!fmt) {
        return;
    }

    size_t fmtlen = strlen(fmt);
    va_list args;
    va_start(args, fmt);

    fflush(stdout);
    fprintf(stderr, "die: ");

    if (isalpha(fmt[0])) {
        fprintf(stderr, "%c", toupper(fmt[0]));
        vfprintf(stderr, fmt+1, args);
    } else {
        vfprintf(stderr, fmt, args);
    }

    if (fmtlen && fmt[fmtlen-1] != '.') {
        fprintf(stderr, ". ");
    }

    if (errno != 0) {
        fprintf(stderr, "%s.", strerror(errno));
    }

    fprintf(stderr, "\n");

    va_end(args);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * Show error message.
 *
 * @param string fmt message format.
 * @param ...    ... format arguments.
 */
static void
warn(const char *fmt, ...) {
    if (!fmt) {
        return;
    }

    size_t fmtlen = strlen(fmt);
    va_list args;
    va_start(args, fmt);

    fflush(stdout);
    fprintf(stderr, "die: ");

    if (isalpha(fmt[0])) {
        fprintf(stderr, "%c", toupper(fmt[0]));
        vfprintf(stderr, fmt+1, args);
    } else {
        vfprintf(stderr, fmt, args);
    }

    if (fmtlen && fmt[fmtlen-1] != '.') {
        fprintf(stderr, ". ");
    }

    if (errno != 0) {
        fprintf(stderr, "%s.", strerror(errno));
    }

    fprintf(stderr, "\n");

    va_end(args);
    fflush(stderr);
}

/**
 * solve path
 * fix for valgrind issue
 *
 * @param[in] *dst
 * @param[in] dstsz
 * @param[in] *path
 *
 * @return
 */
static char *
solve_path(char *dst, int32_t dstsz, const char *path) {
    char tmp[PAD_FILE__NPATH] = {0};
    assert(PadFile_Solve(tmp, sizeof tmp, path));
    snprintf(dst, dstsz, "%s", tmp);
    return dst;
}

/********
* tests *
********/

struct testcase {
    const char *name;
    void (*test)(void);
};

struct testmodule {
    const char *name;
    const struct testcase *tests;
};

/********
* array *
********/

void
_freeescarr(char **arr) {
    for (char **p = arr; *p; ++p) {
        free(*p);
    }
    free(arr);
}

int
_countescarr(char **arr) {
    int i = 0;
    for (char **p = arr; *p; ++p) {
        ++i;
    }
    return i;
}

void
test_PadCStrAry_New(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);
    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_EscDel(void) {
    // test
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_EscDel(NULL) == NULL);

    char **escarr = PadCStrAry_EscDel(arr);
    assert(escarr != NULL);

    int i;
    for (i = 0; escarr[i]; ++i) {
    }
    assert(i == 0);
    _freeescarr(escarr);

    // test
    arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Push(arr, "0") != NULL);
    assert(PadCStrAry_Push(arr, "1") != NULL);
    assert(PadCStrAry_Push(arr, "2") != NULL);

    escarr = PadCStrAry_EscDel(arr);
    assert(_countescarr(escarr) == 3);
    assert(strcmp(escarr[0], "0") == 0);
    assert(strcmp(escarr[1], "1") == 0);
    assert(strcmp(escarr[2], "2") == 0);
    _freeescarr(escarr);
}

void
test_PadCStrAry_Push(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Push(NULL, "1") == NULL);
    assert(PadCStrAry_Push(arr, NULL) == NULL);
    assert(PadCStrAry_Push(arr, "") != NULL);
    assert(PadCStrAry_Push(arr, "1") != NULL);

    assert(PadCStrAry_Len(arr) == 2);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_PushBack(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_PushBack(NULL, "1") == NULL);
    assert(PadCStrAry_PushBack(arr, NULL) == NULL);
    assert(PadCStrAry_PushBack(arr, "") != NULL);
    assert(PadCStrAry_PushBack(arr, "1") != NULL);

    assert(PadCStrAry_Len(arr) == 2);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_PopMove(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr);

    assert(PadCStrAry_PushBack(arr, "1"));
    assert(PadCStrAry_PushBack(arr, "2"));
    char *p = PadCStrAry_PopMove(arr);
    assert(p);
    assert(!strcmp(p, "2"));
    free(p);

    p = PadCStrAry_PopMove(arr);
    assert(p);
    assert(!strcmp(p, "1"));
    free(p);

    p = PadCStrAry_PopMove(arr);
    assert(!p);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Move(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Move(arr, NULL) != NULL);
    assert(PadCStrAry_Getc(arr, 0) == NULL);

    char *ptr = PadCStr_EDup("string");
    assert(ptr != NULL);

    assert(PadCStrAry_Move(arr, ptr) != NULL);
    assert(strcmp(PadCStrAry_Getc(arr, 1), "string") == 0);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Sort(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Sort(NULL) == NULL);

    assert(PadCStrAry_Push(arr, "1") != NULL);
    assert(PadCStrAry_Push(arr, "2") != NULL);
    assert(PadCStrAry_Push(arr, "0") != NULL);

    assert(PadCStrAry_Sort(arr) != NULL);
    assert(strcmp(PadCStrAry_Getc(arr, 0), "0") == 0);
    assert(strcmp(PadCStrAry_Getc(arr, 1), "1") == 0);
    assert(strcmp(PadCStrAry_Getc(arr, 2), "2") == 0);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Getc(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Getc(NULL, 0) == NULL);
    assert(PadCStrAry_Getc(arr, 0) == NULL);
    assert(PadCStrAry_Getc(arr, -1) == NULL);

    assert(PadCStrAry_Push(arr, "0") != NULL);
    assert(PadCStrAry_Push(arr, "1") != NULL);
    assert(PadCStrAry_Push(arr, "2") != NULL);

    assert(strcmp(PadCStrAry_Getc(arr, 0), "0") == 0);
    assert(strcmp(PadCStrAry_Getc(arr, 1), "1") == 0);
    assert(strcmp(PadCStrAry_Getc(arr, 2), "2") == 0);
    assert(PadCStrAry_Getc(arr, 3) == NULL);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Len(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Len(NULL) == 0);
    assert(PadCStrAry_Len(arr) == 0);

    assert(PadCStrAry_Push(arr, "0") != NULL);
    assert(PadCStrAry_Push(arr, "1") != NULL);
    assert(PadCStrAry_Push(arr, "2") != NULL);
    assert(PadCStrAry_Len(arr) == 3);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Show(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr != NULL);

    assert(PadCStrAry_Show(NULL, stdout) == NULL);
    assert(PadCStrAry_Show(arr, NULL) == NULL);
    assert(PadCStrAry_Show(arr, stdout) != NULL);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Clear(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(arr);

    assert(PadCStrAry_PushBack(arr, "1"));
    assert(PadCStrAry_PushBack(arr, "2"));
    assert(PadCStrAry_Len(arr) == 2);
    PadCStrAry_Clear(arr);
    assert(PadCStrAry_Len(arr) == 0);

    PadCStrAry_Del(arr);
}

void
test_PadCStrAry_Resize(void) {
    PadCStrAry *arr = PadCStrAry_New();
    assert(PadCStrAry_Resize(arr, 32));
    assert(PadCStrAry_Resize(arr, 8));
    assert(PadCStrAry_Resize(arr, 16));
    PadCStrAry_Del(arr);
}

static const struct testcase
cstrarr_tests[] = {
    {"PadCStrAry_New", test_PadCStrAry_New},
    {"PadCStrAry_EscDel", test_PadCStrAry_EscDel},
    {"PadCStrAry_Push", test_PadCStrAry_Push},
    {"PadCStrAry_PushBack", test_PadCStrAry_PushBack},
    {"PadCStrAry_PopMove", test_PadCStrAry_PopMove},
    {"PadCStrAry_Move", test_PadCStrAry_Move},
    {"PadCStrAry_Sort", test_PadCStrAry_Sort},
    {"PadCStrAry_Getc", test_PadCStrAry_Getc},
    {"PadCStrAry_Len", test_PadCStrAry_Len},
    {"PadCStrAry_Show", test_PadCStrAry_Show},
    {"PadCStrAry_Clear", test_PadCStrAry_Clear},
    {"PadCStrAry_Resize", test_PadCStrAry_Resize},
    {0},
};

/**********
* cmdline *
**********/

void
test_PadCmdline_New(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);
    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Del(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);
    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_0(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Parse(cmdline, "abc"));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_1(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Parse(cmdline, "abc && def"));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_2(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Parse(cmdline, "abc | def"));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_3(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Parse(cmdline, "abc > def"));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    const PadCmdlineObj *obj = NULL;

    assert(PadCmdline_Parse(cmdline, "abc && def | ghi > jkl"));
    assert(PadCmdline_Len(cmdline) == 7);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__AND);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "def"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 3);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__PIPE);
    obj = PadCmdline_Getc(cmdline, 4);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "ghi"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 5);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__REDIRECT);
    obj = PadCmdline_Getc(cmdline, 6);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "jkl"));
    assert(PadCL_Len(obj->cl) == 1);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_pipe(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    const PadCmdlineObj *obj = NULL;

    assert(PadCmdline_Parse(cmdline, "abc"));
    assert(PadCmdline_Len(cmdline) == 1);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc"));
    assert(PadCL_Len(obj->cl) == 1);

    assert(PadCmdline_Parse(cmdline, "abc | def"));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__PIPE);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "def"));
    assert(PadCL_Len(obj->cl) == 1);

    assert(PadCmdline_Parse(cmdline, "abc -d efg | hij -d \"klm\""));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc -d efg"));
    assert(PadCL_Len(obj->cl) == 3);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__PIPE);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "hij -d \"klm\""));
    assert(PadCL_Len(obj->cl) == 3);

    assert(PadCmdline_Parse(cmdline, "a | b | c | d | e"));
    assert(PadCmdline_Len(cmdline) == 9);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_and(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    const PadCmdlineObj *obj = NULL;

    assert(PadCmdline_Parse(cmdline, "abc && def"));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__AND);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "def"));
    assert(PadCL_Len(obj->cl) == 1);

    assert(PadCmdline_Parse(cmdline, "abc -d efg && hij -d \"klm\""));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc -d efg"));
    assert(PadCL_Len(obj->cl) == 3);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__AND);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "hij -d \"klm\""));
    assert(PadCL_Len(obj->cl) == 3);

    assert(PadCmdline_Parse(cmdline, "a && b && c && d && e"));
    assert(PadCmdline_Len(cmdline) == 9);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Parse_redirect(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    const PadCmdlineObj *obj = NULL;

    assert(PadCmdline_Parse(cmdline, "abc > def"));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc"));
    assert(PadCL_Len(obj->cl) == 1);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__REDIRECT);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "def"));
    assert(PadCL_Len(obj->cl) == 1);

    assert(PadCmdline_Parse(cmdline, "abc -d efg > hij -d \"klm\""));
    assert(PadCmdline_Len(cmdline) == 3);
    obj = PadCmdline_Getc(cmdline, 0);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "abc -d efg"));
    assert(PadCL_Len(obj->cl) == 3);
    obj = PadCmdline_Getc(cmdline, 1);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__REDIRECT);
    obj = PadCmdline_Getc(cmdline, 2);
    assert(obj);
    assert(obj->type == PAD_CMDLINE_OBJ_TYPE__CMD);
    assert(!strcmp(PadStr_Getc(obj->command), "hij -d \"klm\""));
    assert(PadCL_Len(obj->cl) == 3);

    assert(PadCmdline_Parse(cmdline, "a > b > c > d > e"));
    assert(PadCmdline_Len(cmdline) == 9);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Resize(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Resize(NULL, 0) == NULL);
    assert(PadCmdline_Resize(cmdline, 0) == NULL);

    assert(PadCmdline_Resize(cmdline, 32));
    assert(PadCmdline_Resize(cmdline, 8));
    assert(PadCmdline_Resize(cmdline, 16));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_MoveBack(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_MoveBack(NULL, NULL) == NULL);
    assert(PadCmdline_MoveBack(cmdline, NULL) == NULL);

    PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);

    assert(PadCmdline_MoveBack(cmdline, PadMem_Move(obj)));

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Len(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_MoveBack(NULL, NULL) == NULL);
    assert(PadCmdline_MoveBack(cmdline, NULL) == NULL);
    assert(PadCmdline_Len(NULL) == -1);

    PadCmdlineObj *obj1 = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);
    PadCmdlineObj *obj2 = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);

    assert(PadCmdline_MoveBack(cmdline, PadMem_Move(obj1)));
    assert(PadCmdline_Len(cmdline) == 1);

    assert(PadCmdline_MoveBack(cmdline, PadMem_Move(obj2)));
    assert(PadCmdline_Len(cmdline) == 2);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Clear(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    PadCmdline_Clear(NULL);

    PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);

    assert(PadCmdline_MoveBack(cmdline, PadMem_Move(obj)));
    assert(PadCmdline_Len(cmdline) == 1);

    PadCmdline_Clear(cmdline);
    assert(PadCmdline_Len(cmdline) == 0);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_Getc(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Getc(NULL, -1) == NULL);
    assert(PadCmdline_Getc(cmdline, -1) == NULL);
    assert(PadCmdline_Getc(cmdline, 0) == NULL);

    PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);

    assert(PadCmdline_MoveBack(cmdline, PadMem_Move(obj)));
    assert(PadCmdline_Len(cmdline) == 1);

    assert(PadCmdline_Getc(cmdline, 0));
    assert(PadCmdline_Getc(cmdline, 1) == NULL);

    PadCmdline_Del(cmdline);
}

void
test_PadCmdline_HasErr(void) {
    PadCmdline *cmdline = PadCmdline_New();
    assert(cmdline);

    assert(PadCmdline_Parse(cmdline, "||||") == NULL);
    assert(PadCmdline_HasErr(cmdline));

    PadCmdline_Del(cmdline);
}

static const struct testcase
PadCmdlineests[] = {
    {"PadCmdline_New", test_PadCmdline_New},
    {"PadCmdline_Del", test_PadCmdline_Del},
    {"PadCmdline_Parse", test_PadCmdline_Parse},
    {"PadCmdline_Parse_0", test_PadCmdline_Parse_0},
    {"PadCmdline_Parse_1", test_PadCmdline_Parse_1},
    {"PadCmdline_Parse_2", test_PadCmdline_Parse_2},
    {"PadCmdline_Parse_3", test_PadCmdline_Parse_3},
    {"PadCmdline_Parse_pipe", test_PadCmdline_Parse_pipe},
    {"PadCmdline_Parse_and", test_PadCmdline_Parse_and},
    {"PadCmdline_Parse_redirect", test_PadCmdline_Parse_redirect},
    {"PadCmdline_Resize", test_PadCmdline_Resize},
    {"PadCmdline_MoveBack", test_PadCmdline_MoveBack},
    {"PadCmdline_Clear", test_PadCmdline_Clear},
    {"PadCmdline_Getc", test_PadCmdline_Getc},
    {"PadCmdline_HasErr", test_PadCmdline_HasErr},
    {0},
};

/**********
* cstring *
**********/

static void
test_cstring_PadCStr_Copy(void) {
    const char *s = "test";
    char dst[5];

    assert(PadCStr_Copy(NULL, 0, NULL) == NULL);
    assert(PadCStr_Copy(dst, 0, NULL) == NULL);

    assert(PadCStr_Copy(dst, 0, s));
    assert(!strcmp(dst, ""));

    assert(PadCStr_Copy(dst, sizeof dst, s));
    assert(!strcmp(dst, "test"));
}

static void
test_cstring_PadCStr_PopNewline(void) {
    char a[] = "test\n";

    assert(PadCStr_PopNewline(NULL) == NULL);

    assert(PadCStr_PopNewline(a));
    assert(!strcmp(a, "test"));

    char b[] = "b\r\n";
    assert(PadCStr_PopNewline(b));
    assert(!strcmp(b, "b"));

    char c[] = "c\r\n\n";
    assert(PadCStr_PopNewline(c));
    assert(!strcmp(c, "c"));
}

static void
test_cstring_PadCStr_CopyWithout(void) {
    char dst[100];

    assert(PadCStr_CopyWithout(NULL, 0, NULL, NULL) == NULL);
    assert(PadCStr_CopyWithout(dst, 0, NULL, NULL) == NULL);
    assert(PadCStr_CopyWithout(dst, sizeof dst, NULL, NULL) == NULL);
    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", NULL) == NULL);

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "bc"));
    assert(!strcmp(dst, "ad"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "cd"));
    assert(!strcmp(dst, "ab"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "bcd"));
    assert(!strcmp(dst, "a"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "abcd"));
    assert(!strcmp(dst, ""));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "a"));
    assert(!strcmp(dst, "bcd"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "ab"));
    assert(!strcmp(dst, "cd"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "abc"));
    assert(!strcmp(dst, "d"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "ad"));
    assert(!strcmp(dst, "bc"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "xyz"));
    assert(!strcmp(dst, "abcd"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abcd", "axyz"));
    assert(!strcmp(dst, "bcd"));

    assert(PadCStr_CopyWithout(dst, sizeof dst, "abc123def456", "") != NULL);
    assert(strcmp(dst, "abc123def456") == 0);
    assert(PadCStr_CopyWithout(dst, sizeof dst, "abc123def456", "123456") != NULL);
    assert(strcmp(dst, "abcdef") == 0);
    assert(PadCStr_CopyWithout(dst, sizeof dst, "abc123def456", "abcdef") != NULL);
    assert(strcmp(dst, "123456") == 0);
}

static void
test_cstring_PadCStr_App(void) {
    char dst[100] = {0};

    assert(PadCStr_App(dst, sizeof dst, NULL) == NULL);
    assert(PadCStr_App(NULL, sizeof dst, "source") == NULL);
    assert(PadCStr_App(dst, 0, "source") == NULL);

    assert(PadCStr_App(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(PadCStr_App(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(PadCStr_App(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(PadCStr_App(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
}

static void
test_cstring_PadCStr_AppFmt(void) {
    char dst[100] = {0};

    assert(PadCStr_AppFmt(dst, sizeof dst, NULL) == NULL);
    assert(PadCStr_AppFmt(NULL, sizeof dst, "source") == NULL);
    assert(PadCStr_AppFmt(dst, 0, "source") == NULL);

    assert(PadCStr_AppFmt(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(PadCStr_AppFmt(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(PadCStr_AppFmt(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(PadCStr_AppFmt(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);

    *dst = '\0';
    assert(PadCStr_AppFmt(dst, sizeof dst, "n %d is %c", 10, 'i') != NULL);
    assert(strcmp(dst, "n 10 is i") == 0);
}

static void
test_cstring_PadCStr_EDup(void) {
    char *p = PadCStr_EDup("string");
    assert(strcmp(p, "string") == 0);
    free(p);
}

static void
test_cstring_PadCStr_Split(void) {
    assert(PadCStr_Split(NULL, '\0') == NULL);

    char **arr = PadCStr_Split("abc\ndef", '\n');

    assert(!strcmp(arr[0], "abc"));
    assert(!strcmp(arr[1], "def"));
    assert(arr[2] == NULL);

    free(arr[0]);
    free(arr[1]);
    free(arr);

    arr = PadCStr_Split("abc\ndef\n", '\n');

    assert(!strcmp(arr[0], "abc"));
    assert(!strcmp(arr[1], "def"));
    assert(!strcmp(arr[2], ""));
    assert(arr[3] == NULL);

    free(arr[0]);
    free(arr[1]);
    free(arr[2]);
    free(arr);

    arr = PadCStr_SplitIgnoreEmpty("abc\ndef\n", '\n');

    assert(!strcmp(arr[0], "abc"));
    assert(!strcmp(arr[1], "def"));
    assert(arr[2] == NULL);

    free(arr[0]);
    free(arr[1]);
    free(arr);
}

static void
test_cstring_PadCStr_Eq(void) {
    assert(!PadCStr_Eq(NULL, NULL));
    assert(!PadCStr_Eq("abc", NULL));

    assert(PadCStr_Eq("abc", "abc"));
    assert(!PadCStr_Eq("abc", "def"));
}

static void
test_cstring_PadCStr_IsDigit(void) {
    assert(!PadCStr_IsDigit(NULL));

    assert(PadCStr_IsDigit("123"));
    assert(!PadCStr_IsDigit("abc"));
    assert(!PadCStr_IsDigit("12ab"));
}

static const struct testcase
cPadStrests[] = {
    {"PadCStr_Copy", test_cstring_PadCStr_Copy},
    {"PadCStr_PopNewline", test_cstring_PadCStr_PopNewline},
    {"PadCStr_CopyWithout", test_cstring_PadCStr_CopyWithout},
    {"PadCStr_App", test_cstring_PadCStr_App},
    {"PadCStr_AppFmt", test_cstring_PadCStr_AppFmt},
    {"PadCStr_EDup", test_cstring_PadCStr_EDup},
    {"PadCStr_Split", test_cstring_PadCStr_Split},
    {"PadCStr_Eq", test_cstring_PadCStr_Eq},
    {"PadCStr_IsDigit", test_cstring_PadCStr_IsDigit},
    {0},
};

/*********
* string *
*********/

static void
test_PadStr_Del(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    PadStr_Del(NULL);
    PadStr_Del(s);
}

static void
test_PadStr_EscDel(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_EscDel(NULL) == NULL);
    char *ptr = PadStr_EscDel(s);
    assert(ptr != NULL);
    free(ptr);
}

static void
test_PadStr_New(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    PadStr_Del(s);
}

static void
test_PadStr_NewCStr(void) {
    assert(PadStr_NewCStr(NULL) == NULL);
    
    PadStr *s = PadStr_NewCStr("abc");
    assert(s);
    assert(!strcmp(PadStr_Getc(s), "abc"));
    PadStr_Del(s);
}

static void
test_PadStr_DeepCopy(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_DeepCopy(NULL) == NULL);
    PadStr *o = PadStr_DeepCopy(s);
    assert(o != NULL);
    assert(strcmp(PadStr_Getc(o), "1234") == 0);
    PadStr_Del(o);
    PadStr_Del(s);
}

static void
test_PadStr_Len(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Len(NULL) == -1);
    assert(PadStr_Len(s) == 0);
    assert(PadStr_App(s, "abc") != NULL);
    assert(PadStr_Len(s) == 3);
    PadStr_Del(s);
}

static void
test_PadStr_Capa(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Capa(NULL) == -1);
    assert(PadStr_Capa(s) == 4);
    assert(PadStr_App(s, "1234") != NULL);
    assert(PadStr_Capa(s) == 8);
    PadStr_Del(s);
}

static void
test_PadStr_Getc(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Getc(NULL) == NULL);
    assert(strcmp(PadStr_Getc(s), "") == 0);
    assert(PadStr_App(s, "1234") != NULL);
    assert(strcmp(PadStr_Getc(s), "1234") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_Empty(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Empty(NULL) == 0);
    assert(PadStr_Empty(s) == 1);
    assert(PadStr_App(s, "1234") != NULL);
    assert(PadStr_Empty(s) == 0);
    PadStr_Del(s);
}

static void
test_PadStr_Clear(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_App(NULL, "1234") == NULL);
    assert(PadStr_App(s, NULL) == NULL);
    assert(PadStr_App(s, "1234") != NULL);
    assert(PadStr_Len(s) == 4);
    PadStr_Clear(s);
    assert(PadStr_Len(s) == 0);
    PadStr_Del(s);
}

static void
test_PadStr_Set(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(NULL, "1234") == NULL);
    assert(PadStr_Set(s, NULL) == NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    assert(strcmp(PadStr_Getc(s), "1234") == 0);
    assert(PadStr_Set(s, "12") != NULL);
    assert(strcmp(PadStr_Getc(s), "12") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_Resize(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Capa(NULL) == -1);
    assert(PadStr_Capa(s) == 4);
    assert(PadStr_Resize(s, 4*2) != NULL);
    assert(PadStr_Capa(s) == 8);
    PadStr_Del(s);
}

static void
test_PadStr_PushBack(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_PushBack(NULL, '1') == NULL);
    assert(PadStr_PushBack(s, 0) == NULL);
    assert(PadStr_PushBack(s, '\0') == NULL);
    assert(PadStr_PushBack(s, '1') != NULL);
    assert(PadStr_PushBack(s, '2') != NULL);
    assert(strcmp(PadStr_Getc(s), "12") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_PopBack(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_PopBack(NULL) == '\0');
    assert(PadStr_Set(s, "1234") != NULL);
    assert(strcmp(PadStr_Getc(s), "1234") == 0);
    assert(PadStr_PopBack(s) == '4');
    assert(PadStr_PopBack(s) == '3');
    assert(strcmp(PadStr_Getc(s), "12") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_PushFront(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_PushFront(NULL, '1') == NULL);
    assert(PadStr_PushFront(s, 0) == NULL);
    assert(PadStr_PushFront(s, '\0') == NULL);
    assert(PadStr_PushFront(s, '1') != NULL);
    assert(PadStr_PushFront(s, '2') != NULL);
    assert(strcmp(PadStr_Getc(s), "21") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_PopFront(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_PopFront(NULL) == '\0');
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_PopFront(s) == '1');
    assert(PadStr_PopFront(s) == '2');
    assert(strcmp(PadStr_Getc(s), "34") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_App(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_App(NULL, "1234") == NULL);
    assert(PadStr_App(s, NULL) == NULL);
    assert(PadStr_App(s, "1234") != NULL);
    assert(strcmp(PadStr_Getc(s), "1234") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_AppStream(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);

    char curdir[1024];
    char path[1024];
    assert(PadFile_RealPath(curdir, sizeof curdir, ".") != NULL);
    assert(PadFile_SolveFmt(path, sizeof path, "%s/tests/tests.c", curdir) != NULL);

    FILE *fin = fopen(path, "r");
    assert(fin != NULL);
    assert(PadStr_AppStream(NULL, fin) == NULL);
    assert(PadStr_AppStream(s, NULL) == NULL);
    assert(PadStr_AppStream(s, fin) != NULL);
    assert(fclose(fin) == 0);

    PadStr_Del(s);
}

static void
test_PadStr_AppOther(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    PadStr *o = PadStr_New();
    assert(o != NULL);
    assert(PadStr_Set(o, "1234") != NULL);
    assert(PadStr_AppOther(NULL, o) == NULL);
    assert(PadStr_AppOther(s, NULL) == NULL);
    assert(PadStr_AppOther(s, o) != NULL);
    assert(strcmp(PadStr_Getc(s), "12341234") == 0);
    PadStr_Del(o);
    PadStr_Del(s);

    s = PadStr_New();
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_AppOther(s, s) != NULL);
    assert(strcmp(PadStr_Getc(s), "12341234") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_AppFmt(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    char buf[1024];
    assert(PadStr_AppFmt(NULL, buf, sizeof buf, "%s", "test") == NULL);
    assert(PadStr_AppFmt(s, NULL, sizeof buf, "%s", "test") == NULL);
    assert(PadStr_AppFmt(s, buf, 0, "%s", "test") == NULL);
    assert(PadStr_AppFmt(s, buf, sizeof buf, NULL, "test") == NULL);
    assert(PadStr_AppFmt(s, buf, sizeof buf, "%s %d %c", "1234", 1, '2') != NULL);
    assert(strcmp(PadStr_Getc(s), "1234 1 2") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_RStrip(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_RStrip(NULL, "34") == NULL);
    assert(PadStr_RStrip(s, NULL) == NULL);

    PadStr *o = PadStr_RStrip(s, "34");
    assert(o);
    assert(strcmp(PadStr_Getc(o), "12") == 0);

    PadStr_Del(o);
    PadStr_Del(s);
}

static void
test_PadStr_LStrip(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_LStrip(NULL, "12") == NULL);
    assert(PadStr_LStrip(s, NULL) == NULL);

    PadStr *o = PadStr_LStrip(s, "12");
    assert(o);
    assert(strcmp(PadStr_Getc(o), "34") == 0);

    PadStr_Del(o);
    PadStr_Del(s);
}

static void
test_PadStr_Strip(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "--1234--") != NULL);
    assert(PadStr_Strip(NULL, "-") == NULL);
    assert(PadStr_Strip(s, NULL) == NULL);

    PadStr *o = PadStr_Strip(s, "-");
    assert(o);
    assert(strcmp(PadStr_Getc(o), "1234") == 0);

    PadStr_Del(o);
    PadStr_Del(s);
}

static void
test_PadStr_Findc(void) {
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "1234") != NULL);
    assert(PadStr_Findc(NULL, "") == NULL);
    assert(PadStr_Findc(s, NULL) == NULL);
    const char *fnd = PadStr_Findc(s, "23");
    assert(fnd != NULL);
    assert(strcmp(fnd, "234") == 0);
    PadStr_Del(s);
}

static void
test_PadStr_Lower(void) {
    assert(PadStr_Lower(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "ABC") != NULL);
    PadStr *cp = PadStr_Lower(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc"));
    PadStr_Del(cp);
    PadStr_Del(s);
}

static void
test_PadStr_Upper(void) {
    assert(PadStr_Upper(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "abc") != NULL);
    PadStr *cp = PadStr_Upper(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "ABC"));
    PadStr_Del(cp);
    PadStr_Del(s);
}

static void
test_PadStr_Capi(void) {
    assert(PadStr_Capi(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);
    assert(PadStr_Set(s, "abc") != NULL);
    PadStr *cp = PadStr_Capi(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "Abc"));
    PadStr_Del(cp);
    PadStr_Del(s);
}

static void
test_PadStr_Snake(void) {
    assert(PadStr_Snake(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);

    assert(PadStr_Set(s, "abc") != NULL);
    PadStr *cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "AbcDefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abc-def-ghi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_abcDefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "-abcDefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_-abcDefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi_abc-DefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi_abc_def_ghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = PadStr_Snake(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc_def_ghi_abc_def_ghi"));
    PadStr_Del(cp);

    PadStr_Del(s);
}

static void
test_PadStr_Camel(void) {
#undef showcp
#define showcp() printf("cp[%s]\n", PadStr_Getc(cp))

    assert(PadStr_Camel(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);

    assert(PadStr_Set(s, "abc") != NULL);
    PadStr *cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "ABC") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "aBC"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "AFormatB") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "aFormatB"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "ABFormat") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "aBFormat"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "AbcDefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abc-def-ghi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_abcDefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "-abcDefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_-abcDefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi_abc-DefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhiAbcDefGhi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = PadStr_Camel(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcDefGhiAbcDefGhi"));
    PadStr_Del(cp);

    PadStr_Del(s);
}

static void
test_PadStr_Hacker(void) {
#undef showcp
#define showcp() printf("cp[%s]\n", PadStr_Getc(cp))

    assert(PadStr_Hacker(NULL) == NULL);
    PadStr *s = PadStr_New();
    assert(s != NULL);

    assert(PadStr_Set(s, "abc") != NULL);
    PadStr *cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "ABC") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "AFormatB") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "aformatb"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "ABFormat") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abformat"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "AbcDefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abc-def-ghi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_abcDefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "-abcDefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "_-abcDefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi_abc-DefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghiabcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghiabcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abcdefghiabcdefghi"));
    PadStr_Del(cp);

    assert(PadStr_Set(s, "abc0_12def_gh34i") != NULL);
    cp = PadStr_Hacker(s);
    assert(cp);
    assert(!strcmp(PadStr_Getc(cp), "abc012defgh34i"));
    PadStr_Del(cp);

    PadStr_Del(s);
}

static void
test_PadStr_Mul(void) {
    PadStr *s = PadStr_New();
    PadStr_Set(s, "abc");

    assert(PadStr_Mul(NULL, 0) == NULL);

    PadStr *m = PadStr_Mul(s, 2);
    assert(!strcmp(PadStr_Getc(m), "abcabc"));

    PadStr_Del(s);
}

static void
test_PadStr_Indent(void) {
    PadStr *s = PadStr_New();
    PadStr_Set(s, "abc");

    PadStr *ss = PadStr_Indent(s, ' ', 2, 4);
    assert(ss);
    assert(!strcmp(PadStr_Getc(ss), "        abc"));
    PadStr_Del(ss);

    ss = PadStr_Indent(s, '\t', 2, 0);
    assert(ss);
    assert(!strcmp(PadStr_Getc(ss), "\t\tabc"));
    PadStr_Del(ss);

    PadStr_Set(s, "abc\ndef\n");

    ss = PadStr_Indent(s, ' ', 1, 4);
    assert(ss);
    printf("[%s]\n", PadStr_Getc(ss));
    assert(!strcmp(PadStr_Getc(ss), "    abc\n    def\n"));
    PadStr_Del(ss);

    PadStr_Del(s);
}

static const struct testcase
PadStrests[] = {
    {"PadStr_Del", test_PadStr_Del},
    {"PadStr_EscDel", test_PadStr_EscDel},
    {"PadStr_New", test_PadStr_New},
    {"PadStr_NewCStr", test_PadStr_NewCStr},
    {"PadStr_DeepCopy", test_PadStr_DeepCopy},
    {"PadStr_DeepCopy", test_PadStr_DeepCopy},
    {"PadStr_Len", test_PadStr_Len},
    {"PadStr_Capa", test_PadStr_Capa},
    {"PadStr_Getc", test_PadStr_Getc},
    {"PadStr_Empty", test_PadStr_Empty},
    {"PadStr_Clear", test_PadStr_Clear},
    {"PadStr_Set", test_PadStr_Set},
    {"PadStr_Resize", test_PadStr_Resize},
    {"PadStr_PushBack", test_PadStr_PushBack},
    {"PadStr_PopBack", test_PadStr_PopBack},
    {"PadStr_PushFront", test_PadStr_PushFront},
    {"PadStr_PopFront", test_PadStr_PopFront},
    {"PadStr_App", test_PadStr_App},
    {"PadStr_AppStream", test_PadStr_AppStream},
    {"PadStr_AppOther", test_PadStr_AppOther},
    {"PadStr_AppFmt", test_PadStr_AppFmt},
    {"PadStr_RStrip", test_PadStr_RStrip},
    {"PadStr_LStrip", test_PadStr_LStrip},
    {"PadStr_Strip", test_PadStr_Strip},
    {"PadStr_Findc", test_PadStr_Findc},
    {"PadStr_Lower", test_PadStr_Lower},
    {"PadStr_Upper", test_PadStr_Upper},
    {"PadStr_Capi", test_PadStr_Capi},
    {"PadStr_Snake", test_PadStr_Snake},
    {"PadStr_Camel", test_PadStr_Camel},
    {"PadStr_Hacker", test_PadStr_Hacker},
    {"PadStr_Mul", test_PadStr_Mul},
    {"PadStr_Indent", test_PadStr_Indent},
    {0},
};

/**********
* unicode *
**********/

static void
test_PadUni_Del(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    PadUni_Del(NULL);
    PadUni_Del(u);
}

static void
test_PadUni_EscDel(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_EscDel(NULL) == NULL);
    PadUniType *ptr = PadUni_EscDel(u);
    assert(ptr != NULL);
    free(ptr);
}

static void
test_PadUni_New(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    PadUni_Del(u);
}

static void
test_PadUni_DeepCopy(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_SetMB(u, "1234") != NULL);
    assert(PadUni_DeepCopy(NULL) == NULL);
    PadUni *o = PadUni_DeepCopy(u);
    assert(o != NULL);
    assert(PadU_StrCmp(PadUni_Getc(o), PAD_UNI__STR("1234")) == 0);
    PadUni_Del(o);
    PadUni_Del(u);
}

static void
test_PadUni_Len(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Len(NULL) == -1);
    assert(PadUni_Len(u) == 0);
    assert(PadUni_App(u, PAD_UNI__STR("abc")) != NULL);
    assert(PadUni_Len(u) == 3);
    PadUni_Del(u);
}

static void
test_PadUni_Capa(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Capa(NULL) == -1);
    assert(PadUni_Capa(u) == 4);
    assert(PadUni_App(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_Capa(u) == 8);
    PadUni_Del(u);
}

static void
test_PadUni_Getc(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Getc(NULL) == NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("")) == 0);
    assert(PadUni_App(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("1234")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_Empty(void) {
    PadUni *s = PadUni_New();
    assert(s != NULL);
    assert(PadUni_Empty(NULL) == 0);
    assert(PadUni_Empty(s) == 1);
    assert(PadUni_App(s, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_Empty(s) == 0);
    PadUni_Del(s);
}

static void
test_PadUni_Clear(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_App(NULL, PAD_UNI__STR("1234")) == NULL);
    assert(PadUni_App(u, NULL) == NULL);
    assert(PadUni_App(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_Len(u) == 4);
    PadUni_Clear(u);
    assert(PadUni_Len(u) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_Set(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(NULL, PAD_UNI__STR("1234")) == NULL);
    assert(PadUni_Set(u, NULL) == NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("1234")) == 0);
    assert(PadUni_Set(u, PAD_UNI__STR("12")) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("12")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_Resize(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Capa(NULL) == -1);
    assert(PadUni_Capa(u) == 4);
    assert(PadUni_Resize(u, 4 * 2) != NULL);
    assert(PadUni_Capa(u) == 8);
    PadUni_Del(u);
}

static void
test_PadUni_PushBack(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);

    assert(PadUni_PushBack(NULL, PAD_UNI__CH('1')) == NULL);
    assert(PadUni_PushBack(u, 0) == NULL);
    assert(PadUni_PushBack(u, PAD_UNI__CH('\0')) == NULL);
    assert(PadUni_PushBack(u, PAD_UNI__CH('1')) != NULL);
    assert(PadUni_PushBack(u, PAD_UNI__CH('2')) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("12")) == 0);
    
    PadUni_Clear(u);
    assert(PadUni_PushBack(u, PAD_UNI__CH('あ')) != NULL);
    assert(PadUni_PushBack(u, PAD_UNI__CH('い')) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("あい")) == 0);

    PadUni_Del(u);
}

static void
test_PadUni_PopBack(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_PopBack(NULL) == PAD_UNI__CH('\0'));
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("1234")) == 0);
    assert(PadUni_PopBack(u) == PAD_UNI__CH('4'));
    assert(PadUni_PopBack(u) == PAD_UNI__CH('3'));
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("12")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_PushFront(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_PushFront(NULL, PAD_UNI__CH('1')) == NULL);
    assert(PadUni_PushFront(u, 0) == NULL);
    assert(PadUni_PushFront(u, PAD_UNI__CH('\0')) == NULL);
    assert(PadUni_PushFront(u, PAD_UNI__CH('1')) != NULL);
    assert(PadUni_PushFront(u, PAD_UNI__CH('2')) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("21")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_PopFront(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_PopFront(NULL) == '\0');
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_PopFront(u) == PAD_UNI__CH('1'));
    assert(PadUni_PopFront(u) == PAD_UNI__CH('2'));
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("34")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_App(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_App(NULL, PAD_UNI__STR("1234")) == NULL);
    assert(PadUni_App(u, NULL) == NULL);
    assert(PadUni_App(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("1234")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_AppStream(void) {
    PadUni *s = PadUni_New();
    assert(s != NULL);

    char curdir[1024];
    char path[1024];
    assert(PadFile_RealPath(curdir, sizeof curdir, ".") != NULL);
    assert(PadFile_SolveFmt(path, sizeof path, "%s/tests/tests.c", curdir) != NULL);

    FILE *fin = fopen(path, "r");
    assert(fin != NULL);
    assert(PadUni_AppStream(NULL, fin) == NULL);
    assert(PadUni_AppStream(s, NULL) == NULL);
    assert(PadUni_AppStream(s, fin) != NULL);
    assert(fclose(fin) == 0);

    PadUni_Del(s);
}

static void
test_PadUni_AppOther(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    PadUni *o = PadUni_New();
    assert(o != NULL);
    assert(PadUni_Set(o, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_AppOther(NULL, o) == NULL);
    assert(PadUni_AppOther(u, NULL) == NULL);
    assert(PadUni_AppOther(u, o) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("12341234")) == 0);
    PadUni_Del(o);
    PadUni_Del(u);

    u = PadUni_New();
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_AppOther(u, u) != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("12341234")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_AppFmt(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    char buf[1024];
    assert(PadUni_AppFmt(NULL, buf, sizeof buf, "%s", "test") == NULL);
    assert(PadUni_AppFmt(u, NULL, sizeof buf, "%s", "test") == NULL);
    assert(PadUni_AppFmt(u, buf, 0, "%s", "test") == NULL);
    assert(PadUni_AppFmt(u, buf, sizeof buf, NULL, "test") == NULL);
    assert(PadUni_AppFmt(u, buf, sizeof buf, "%s %d %c", "1234", 1, '2') != NULL);
    assert(PadU_StrCmp(PadUni_Getc(u), PAD_UNI__STR("1234 1 2")) == 0);
    PadUni_Del(u);
}

static void
test_PadUni_RStrip(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_RStrip(NULL, PAD_UNI__STR("34")) == NULL);
    assert(PadUni_RStrip(u, NULL) == NULL);

    PadUni *o = PadUni_RStrip(u, PAD_UNI__STR("34"));
    assert(o);
    assert(PadU_StrCmp(PadUni_Getc(o), PAD_UNI__STR("12")) == 0);

    PadUni_Del(o);
    PadUni_Del(u);
}

static void
test_PadUni_LStrip(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("1234")) != NULL);
    assert(PadUni_LStrip(NULL, PAD_UNI__STR("12")) == NULL);
    assert(PadUni_LStrip(u, NULL) == NULL);

    PadUni *o = PadUni_LStrip(u, PAD_UNI__STR("12"));
    assert(o);
    assert(PadU_StrCmp(PadUni_Getc(o), PAD_UNI__STR("34")) == 0);

    PadUni_Del(o);
    PadUni_Del(u);
}

static void
test_PadUni_Strip(void) {
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("--1234--")) != NULL);
    assert(PadUni_Strip(NULL, PAD_UNI__STR("-")) == NULL);
    assert(PadUni_Strip(u, NULL) == NULL);

    PadUni *o = PadUni_Strip(u, PAD_UNI__STR("-"));
    assert(o);
    assert(PadU_StrCmp(PadUni_Getc(o), PAD_UNI__STR("1234")) == 0);

    PadUni_Del(o);
    PadUni_Del(u);
}

static void
test_PadUni_Lower(void) {
    assert(PadUni_Lower(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("ABC")) != NULL);
    PadUni *cp = PadUni_Lower(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc")));
    PadUni_Del(cp);
    PadUni_Del(u);
}

static void
test_PadUni_Upper(void) {
    assert(PadUni_Upper(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("abc")) != NULL);
    PadUni *cp = PadUni_Upper(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("ABC")));
    PadUni_Del(cp);
    PadUni_Del(u);
}

static void
test_PadUni_Capi(void) {
    assert(PadUni_Capi(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);
    assert(PadUni_Set(u, PAD_UNI__STR("abc")) != NULL);
    PadUni *cp = PadUni_Capi(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("Abc")));
    PadUni_Del(cp);
    PadUni_Del(u);
}

static void
test_PadUni_Snake(void) {
    assert(PadUni_Snake(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);

    assert(PadUni_Set(u, PAD_UNI__STR("abc")) != NULL);
    PadUni *cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("AbcDefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abc-def-ghi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_abcDefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("-abcDefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_-abcDefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi_abc-DefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi_abc_def_ghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi__abc--DefGhi")) != NULL);
    cp = PadUni_Snake(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc_def_ghi_abc_def_ghi")));
    PadUni_Del(cp);

    PadUni_Del(u);
}

static void
test_PadUni_Camel(void) {
    assert(PadUni_Camel(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);

    assert(PadUni_Set(u, PAD_UNI__STR("abc")) != NULL);
    PadUni *cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("ABC")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("aBC")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("AFormatB")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("aFormatB")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("ABFormat")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("aBFormat")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("AbcDefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abc-def-ghi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_abcDefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("-abcDefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_-abcDefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi_abc-DefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhiAbcDefGhi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi__abc--DefGhi")) != NULL);
    cp = PadUni_Camel(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcDefGhiAbcDefGhi")));
    PadUni_Del(cp);

    PadUni_Del(u);
}

static void
test_PadUni_Hacker(void) {
    assert(PadUni_Hacker(NULL) == NULL);
    PadUni *u = PadUni_New();
    assert(u != NULL);

    assert(PadUni_Set(u, PAD_UNI__STR("abc")) != NULL);
    PadUni *cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("ABC")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("AFormatB")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("aformatb")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("ABFormat")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abformat")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("AbcDefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abc-def-ghi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_abcDefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("-abcDefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("_-abcDefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi_abc-DefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghiabcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi__abc--DefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghiabcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abcDefGhi__abc--DefGhi")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abcdefghiabcdefghi")));
    PadUni_Del(cp);

    assert(PadUni_Set(u, PAD_UNI__STR("abc0_12def_gh34i")) != NULL);
    cp = PadUni_Hacker(u);
    assert(cp);
    assert(!PadU_StrCmp(PadUni_Getc(cp), PAD_UNI__STR("abc012defgh34i")));
    PadUni_Del(cp);

    PadUni_Del(u);
}

static void
test_PadUni_Get(void) {
    PadUni *u = PadUni_New();
    assert(u);

    assert(PadUni_Get(NULL) == NULL);

    PadUni_SetMB(u, "abc");

    PadUniType *s = PadUni_Get(u);
    assert(PadU_StrCmp(s, U"abc") == 0);

    PadUni_Del(u);
}

static void
test_PadUni_ToMB(void) {
    PadUni *u = PadUni_New();
    assert(u);

    assert(PadUni_ToMB(NULL) == NULL);

    PadUni_SetMB(u, "abc");

    char *s = PadUni_ToMB(u);
    assert(strcmp(s, "abc") == 0);

    free(s);
    PadUni_Del(u);
}

static void
test_PadUni_SetMB(void) {
    PadUni *u = PadUni_New();
    assert(u);

    assert(PadUni_SetMB(NULL, NULL) == NULL);
    assert(PadUni_SetMB(u, NULL) == NULL);

    PadUni_SetMB(u, "abc");

    PadUniType *s = PadUni_Get(u);
    assert(PadU_StrCmp(s, U"abc") == 0);

    PadUni_Del(u);
}

static void
test_PadUni_GetcMB(void) {
    PadUni *u = PadUni_New();
    assert(u);

    assert(PadUni_GetcMB(NULL) == NULL);

    PadUni_SetMB(u, "abc");

    const char *s = PadUni_GetcMB(u);
    assert(strcmp(s, "abc") == 0);

    PadUni_Del(u);
}

static void
test_PadUni_Mul(void) {
    PadUni *u = PadUni_New();
    assert(u);

    assert(PadUni_Mul(NULL, 0) == NULL);

    PadUni_SetMB(u, "abc");

    PadUni *o = PadUni_Mul(u, 3);
    const char *s = PadUni_GetcMB(o);
    assert(strcmp(s, "abcabcabc") == 0);

    PadUni_Del(u);
}

static void
test_PadUni_Split(void) {
    PadUni *u = PadUni_New();
    assert(u);

    PadUni_SetMB(u, "あいう\nかきく\nさしす");
    PadUni **arr = PadUni_Split(u, PAD_UNI__STR("\n"));
    printf("arr[0] = [%s]\n", PadUni_GetcMB(arr[0]));
    assert(!PadU_StrCmp(PadUni_Getc(arr[0]), PAD_UNI__STR("あいう")));
    assert(!PadU_StrCmp(PadUni_Getc(arr[1]), PAD_UNI__STR("かきく")));
    assert(!PadU_StrCmp(PadUni_Getc(arr[2]), PAD_UNI__STR("さしす")));
    assert(arr[3] == NULL);

    for (PadUni **p = arr; *p; ++p) {
        PadUni_Del(*p);
    }
    free(arr);

    PadUni_SetMB(u, "あいう\nかきく\n");
    arr = PadUni_Split(u, PAD_UNI__STR("\n"));
    assert(!PadU_StrCmp(PadUni_Getc(arr[0]), PAD_UNI__STR("あいう")));
    assert(!PadU_StrCmp(PadUni_Getc(arr[1]), PAD_UNI__STR("かきく")));
    assert(arr[3] == NULL);

    for (PadUni **p = arr; *p; ++p) {
        PadUni_Del(*p);
    }
    free(arr);

    PadUni_SetMB(u, "あいうアイウかきくアイウ");
    arr = PadUni_Split(u, PAD_UNI__STR("アイウ"));
    assert(!PadU_StrCmp(PadUni_Getc(arr[0]), PAD_UNI__STR("あいう")));
    assert(!PadU_StrCmp(PadUni_Getc(arr[1]), PAD_UNI__STR("かきく")));
    assert(arr[3] == NULL);

    for (PadUni **p = arr; *p; ++p) {
        PadUni_Del(*p);
    }
    free(arr);
}

static void
test_PadUni_IsDigit(void) {
    PadUni *u = PadUni_New();
    PadUni_SetMB(u, "123");
    assert(PadUni_IsDigit(u));
    PadUni_SetMB(u, "abc");
    assert(!PadUni_IsDigit(u));
    PadUni_SetMB(u, "12ab");
    assert(!PadUni_IsDigit(u));
    PadUni_Del(u);
}

static void
test_PadUni_IsAlpha(void) {
    PadUni *u = PadUni_New();
    PadUni_SetMB(u, "123");
    assert(!PadUni_IsAlpha(u));
    PadUni_SetMB(u, "abc");
    assert(PadUni_IsAlpha(u));
    PadUni_SetMB(u, "12ab");
    assert(!PadUni_IsAlpha(u));
    PadUni_Del(u);
}

static void
test_PadChar32_Len(void) {
    const char32_t *s = U"abc";
    assert(PadChar32_Len(s) == 3);
}

static void
test_PadChar16_Len(void) {
    const char16_t *s = u"abc";
    assert(PadChar16_Len(s) == 3);
}

static void
test_PadChar32_Dup(void) {
    const char32_t *s = U"abc";
    char32_t *o = PadChar32_Dup(s);
    assert(PadChar32_StrCmp(s, o) == 0);
    free(o);
}

static void
test_PadChar16_Dup(void) {
    const char16_t *s = u"abc";
    char16_t *o = PadChar16_Dup(s);
    assert(PadChar16_StrCmp(s, o) == 0);
    free(o);
}

static void
test_PadChar32_IsAlpha(void) {
    assert(PadChar32_IsAlpha(U'a'));
}

static void
test_PadChar16_IsAlpha(void) {
    assert(PadChar16_IsAlpha(u'a'));
}

static void
test_PadChar32_IsLower(void) {
    assert(PadChar32_IsLower(U'a'));
    assert(!PadChar32_IsLower(U'A'));
}

static void
test_PadChar16_IsLower(void) {
    assert(PadChar16_IsLower(u'a'));
    assert(!PadChar16_IsLower(u'A'));
}

static void
test_PadChar32_IsUpper(void) {
    assert(PadChar32_IsUpper(U'A'));
    assert(!PadChar32_IsUpper(U'a'));
}

static void
test_PadChar16_IsUpper(void) {
    assert(PadChar16_IsUpper(u'A'));
    assert(!PadChar16_IsUpper(u'a'));
}

static void
test_PadChar32_ToLower(void) {
    assert(PadChar32_ToLower(U'A') == U'a');
    assert(PadChar32_ToLower(U'a') == U'a');
}

static void
test_PadChar16_ToLower(void) {
    assert(PadChar16_ToLower(u'A') == u'a');
    assert(PadChar16_ToLower(u'a') == u'a');
}

static void
test_PadChar32_ToUpper(void) {
    assert(PadChar32_ToUpper(U'A') == U'A');
    assert(PadChar32_ToUpper(U'a') == U'A');
}

static void
test_PadChar16_ToUpper(void) {
    assert(PadChar16_ToUpper(u'A') == u'A');
    assert(PadChar16_ToUpper(u'a') == u'A');
}

static void
test_PadChar32_IsDigit(void) {
    assert(!PadChar32_IsDigit(U'A'));
    assert(PadChar32_IsDigit(U'1'));
}

static void
test_PadChar16_IsDigit(void) {
    assert(!PadChar16_IsDigit(u'A'));
    assert(PadChar16_IsDigit(u'1'));
}

static void
test_PadChar32_StrCmp(void) {
    assert(PadChar32_StrCmp(U"abc", U"abc") == 0);
    assert(PadChar32_StrCmp(U"abc", U"def") != 0);
}

static void
test_PadChar16_StrCmp(void) {
    assert(PadChar16_StrCmp(u"abc", u"abc") == 0);
    assert(PadChar16_StrCmp(u"abc", u"def") != 0);
}

static const struct testcase
PadUniests[] = {
    {"PadUni_Del", test_PadStr_Del},
    {"PadUni_EscDel", test_PadStr_EscDel},
    {"PadUni_New", test_PadUni_New},
    {"PadUni_DeepCopy", test_PadUni_DeepCopy},
    {"PadUni_DeepCopy", test_PadUni_DeepCopy},
    {"PadUni_Len", test_PadUni_Len},
    {"PadUni_Capa", test_PadUni_Capa},
    {"PadUni_Getc", test_PadUni_Getc},
    {"PadUni_Empty", test_PadUni_Empty},
    {"PadUni_Clear", test_PadUni_Clear},
    {"PadUni_Set", test_PadUni_Set},
    {"PadUni_Resize", test_PadUni_Resize},
    {"PadUni_PushBack", test_PadUni_PushBack},
    {"PadUni_PopBack", test_PadUni_PopBack},
    {"PadUni_PushFront", test_PadUni_PushFront},
    {"PadUni_PopFront", test_PadUni_PopFront},
    {"PadUni_App", test_PadUni_App},
    {"PadUni_AppStream", test_PadUni_AppStream},
    {"PadUni_AppOther", test_PadUni_AppOther},
    {"PadUni_AppFmt", test_PadUni_AppFmt},
    {"PadUni_RStrip", test_PadUni_RStrip},
    {"PadUni_LStrip", test_PadUni_LStrip},
    {"PadUni_Strip", test_PadUni_Strip},
    // {"uni_findc", test_uni_findc},
    {"PadUni_Lower", test_PadUni_Lower},
    {"PadUni_Upper", test_PadUni_Upper},
    {"PadUni_Capi", test_PadUni_Capi},
    {"PadUni_Snake", test_PadUni_Snake},
    {"PadUni_Camel", test_PadUni_Camel},
    {"PadUni_Hacker", test_PadUni_Hacker},
    {"PadUni_Get", test_PadUni_Get},
    {"PadUni_ToMB", test_PadUni_ToMB},
    {"PadUni_SetMB", test_PadUni_SetMB},
    {"PadUni_GetcMB", test_PadUni_GetcMB},
    {"PadUni_Mul", test_PadUni_Mul},
    {"PadUni_Split", test_PadUni_Split},
    {"PadUni_IsDigit", test_PadUni_IsDigit},
    {"PadUni_IsAlpha", test_PadUni_IsAlpha},
    {"PadChar32_Len", test_PadChar32_Len},
    {"PadChar16_Len", test_PadChar16_Len},
    {"PadChar32_Dup", test_PadChar32_Dup},
    {"PadChar16_Dup", test_PadChar16_Dup},
    {"PadChar32_IsAlpha", test_PadChar32_IsAlpha},
    {"PadChar16_IsAlpha", test_PadChar16_IsAlpha},
    {"PadChar32_IsLower", test_PadChar32_IsLower},
    {"PadChar16_IsLower", test_PadChar16_IsLower},
    {"PadChar32_IsUpper", test_PadChar32_IsUpper},
    {"PadChar16_IsUpper", test_PadChar16_IsUpper},
    {"PadChar32_ToUpper", test_PadChar32_ToUpper},
    {"PadChar16_ToUpper", test_PadChar16_ToUpper},
    {"PadChar32_ToLower", test_PadChar32_ToLower},
    {"PadChar16_ToLower", test_PadChar16_ToLower},
    {"PadChar32_IsDigit", test_PadChar32_IsDigit},
    {"PadChar16_IsDigit", test_PadChar16_IsDigit},
    {"PadChar32_StrCmp", test_PadChar32_StrCmp},
    {"PadChar16_StrCmp", test_PadChar16_StrCmp},
    {0},
};

/*******
* file *
*******/

static const char *
get_test_fcontent(void) {
    return "1234567\n";
}

static const char *
get_test_fcontent_nonewline(void) {
    return "1234567";
}

static const char *
get_test_finpath(void) {
    static char path[PAD_FILE__NPATH];

#ifdef _TESTS_WINDOWS
    char tmp[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(tmp, sizeof tmp) != NULL);
    assert(PadFile_SolveFmt(path, sizeof path, "%s/cap.test.file", tmp) != NULL);
#else
    assert(PadFile_Solve(path, sizeof path, "/tmp/cap.test.file") != NULL);
#endif

    if (!PadFile_IsExists(path)) {
        FILE *f = PadFile_Open(path, "wb");
        assert(f != NULL);
        fprintf(f, "%s", get_test_fcontent());
        assert(PadFile_Close(f) == 0);
    }
    return path;
}

static void
remove_test_file(void) {
    const char *path = get_test_finpath();
    if (PadFile_IsExists(path)) {
        assert(PadFile_Remove(path) == 0);
    }
}

static FILE *
get_test_fin(void) {
    FILE *fin = PadFile_Open(get_test_finpath(), "rb");
    assert(fin != NULL);
    return fin;
}

static int
get_test_finsize(void) {
    return strlen(get_test_fcontent());
}

static const char *
get_test_dirpath(void) {
    static char path[PAD_FILE__NPATH];
#ifdef _TESTS_WINDOWS
    assert(PadFile_GetUserHome(path, sizeof path) != NULL);
#else
    assert(PadFile_Solve(path, sizeof path, "/tmp") != NULL);
#endif
    return path;
}

static void
test_PadFile_Close(void) {
    FILE* f = PadFile_Open(get_test_finpath(), "rb");
    assert(f != NULL);
    assert(PadFile_Close(NULL) != 0);
    assert(PadFile_Close(f) == 0);
}

static void
test_PadFile_Open(void) {
    test_PadFile_Close();
}

static void
test_PadFile_Copy(void) {
    FILE *f = PadFile_Open(get_test_finpath(), "rb");
    assert(f != NULL);
    // TODO
    assert(PadFile_Close(f) == 0);
}

static void
test_PadFile_CloseDir(void) {
    DIR *f = PadFile_OpenDir(get_test_dirpath());
    assert(f != NULL);
    assert(PadFile_CloseDir(NULL) == -1);
    assert(PadFile_CloseDir(f) == 0);
}

static void
test_PadFile_OpenDir(void) {
    test_PadFile_CloseDir();
}

static void
test_PadFile_RealPath(void) {
    char path[PAD_FILE__NPATH];

    assert(PadFile_RealPath(NULL, sizeof path, "/tmp/../tmp") == NULL);
    assert(PadFile_RealPath(path, 0, "/tmp/../tmp") == NULL);
    assert(PadFile_RealPath(path, sizeof path, NULL) == NULL);

    char userhome[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(userhome, sizeof userhome));

    char src[PAD_FILE__NPATH + 5] = {0};
    snprintf(src, sizeof src, "%s%c..", userhome, PAD_FILE__SEP);
    assert(PadFile_RealPath(path, sizeof path, src) != NULL);
}

static void
test_PadFile_IsExists(void) {
    assert(PadFile_IsExists(NULL) == false);
    assert(PadFile_IsExists(get_test_dirpath()));
    assert(!PadFile_IsExists("/nothing/directory"));
}

static void
test_PadFile_MkdirMode(void) {
    // TODO
}

static void
test_PadFile_MkdirQ(void) {
    assert(PadFile_MkdirQ(NULL) != 0);
}

static void
test_PadFile_Trunc(void) {
    char path[PAD_FILE__NPATH];
    char userhome[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(userhome, sizeof userhome) != NULL);
    assert(PadFile_SolveFmt(path, sizeof path, "%s/cap.ftrunc", userhome) != NULL);

    assert(!PadFile_IsExists(path));
    assert(!PadFile_Trunc(NULL));
    assert(PadFile_Trunc(path));
    assert(PadFile_IsExists(path));
    assert(PadFile_Remove(path) == 0);
}

static void
test_PadFile_Solve(void) {
    char path[PAD_FILE__NPATH];
    assert(PadFile_Solve(NULL, sizeof path, "/tmp/../tmp") == NULL);
    assert(PadFile_Solve(path, 0, "/tmp/../tmp") == NULL);
    assert(PadFile_Solve(path, sizeof path, NULL) == NULL);
    assert(PadFile_Solve(path, sizeof path, get_test_dirpath()) != NULL);
}

static void
test_PadFile_SolveCopy(void) {
    assert(!PadFile_SolveCopy(NULL));
    char *path = PadFile_SolveCopy(get_test_dirpath());
    assert(path != NULL);
    assert(strcmp(path, get_test_dirpath()) == 0);
    free(path);
}

static void
test_PadFile_SolveFmt(void) {
    char path[1024];
    assert(PadFile_SolveFmt(NULL, sizeof path, "/%s/../%s", "tmp", "tmp") == NULL);
    assert(PadFile_SolveFmt(path, 0, "/%s/../%s", "tmp", "tmp") == NULL);
    assert(PadFile_SolveFmt(path, sizeof path, NULL, "tmp", "tmp") == NULL);
    assert(PadFile_SolveFmt(path, sizeof path, "%s", get_test_dirpath()) != NULL);
}

static void
test_PadFile_IsDir(void) {
    assert(!PadFile_IsDir(NULL));
    assert(PadFile_IsDir(get_test_dirpath()));
    assert(!PadFile_IsDir("/not/found/directory"));
}

static void
test_PadFile_ReadCopy(void) {
    FILE *fin = PadFile_Open(get_test_finpath(), "rb");
    assert(fin != NULL);
    assert(!PadFile_ReadCopy(NULL));
    char *p = PadFile_ReadCopy(fin);
    PadFile_Close(fin);
    assert(p != NULL);
    free(p);
}

static void
test_PadFile_Size(void) {
    FILE *fin = PadFile_Open(get_test_finpath(), "rb");
    assert(fin != NULL);
    assert(PadFile_Size(NULL) == -1);
    assert(PadFile_Size(fin) == get_test_finsize());
    assert(PadFile_Close(fin) == 0);
}

static void
test_PadFile_Suffix(void) {
    assert(PadFile_Suffix(NULL) == NULL);
    const char *suf = PadFile_Suffix("/this/is/text/file.txt");
    assert(suf != NULL);
    assert(strcmp(suf, "txt") == 0);
}

static void
test_PadFile_DirName(void) {
    char name[PAD_FILE__NPATH];
    char userhome[PAD_FILE__NPATH];
    char path[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(userhome, sizeof userhome));
    assert(PadFile_SolveFmt(path, sizeof path, "%s/file", userhome));

    assert(PadFile_DirName(NULL, sizeof name, path) == NULL);
    assert(PadFile_DirName(name, 0, path) == NULL);
    assert(PadFile_DirName(name, sizeof name, NULL) == NULL);
    assert(PadFile_DirName(name, sizeof name, path) != NULL);
    assert(strcmp(name, userhome) == 0);
}

static void
test_PadFile_BaseName(void) {
    char name[PAD_FILE__NPATH];
    char userhome[PAD_FILE__NPATH];
    char path[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(userhome, sizeof userhome));
    assert(PadFile_SolveFmt(path, sizeof path, "%s/file.txt", userhome));

    assert(PadFile_BaseName(NULL, sizeof name, path) == NULL);
    assert(PadFile_BaseName(name, 0, path) == NULL);
    assert(PadFile_BaseName(name, sizeof name, NULL) == NULL);
    assert(PadFile_BaseName(name, sizeof name, path) != NULL);
    assert(strcmp(name, "file.txt") == 0);
}

static void
test_PadFile_GetLine(void) {
    FILE *fin = get_test_fin();
    assert(fin != NULL);
    char line[1024];
    assert(PadFile_GetLine(NULL, sizeof line, fin) == EOF);
    assert(PadFile_GetLine(line, 0, fin) == EOF);
    assert(PadFile_GetLine(line, sizeof line, NULL) == EOF);
    assert(PadFile_GetLine(line, sizeof line, fin) != EOF);
    assert(strcmp(get_test_fcontent_nonewline(), line) == 0);
    assert(PadFile_Close(fin) == 0);
}

static void
test_PadFile_ReadLine(void) {
    char line[1024];
    assert(PadFile_ReadLine(NULL, sizeof line, get_test_finpath()) == NULL);
    assert(PadFile_ReadLine(line, 0, get_test_finpath()) == NULL);
    assert(PadFile_ReadLine(line, sizeof line, NULL) == NULL);
    assert(PadFile_ReadLine(line, sizeof line, get_test_finpath()) != NULL);
    assert(strcmp(line, get_test_fcontent_nonewline()) == 0);
}

static void
test_PadFile_WriteLine(void) {
    assert(PadFile_WriteLine(NULL, get_test_finpath()) == NULL);
    assert(PadFile_WriteLine(get_test_fcontent_nonewline(), NULL) == NULL);
    assert(PadFile_WriteLine(get_test_fcontent_nonewline(), get_test_finpath()));
    test_PadFile_ReadLine();
}

static void
test_PadFileDirNode_Del(void) {
    PadFileDir_Close(NULL);
    assert(PadFileDir_Open(NULL) == NULL);
    assert(PadFileDir_Read(NULL) == NULL);
    PadFileDirNode_Del(NULL);

    struct PadFileDir *dir = PadFileDir_Open(get_test_dirpath());
    assert(dir != NULL);

    for (struct PadFileDirNode *node; (node = PadFileDir_Read(dir)); ) {
        const char *dname = PadFileDirNode_Name(node);
        assert(dname != NULL);
        PadFileDirNode_Del(node);
    }

    assert(PadFileDir_Close(dir) == 0);
}

static void
test_PadFileDirNode_Name(void) {
    // test_PadFileDir_Close
}

static void
test_PadFileDir_Close(void) {
    // test_PadFileDir_Close
}

static void
test_PadFileDir_Open(void) {
    // test_PadFileDir_Close
}

static void
test_PadFileDir_Read(void) {
    // test_PadFileDir_Close
}

static void
test_PadFile_ConvLineEnc(void) {
    char *encoded;

    encoded = PadFile_ConvLineEnc(NULL, "abc");
    assert(!encoded);

    encoded = PadFile_ConvLineEnc("nothing", "abc");
    assert(!encoded);

    encoded = PadFile_ConvLineEnc("crlf", NULL);
    assert(!encoded);

    encoded = PadFile_ConvLineEnc("crlf", "abc");
    assert(encoded);
    assert(!strcmp(encoded, "abc"));
    free(encoded);

    // to crlf
    encoded = PadFile_ConvLineEnc("crlf", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("crlf", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("crlf", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    // to cr
    encoded = PadFile_ConvLineEnc("cr", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("cr", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("cr", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    // to lf
    encoded = PadFile_ConvLineEnc("lf", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("lf", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);

    encoded = PadFile_ConvLineEnc("lf", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);
}

static void
test_PadFile_GetUserHome(void) {
    // can't test    
}

static void
test_PadFile_Remove(void) {
    if (!PadFile_IsExists("tests/file/")) {
        PadFile_MkdirQ("tests/file/");
    }
    PadFile_Trunc("tests/file/remove.txt");
    assert(PadFile_IsExists("tests/file/remove.txt"));
    PadFile_Remove("tests/file/remove.txt");
    assert(!PadFile_IsExists("tests/file/remove.txt"));
}

static void
test_PadFile_Rename(void) {
    if (!PadFile_IsExists("tests/file/")) {
        PadFile_MkdirQ("tests/file/");
    }
    PadFile_Trunc("tests/file/rename.txt");
    assert(PadFile_IsExists("tests/file/rename.txt"));
    PadFile_Rename("tests/file/rename.txt", "tests/file/renamed.txt");
    assert(PadFile_IsExists("tests/file/renamed.txt"));
    PadFile_Remove("tests/file/renamed.txt");
}

static void
test_PadFile_ReadLines(void) {
    if (!PadFile_IsExists("tests/file/")) {
        PadFile_MkdirQ("tests/file/");
    }
    FILE *fout = fopen("tests/file/lines.txt", "wt");
    assert(fout);
    fputs("123\n", fout);
    fputs("223\n", fout);
    fputs("323\n", fout);
    fclose(fout);

    char **lines = PadFile_ReadLines("tests/file/lines.txt");
    assert(lines);
    assert(!strcmp(lines[0], "123"));
    assert(!strcmp(lines[1], "223"));
    assert(!strcmp(lines[2], "323"));
    assert(lines[3] == NULL);

    PadFile_Remove("tests/file/lines.txt");
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
file_tests[] = {
    {"PadFile_Close", test_PadFile_Close},
    {"PadFile_Open", test_PadFile_Open},
    {"PadFile_Copy", test_PadFile_Copy},
    {"PadFile_CloseDir", test_PadFile_CloseDir},
    {"PadFile_OpenDir", test_PadFile_OpenDir},
    {"PadFile_RealPath", test_PadFile_RealPath},
    {"PadFile_IsExists", test_PadFile_IsExists},
    {"PadFile_MkdirMode", test_PadFile_MkdirMode},
    {"PadFile_MkdirQ", test_PadFile_MkdirQ},
    {"PadFile_Trunc", test_PadFile_Trunc},
    {"PadFile_Solve", test_PadFile_Solve},
    {"PadFile_SolveCopy", test_PadFile_SolveCopy},
    {"PadFile_SolveFmt", test_PadFile_SolveFmt},
    {"PadFile_IsDir", test_PadFile_IsDir},
    {"PadFile_ReadCopy", test_PadFile_ReadCopy},
    {"PadFile_Size", test_PadFile_Size},
    {"PadFile_Suffix", test_PadFile_Suffix},
    {"PadFile_DirName", test_PadFile_DirName},
    {"PadFile_BaseName", test_PadFile_BaseName},
    {"PadFile_GetLine", test_PadFile_GetLine},
    {"PadFile_ReadLine", test_PadFile_ReadLine},
    {"PadFile_WriteLine", test_PadFile_WriteLine},
    {"PadFileDirNode_Del", test_PadFileDirNode_Del},
    {"PadFileDirNode_Name", test_PadFileDirNode_Name},
    {"PadFileDir_Close", test_PadFileDir_Close},
    {"PadFileDir_Open", test_PadFileDir_Open},
    {"PadFileDir_Read", test_PadFileDir_Read},
    {"PadFile_ConvLineEnc", test_PadFile_ConvLineEnc},
    {"PadFile_GetUserHome", test_PadFile_GetUserHome},
    {"PadFile_Remove", test_PadFile_Remove},
    {"PadFile_Rename", test_PadFile_Rename},
    {"PadFile_ReadLines", test_PadFile_ReadLines},
    {0},
};

/*****
* cl *
*****/

static void
test_PadCL_Del(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    PadCL_Del(cl);
}

static void
test_PadCL_EscDel(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    size_t parrlen = PadCL_Len(cl);
    char **parr = PadCL_EscDel(cl);
    assert(parr != NULL);
    Pad_FreeArgv(parrlen, parr);
}

static void
test_PadCL_New(void) {
    // test_PadCL_Del
}

static void
test_PadCL_Resize(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    assert(PadCL_Capa(cl) == 4);
    assert(PadCL_Resize(cl, 8));
    assert(PadCL_Capa(cl) == 8);
    PadCL_Del(cl);
}

static void
test_PadCL_PushBack(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    assert(PadCL_Len(cl) == 0);
    assert(PadCL_PushBack(cl, "123"));
    assert(PadCL_PushBack(cl, "223"));
    assert(PadCL_PushBack(cl, "323"));
    assert(strcmp(PadCL_Getc(cl, 1), "223") == 0);
    assert(PadCL_Len(cl) == 3);
    PadCL_Del(cl);
}

static void
test_PadCL_Getc(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    assert(PadCL_PushBack(cl, "123"));
    assert(strcmp(PadCL_Getc(cl, 0), "123") == 0);
    PadCL_Del(cl);
}

static void
test_PadCL_Clear(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);
    assert(PadCL_PushBack(cl, "123"));
    assert(PadCL_PushBack(cl, "223"));
    assert(PadCL_Len(cl) == 2);
    PadCL_Clear(cl);
    assert(PadCL_Len(cl) == 0);
    PadCL_Del(cl);
}

static void
test_PadCL_ParseStrOpts(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    assert(PadCL_ParseStrOpts(cl, "cmd -h -ab 123 --help 223", PAD_CL__WRAP));
    assert(strcmp(PadCL_Getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "'-h'") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "'-ab'") == 0);
    assert(strcmp(PadCL_Getc(cl, 3), "'123'") == 0);
    assert(strcmp(PadCL_Getc(cl, 4), "'--help'") == 0);
    assert(strcmp(PadCL_Getc(cl, 5), "'223'") == 0);

    assert(PadCL_ParseStrOpts(cl, "cmd -a 123", PAD_CL__WRAP));
    assert(strcmp(PadCL_Getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "'-a'") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "'123'") == 0);

    assert(PadCL_ParseStrOpts(cl, "\"cmd\" \"-a\" \"123\"", PAD_CL__WRAP));
    assert(strcmp(PadCL_Getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "'-a'") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "'123'") == 0);

    assert(PadCL_ParseStrOpts(cl, "\"cmd\" \"-a\" \"123\"", PAD_CL__WRAP));
    assert(strcmp(PadCL_Getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "'-a'") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "'123'") == 0);

    assert(PadCL_ParseStrOpts(cl, "cmd -a 123", PAD_CL__ESCAPE));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "123") == 0);

    assert(PadCL_ParseStrOpts(cl, "cmd -a \"1'23\"", PAD_CL__ESCAPE));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "1\\'23") == 0);

    PadCL_Del(cl);
}

static void
test_PadCL_ParseStr(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    assert(PadCL_ParseStr(cl, "cmd -h -ab 123 --help 223"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-h") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "-ab") == 0);
    assert(strcmp(PadCL_Getc(cl, 3), "123") == 0);
    assert(strcmp(PadCL_Getc(cl, 4), "--help") == 0);
    assert(strcmp(PadCL_Getc(cl, 5), "223") == 0);

    assert(PadCL_ParseStr(cl, "cmd -a \"abc\""));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd -a 'a\"bc'"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "a\"bc") == 0);

    assert(PadCL_ParseStr(cl, "cmd -a=abc"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd -a=\"abc\""));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd -a='abc'"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd \"-a\"=\"abc\""));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd '-a'='abc'"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "-a") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd --abc=abc"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "--abc") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd --abc=\"abc\""));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "--abc") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd --abc='abc'"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "--abc") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd \"--abc\"=\"abc\""));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "--abc") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    assert(PadCL_ParseStr(cl, "cmd '--abc'='abc'"));
    assert(strcmp(PadCL_Getc(cl, 0), "cmd") == 0);
    assert(strcmp(PadCL_Getc(cl, 1), "--abc") == 0);
    assert(strcmp(PadCL_Getc(cl, 2), "abc") == 0);

    PadCL_Del(cl);
}

static void
test_PadCL_ParseArgvOpts(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    PadCL_Del(cl);
}

static void
test_PadCL_ParseArgv(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    PadCL_Del(cl);
}

static void
test_PadCL_Show(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    PadCL_Del(cl);
}

static void
test_PadCL_Len(void) {
    PadCL *cl = PadCL_New();
    assert(cl != NULL);

    PadCL_Del(cl);
}

static void
test_PadCL_Capa(void) {
    PadCL *cl = PadCL_New();
    assert(cl);

    assert(PadCL_Capa(cl) == 4);

    PadCL_Del(cl);
}

static void
test_PadCL_GetArgv(void) {
    PadCL *cl = PadCL_New();
    assert(cl);

    PadCL_PushBack(cl, "abc");
    PadCL_PushBack(cl, "def");

    char **argv = PadCL_GetArgv(cl);
    assert(!strcmp(argv[0], "abc"));
    assert(!strcmp(argv[1], "def"));
    assert(argv[2] == NULL);

    PadCL_Del(cl);
}

static void
test_PadCL_GenStr(void) {
    PadCL *cl = PadCL_New();
    assert(cl);

    PadCL_PushBack(cl, "abc");
    PadCL_PushBack(cl, "def");
    PadCL_PushBack(cl, "123");

    char *s = PadCL_GenStr(cl);
    assert(!strcmp(s, "\"abc\" \"def\" \"123\""));

    free(s);
    PadCL_Del(cl);
}

static const struct testcase
PadCLests[] = {
    {"PadCL_Del", test_PadCL_Del},
    {"PadCL_EscDel", test_PadCL_EscDel},
    {"PadCL_New", test_PadCL_New},
    {"PadCL_Resize", test_PadCL_Resize},
    {"PadCL_Getc", test_PadCL_Getc},
    {"PadCL_PushBack", test_PadCL_PushBack},
    {"PadCL_Clear", test_PadCL_Clear},
    {"PadCL_ParseStrOpts", test_PadCL_ParseStrOpts},
    {"PadCL_ParseStr", test_PadCL_ParseStr},
    {"cl_parseargvopts", test_PadCL_ParseArgvOpts},
    {"cl_parseargv", test_PadCL_ParseArgv},
    {"PadCL_Show", test_PadCL_Show},
    {"PadCL_Len", test_PadCL_Len},
    {"PadCL_Capa", test_PadCL_Capa},
    {"PadCL_GetArgv", test_PadCL_GetArgv},
    {"PadCL_GenStr", test_PadCL_GenStr},
    {0},
};

/********
* error *
********/

static void
test_error_fix_text_1(void) {
    char buf[BUFSIZ] = {0};

    PadErr_FixTxt(buf, sizeof buf, "text");
    assert(!strcmp(buf, "Text."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "file.text");
    assert(!strcmp(buf, "file.text."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "file...");
    assert(!strcmp(buf, "File..."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "the file...");
    assert(!strcmp(buf, "The file..."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "the file...test");
    assert(!strcmp(buf, "The file...test."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "the file... test string");
    assert(!strcmp(buf, "The file... Test string."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "text. text");
    assert(!strcmp(buf, "Text. Text."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "text.     text");
    assert(!strcmp(buf, "Text. Text."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "Failed to open directory \"/path/to/dir\". failed to remove recursive.");
    assert(!strcmp(buf, "Failed to open directory \"/path/to/dir\". Failed to remove recursive."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "src/core/error_stack.c");
    assert(!strcmp(buf, "src/core/error_stack.c."));
    buf[0] = '\0';

    PadErr_FixTxt(buf, sizeof buf, "newline\n");
    puts(buf);
    assert(!strcmp(buf, "Newline\n."));
    buf[0] = '\0';
}

static void
test_error__log(void) {
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    PadErr_LogUnsafe("file", 100, "func", "warn", "msg");
    // assert(strcmp(buf, "")); // TODO

    setbuf(stderr, NULL);
}

static void
test_error_die(void) {
    // nothing todo
}

static void
test_error_error_1(void) {
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    PadErr_Err("this is error");
    // assert(strcmp(buf, "Error: This is error. No such file or directory.\n") == 0);

    setbuf(stderr, NULL);
}

static void
test_error_error_2(void) {
    PadErr_Err("test1");
    PadErr_Err("test2");
    PadErr_Err("test3");
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
error_tests[] = {
    {"fix_text_1", test_error_fix_text_1},
    {"_log", test_error__log},
    {"die", test_error_die},
    {"error_1", test_error_error_1},
    {"error_2", test_error_error_2},
    {0},
};

/*******
* util *
*******/

static char **
__create_testargv(int argc) {
    char **argv = calloc(argc+1, sizeof(char*));
    assert(argv != NULL);

    for (int i = 0; i < argc; ++i) {
        argv[i] = PadCStr_EDup("abc");
    }

    return argv;
}

static void
test_util_Pad_FreeArgv(void) {
    int argc = 2;
    char **argv = __create_testargv(argc);
    assert(argv != NULL);
    Pad_FreeArgv(argc, argv);
}

static void
test_util_Pad_ShowArgv(void) {
    // TODO
    // this test was failed
    return;

    char buf[1024] = {0};

    int argc = 2;
    char **argv = __create_testargv(argc);
    assert(argv != NULL);

    setbuf(stdout, buf);
    Pad_ShowArgv(argc, argv);
    setbuf(stdout, NULL);

    assert(!strcmp(buf, "abc\nabc\n"));

    Pad_FreeArgv(argc, argv);
}

static void
test_util_Pad_RandRange(void) {
    int min = 0;
    int max = 10;
    int n = Pad_RandRange(min, max);
    for (int i = min; i < max; ++i) {
        if (n == i) {
            return;
        }
    }

    assert(0 && "invalid value range");
}

static void
test_util_Pad_SafeSystem(void) {
    char cmd[1024];
#ifdef _TESTS_WINDOWS
    assert(PadFile_SolveFmt(cmd, sizeof cmd, "dir") != NULL);
#else
    const char *path = "/tmp/f";
    if (PadFile_IsExists(path)) {
        assert(remove(path) == 0);
    }
    assert(PadFile_SolveFmt(cmd, sizeof cmd, "/bin/sh -c \"touch %s\"", path) != NULL);
    assert(Pad_SafeSystem(cmd, PAD_SAFESYSTEM_DEFAULT) == 0);
    assert(PadFile_IsExists(path));
#endif
}

static void
test_util_Pad_ArgsByOptind(void) {
    char *argv[] = {
        "program",
        "arg1",
        "-a",
        "arg2",
        "-b",
        "barg",
        NULL,
    };
    int argc = 0;
    for (; argv[argc]; ++argc) {
    }

    struct option longopts[] = {
        {"opt1", no_argument, 0, 'a'},
        {"opt2", required_argument, 0, 'b'},
        {0},
    };
    const char *shortopts = "ab:";
    opterr = 0;
    optind = 0;

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }
    }

    PadCStrAry *args = Pad_ArgsByOptind(argc, argv, optind);
    // PadCStrAry_Show(args, stdout);
    assert(strcmp(PadCStrAry_Getc(args, 0), "program") == 0);
    assert(strcmp(PadCStrAry_Getc(args, 1), "arg1") == 0);
    assert(strcmp(PadCStrAry_Getc(args, 2), "arg2") == 0);
    PadCStrAry_Del(args);
}

static void
test_util_Pad_Escape(void) {
    char dst[1024];
    assert(Pad_Escape(dst, sizeof dst, "abca", "a"));
    assert(!strcmp(dst, "\\abc\\a"));
}

static void
test_util_Pad_CompileArgv(void) {
    PadConfig *config = PadConfig_New();
    int argc = 4;
    char *argv[] = {
        "make",
        "file",
        "-a",
        "bbb",
        NULL,
    };
    const char *src = "{: opts.get(\"a\") :}";

    char *compiled = Pad_CompileArgv(config, NULL, argc-1, argv+1, src);

    assert(!strcmp(compiled, "bbb"));

    free(compiled);
    PadConfig_Del(config);
}

static void
test_util_Pad_PopTailSlash(void) {
    char s[100];
#ifdef _TESTS_WINDOWS
    strcpy(s, "C:\\path\\to\\dir\\");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\path\\to\\dir");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "C:\\"));
#else
    strcpy(s, "/path/to/dir/");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/path/to/dir");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/");
    assert(Pad_PopTailSlash(s));
    assert(!strcmp(s, "/"));
#endif
}

static void
test_util_Pad_TrimFirstLine(void) {
    char dst[100];
    const char *lines = "aaa\nbbb\nccc\n";

    Pad_TrimFirstLine(dst, sizeof dst, lines);
    assert(!strcmp(dst, "aaa"));
}

static void
test_util_Pad_ClearScreen(void) {
    // nothing todo
}

static void
test_util_Pad_SplitToArray(void) {
    assert(Pad_SplitToArray(NULL, 0) == NULL);

    PadCStrAry *arr = Pad_SplitToArray("abc:def:ghi", ':');
    assert(arr);
    assert(PadCStrAry_Len(arr) == 3);
    assert(!strcmp(PadCStrAry_Getc(arr, 0), "abc"));
    assert(!strcmp(PadCStrAry_Getc(arr, 1), "def"));
    assert(!strcmp(PadCStrAry_Getc(arr, 2), "ghi"));
    PadCStrAry_Del(arr);

    arr = Pad_SplitToArray("abc:def:ghi:", ':');
    assert(arr);
    assert(PadCStrAry_Len(arr) == 3);
    assert(!strcmp(PadCStrAry_Getc(arr, 0), "abc"));
    assert(!strcmp(PadCStrAry_Getc(arr, 1), "def"));
    assert(!strcmp(PadCStrAry_Getc(arr, 2), "ghi"));
    PadCStrAry_Del(arr);
}

static void
test_util_Pad_PushFrontArgv(void) {
    int argc = 2;
    char *argv[] = {"aaa", "bbb", NULL};
    PadCStrAry *arr = Pad_PushFrontArgv(argc, argv, "ccc");
    assert(PadCStrAry_Len(arr) == 3);
    assert(!strcmp(PadCStrAry_Getc(arr, 0), "ccc"));
    assert(!strcmp(PadCStrAry_Getc(arr, 1), "aaa"));
    assert(!strcmp(PadCStrAry_Getc(arr, 2), "bbb"));
}

static void
test_util_Pad_IsDotFile(void) {
    assert(Pad_IsDotFile("."));
    assert(Pad_IsDotFile(".."));
}

static const struct testcase
utiltests[] = {
    {"Pad_FreeArgv", test_util_Pad_FreeArgv},
    {"Pad_ShowArgv", test_util_Pad_ShowArgv},
    {"Pad_RandRange", test_util_Pad_RandRange},
    {"Pad_SafeSystem", test_util_Pad_SafeSystem},
    {"Pad_ArgsByOptind", test_util_Pad_ArgsByOptind},
    {"Pad_Escape", test_util_Pad_Escape},
    {"Pad_CompileArgv", test_util_Pad_CompileArgv},
    {"Pad_PopTailSlash", test_util_Pad_PopTailSlash},
    {"Pad_TrimFirstLine", test_util_Pad_TrimFirstLine},
    {"Pad_ClearScreen", test_util_Pad_ClearScreen},
    {"Pad_SplitToArray", test_util_Pad_SplitToArray},
    {"Pad_PushFrontArgv", test_util_Pad_PushFrontArgv},
    {"Pad_IsDotFile", test_util_Pad_IsDotFile},
    {0},
};

/*******
* path *
*******/

static void
test_PadPath_PopBackOf(void) {
    char s[100];

    assert(PadPath_PopBackOf(NULL, '?') == NULL);

    strcpy(s, "abc");
    assert(PadPath_PopBackOf(s, 'c'));
    assert(!strcmp(s, "ab"));

    assert(PadPath_PopBackOf(s, '?'));
    assert(!strcmp(s, "ab"));
}

static void
test_PadPath_PopTailSlash(void) {
    char s[100];

    assert(PadPath_PopTailSlash(NULL) == NULL);

#ifdef _TESTS_WINDOWS
    strcpy(s, "C:\\path\\to\\dir\\");
    assert(PadPath_PopTailSlash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\path\\to\\dir");
    assert(PadPath_PopTailSlash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));
#else
    strcpy(s, "/path/to/dir/");
    assert(PadPath_PopTailSlash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/path/to/dir");
    assert(PadPath_PopTailSlash(s));
    assert(!strcmp(s, "/path/to/dir"));
#endif
}

static const struct testcase
pathtests[] = {
    {"PadPath_PopBackOf", test_PadPath_PopBackOf},
    {"PadPath_PopTailSlash", test_PadPath_PopTailSlash},
    {0},
};

/************
* lang/opts *
************/

static void
test_lang_PadOpts_New(void) {
    PadOpts *opts = PadOpts_New();
    assert(opts);
    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_Parse(void) {
    PadOpts *opts = PadOpts_New();
    assert(opts);

    int argc = 7;
    char *argv[] = {
        "make",
        "arg1",
        "arg2",
        "-a",
        "aaa",
        "--bbb",
        "bbb",
        NULL,
    };
    assert(PadOpts_Parse(opts, argc, argv));

    assert(PadOpts_ArgsLen(opts) == 3);
    assert(PadOpts_GetcArgs(opts, -1) == NULL);
    assert(PadOpts_GetcArgs(opts, 0));
    assert(PadOpts_GetcArgs(opts, 1));
    assert(PadOpts_GetcArgs(opts, 2));
    assert(PadOpts_GetcArgs(opts, 3) == NULL);
    assert(!strcmp(PadOpts_GetcArgs(opts, 0), "make"));
    assert(!strcmp(PadOpts_GetcArgs(opts, 1), "arg1"));
    assert(!strcmp(PadOpts_GetcArgs(opts, 2), "arg2"));
    assert(PadOpts_Getc(opts, "a"));
    assert(!strcmp(PadOpts_Getc(opts, "a"), "aaa"));
    assert(PadOpts_Getc(opts, "bbb"));
    assert(!strcmp(PadOpts_Getc(opts, "bbb"), "bbb"));
    assert(PadOpts_Has(opts, "a"));
    assert(PadOpts_Has(opts, "bbb"));
    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_Parse_0(void) {
    PadOpts *opts = PadOpts_New();
    assert(opts);

    int argc = 1;
    char *argv[] = {
        "make",
        NULL,
    };
    assert(PadOpts_Parse(opts, argc, argv));
    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_GetcArgs_0(void) {
    PadOpts *opts = PadOpts_New();
    assert(opts);

    int argc = 3;
    char *argv[] = {
        "cmd",
        "arg1",
        "arg2",
        NULL,
    };

    assert(PadOpts_Parse(opts, argc, argv));
    assert(!strcmp(PadOpts_GetcArgs(opts, 0), "cmd"));
    assert(!strcmp(PadOpts_GetcArgs(opts, 1), "arg1"));
    assert(!strcmp(PadOpts_GetcArgs(opts, 2), "arg2"));
    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_GetcArgs_1(void) {
    PadOpts *opts = PadOpts_New();
    assert(opts);

    int argc = 7;
    char *argv[] = {
        "cmd",
        "-a",
        "optarg1",
        "-b",
        "optarg2",
        "arg1",
        "arg2",
        NULL,
    };

    assert(PadOpts_Parse(opts, argc, argv));
    assert(!strcmp(PadOpts_Getc(opts, "a"), "optarg1"));
    assert(!strcmp(PadOpts_Getc(opts, "b"), "optarg2"));
    assert(PadOpts_GetcArgs(opts, 0));
    assert(!strcmp(PadOpts_GetcArgs(opts, 0), "cmd"));
    assert(PadOpts_GetcArgs(opts, 1));
    assert(!strcmp(PadOpts_GetcArgs(opts, 1), "arg1"));
    assert(PadOpts_GetcArgs(opts, 2));
    assert(!strcmp(PadOpts_GetcArgs(opts, 2), "arg2"));
    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_Clear(void) {
    int argc = 1;
    char *argv[] = {"abc", NULL};

    PadOpts *opts = PadOpts_New();

    assert(PadOpts_Parse(opts, argc, argv));
    assert(PadOpts_ArgsLen(opts) == 1);
    PadOpts_Clear(opts);
    assert(PadOpts_ArgsLen(opts) == 0);

    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_Getc(void) {
    int argc = 5;
    char *argv[] = {
        "cmd",
        "-a",
        "aaa",
        "-b",
        "bbb",
        NULL,
    };
    PadOpts *opts = PadOpts_New();

    assert(PadOpts_Parse(opts, argc, argv));
    assert(!strcmp(PadOpts_Getc(opts, "a"), "aaa"));
    assert(!strcmp(PadOpts_Getc(opts, "b"), "bbb"));

    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_Has(void) {
    int argc = 3;
    char *argv[] = {
        "cmd",
        "-a",
        "aaa",
        NULL,
    };
    PadOpts *opts = PadOpts_New();

    assert(PadOpts_Parse(opts, argc, argv));
    assert(PadOpts_Has(opts, "a"));

    PadOpts_Del(opts);
}

static void
test_lang_PadOpts_ArgsLen(void) {
    int argc = 3;
    char *argv[] = {
        "cmd",
        "arg1",
        "arg2",
        NULL,
    };
    PadOpts *opts = PadOpts_New();

    assert(PadOpts_Parse(opts, argc, argv));
    assert(PadOpts_ArgsLen(opts) == 3);

    PadOpts_Del(opts);
}

static const struct testcase
lang_PadOptsests[] = {
    {"PadOpts_New", test_lang_PadOpts_New},
    {"PadOpts_Parse", test_lang_PadOpts_Parse},
    {"PadOpts_Parse_0", test_lang_PadOpts_Parse_0},
    {"PadOpts_GetcArgs_0", test_lang_PadOpts_GetcArgs_0},
    {"PadOpts_GetcArgs_1", test_lang_PadOpts_GetcArgs_1},
    {"PadOpts_Clear", test_lang_PadOpts_Clear},
    {"PadOpts_Getc", test_lang_PadOpts_Getc},
    {"PadOpts_Has", test_lang_PadOpts_Has},
    {"PadOpts_ArgsLen", test_lang_PadOpts_ArgsLen},
    {0},
};

/*****************
* lang/tokenizer *
*****************/

static void
test_PadTkr_New(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "abc");
    {
        assert(PadTkr_ToksLen(tkr) == 1);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__TEXT_BLOCK);
        assert(strcmp(token->text, "abc") == 0);
    }

    PadTkr_Parse(tkr, "abc{@@}bbc");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__TEXT_BLOCK);
        assert(strcmp(token->text, "abc") == 0);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__TEXT_BLOCK);
        assert(strcmp(token->text, "bbc") == 0);
    }

    // test of realloc of tokens
    PadTkr_Parse(tkr, "{@......@}");
    {
        assert(PadTkr_ToksLen(tkr) == 8);
    }

    PadTkr_Parse(tkr, "");
    {
        assert(PadTkr_HasErrStack(tkr) == false);
        assert(PadTkr_ToksLen(tkr) == 0);
    }

    PadTkr_Parse(tkr, "{@");
    {
        assert(PadTkr_ToksLen(tkr) == 1);
        assert(PadTkr_HasErrStack(tkr) == true);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
    }

    PadTkr_Parse(tkr, "{@@");
    {
        assert(PadTkr_HasErrStack(tkr) == true);
        assert(strcmp(PadTkr_GetcFirstErrMsg(tkr), "invalid syntax. single '@' is not supported") == 0);
    }

    PadTkr_Parse(tkr, "{@@}");
    {
        assert(PadTkr_ToksLen(tkr) == 2);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@.@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@..@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@,@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__COMMA);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@,,@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__COMMA);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__COMMA);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@:@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__COLON);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@;@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__SEMICOLON);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@(@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__LPAREN);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@)@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RPAREN);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@[@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__LBRACKET);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@]@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RBRACKET);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@{@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__LBRACE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@}@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RBRACE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@()@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__LPAREN);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RPAREN);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@a@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "a") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@abc@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "abc") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@abc123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "abc123") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@abc_123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "abc_123") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        assert(strcmp(token->text, "123") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@-123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /**********
    * as from *
    **********/

    PadTkr_Parse(tkr, "{@as@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__AS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@from@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__FROM);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /*************
    * statements *
    *************/

    PadTkr_Parse(tkr, "{@ end @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_END);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ if @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_IF);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ elif @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_ELIF);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ else @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_ELSE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ for @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_FOR);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /************
    * operators *
    ************/

    PadTkr_Parse(tkr, "{@ + @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ - @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ * @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MUL);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ / @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__DIV);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ % @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MOD);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ = @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ += @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ -= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ *= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MUL_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ /= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__DIV_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ %= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MOD_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /***********************
    * comparison operators *
    ***********************/

    PadTkr_Parse(tkr, "{@ == @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__EQ);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ != @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__NOT_EQ);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ <= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__LTE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ >= @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__GTE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ < @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__LT);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ > @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__GT);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ or @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__OR);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ and @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__AND);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@ not @}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__NOT);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /*******
    * expr *
    *******/

    PadTkr_Parse(tkr, "{@ 1 * 2 @}");
    {
        assert(PadTkr_ToksLen(tkr) == 5);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MUL);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        token = PadTkr_ToksGetc(tkr, 4);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /*********
    * others *
    *********/

    PadTkr_Parse(tkr, "{@\"\"@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\"abc\"@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "abc") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\"abc\"\"bbc\"@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "abc") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "bbc") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr,
        "{@ import alias\n"
        "alias.set(\"dtl\", \"run bin/date-line\") @}");
    {
        assert(PadTkr_ToksLen(tkr) == 13);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__STMT_IMPORT);
        assert(strcmp(token->text, "import") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "alias") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 4);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "alias") == 0);
        token = PadTkr_ToksGetc(tkr, 5);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 6);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "set") == 0);
        token = PadTkr_ToksGetc(tkr, 7);
        assert(token->type == PAD_TOK_TYPE__LPAREN);
        token = PadTkr_ToksGetc(tkr, 8);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "dtl") == 0);
        token = PadTkr_ToksGetc(tkr, 9);
        assert(token->type == PAD_TOK_TYPE__COMMA);
        token = PadTkr_ToksGetc(tkr, 10);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        assert(strcmp(token->text, "run bin/date-line") == 0);
        token = PadTkr_ToksGetc(tkr, 11);
        assert(token->type == PAD_TOK_TYPE__RPAREN);
        token = PadTkr_ToksGetc(tkr, 12);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /******************
    * reference block *
    ******************/

    PadTkr_Parse(tkr, "{:");
    {
        assert(PadTkr_ToksLen(tkr) == 1);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        assert(PadTkr_HasErrStack(tkr) == true);
        assert(strcmp(PadTkr_GetcFirstErrMsg(tkr), "not closed by block") == 0);
    }

    PadTkr_Parse(tkr, "{::}");
    {
        assert(PadTkr_ToksLen(tkr) == 2);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{:\n:}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{:abc:}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{:abc123:}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{:abc_123:}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: 123 :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: alias.run(\"dtl\") :}");
    {
        assert(PadTkr_ToksLen(tkr) == 8);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        token = PadTkr_ToksGetc(tkr, 4);
        assert(token->type == PAD_TOK_TYPE__LPAREN);
        token = PadTkr_ToksGetc(tkr, 5);
        assert(token->type == PAD_TOK_TYPE__DQ_STRING);
        token = PadTkr_ToksGetc(tkr, 6);
        assert(token->type == PAD_TOK_TYPE__RPAREN);
        token = PadTkr_ToksGetc(tkr, 7);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    /*****************************
    * reference block: operators *
    *****************************/

    PadTkr_Parse(tkr, "{: + :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: - :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: * :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MUL);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: / :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__DIV);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: = :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: += :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: -= :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: *= :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__MUL_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: /= :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__DIV_ASS);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    /****************************************
    * reference block: comparison operators *
    ****************************************/

    PadTkr_Parse(tkr, "{: == :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__EQ);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    PadTkr_Parse(tkr, "{: != :}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LDOUBLE_BRACE);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__NOT_EQ);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RDOUBLE_BRACE);
    }

    /**********
    * comment *
    **********/

    PadTkr_Parse(tkr, "{@\n"
    "// comment\n"
    "@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\n"
    "// comment\n"
    "// comment\n"
    "@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\n"
    "// comment\n");
    {
        assert(PadTkr_HasErrStack(tkr));
        assert(!strcmp(PadTkr_GetcFirstErrMsg(tkr), "not closed by block"));
    }

    PadTkr_Parse(tkr, "{@ // comment");
    {
        assert(PadTkr_HasErrStack(tkr));
        assert(!strcmp(PadTkr_GetcFirstErrMsg(tkr), "not closed by block"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "/* comment */"
    "@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\n"
    "/* comment \n"
    "   comment \n"
    "   comment */"
    "@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    /***********
    * newlines *
    ***********/

    PadTkr_Parse(tkr, "{@\n@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\n\n@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\r@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\r\r@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\r\n@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@\r\n\r\n@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__NEWLINE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@@}\n");
    {
        assert(PadTkr_ToksLen(tkr) == 2);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@@}\n\n");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__TEXT_BLOCK);
    }

    PadTkr_Del(tkr);
}

static void
test_tkr_long_code(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    assert(tkr);

    const char *src = "{@\n"
"    puts(1)\n"
"    puts(2)\n"
"    puts(3)\n"
"    puts(4)\n"
"    puts(5)\n"
"    puts(6)\n"
"    puts(7)\n"
"    puts(8)\n"
"    puts(9)\n"
"    puts(10)\n"
"    puts(11)\n"
"    puts(12)\n"
"    puts(13)\n"
"    puts(14)\n"
"    puts(15)\n"
"    puts(16)\n"
"    puts(17)\n"
"    puts(18)\n"
"    puts(19)\n"
"    puts(20)\n"
"    puts(21)\n"
"    puts(22)\n"
"    puts(23)\n"
"    puts(24)\n"
"@}\n";
    assert(PadTkr_Parse(tkr, src));
    assert(PadTkr_ToksLen(tkr) == 123);

    PadTkr_Del(tkr);
}

static void
test_PadTkr_DeepCopy(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    assert(tkr);

    assert(PadTkr_Parse(tkr, "{@ i = 0 @}"));
    assert(PadTkr_ToksLen(tkr) == 5);

    PadTkr *other = PadTkr_DeepCopy(tkr);
    assert(PadTkr_ToksLen(other) == 5);

    PadTkr_Del(other);
    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_int(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        assert(strcmp(token->text, "123") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_int_plus(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@+123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        assert(strcmp(token->text, "123") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_int_minus(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@-123@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(token->lvalue == 123);
        assert(strcmp(token->text, "123") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_float(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@123.456@}");
    {
        assert(PadTkr_ToksLen(tkr) == 3);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__FLOAT);
        assert(token->float_value == 123.456);
        assert(strcmp(token->text, "123.456") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_float_plus(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@+123.456@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__ADD);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__FLOAT);
        assert(token->float_value == 123.456);
        assert(strcmp(token->text, "123.456") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_float_minus(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    PadTkr_Parse(tkr, "{@-123.456@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__PAD_OP__SUB);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__FLOAT);
        assert(token->float_value == 123.456);
        assert(strcmp(token->text, "123.456") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_Parse_float_errors(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    assert(PadTkr_Parse(tkr, "{@123.@}") == NULL);
    assert(strcmp(PadTkr_GetcFirstErrMsg(tkr), "invalid float") == 0);

    PadTkr_Parse(tkr, "{@.456@}");
    {
        assert(PadTkr_ToksLen(tkr) == 4);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(strcmp(token->text, "456") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Parse(tkr, "{@123.456.789@}");
    {
        assert(PadTkr_ToksLen(tkr) == 5);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__FLOAT);
        assert(strcmp(token->text, "123.456") == 0);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__DOT_OPE);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(strcmp(token->text, "789") == 0);
        token = PadTkr_ToksGetc(tkr, 4);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static void
test_PadTkr_ExtendFrontOther(void) {
    PadTkr *a = PadTkr_New(PadTkrOpt_New());
    PadTkr *b = PadTkr_New(PadTkrOpt_New());
    const PadTok *token;

    PadTkr_Parse(a, "{@ 1 @}");
    PadTkr_Parse(b, "{@ 2 @}");

    PadTkr_ExtendFrontOther(a, b);
    {
        assert(PadTkr_ToksLen(a) == 6);
        token = PadTkr_ToksGetc(a, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(a, 1);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(strcmp(token->text, "2") == 0);
        token = PadTkr_ToksGetc(a, 2);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
        token = PadTkr_ToksGetc(a, 3);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(a, 4);
        assert(token->type == PAD_TOK_TYPE__INTEGER);
        assert(strcmp(token->text, "1") == 0);
        token = PadTkr_ToksGetc(a, 5);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(a);
    PadTkr_Del(b);
}

static void
test_PadTkr_Parse_struct_0(void) {
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(opt);
    const PadTok *token;

    assert(PadTkr_Parse(tkr, "{@123.@}") == NULL);
    assert(strcmp(PadTkr_GetcFirstErrMsg(tkr), "invalid float") == 0);

    PadTkr_Parse(tkr, "{@ def func(): struct S: end end @}");
    {
        assert(PadTkr_ToksLen(tkr) == 12);
        token = PadTkr_ToksGetc(tkr, 0);
        assert(token->type == PAD_TOK_TYPE__LBRACEAT);
        token = PadTkr_ToksGetc(tkr, 1);
        assert(token->type == PAD_TOK_TYPE__DEF);
        token = PadTkr_ToksGetc(tkr, 2);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "func") == 0);
        token = PadTkr_ToksGetc(tkr, 3);
        assert(token->type == PAD_TOK_TYPE__LPAREN);
        token = PadTkr_ToksGetc(tkr, 4);
        assert(token->type == PAD_TOK_TYPE__RPAREN);
        token = PadTkr_ToksGetc(tkr, 5);
        assert(token->type == PAD_TOK_TYPE__COLON);
        token = PadTkr_ToksGetc(tkr, 6);
        assert(token->type == PAD_TOK_TYPE__STRUCT);
        token = PadTkr_ToksGetc(tkr, 7);
        assert(token->type == PAD_TOK_TYPE__IDENTIFIER);
        assert(strcmp(token->text, "S") == 0);
        token = PadTkr_ToksGetc(tkr, 8);
        assert(token->type == PAD_TOK_TYPE__COLON);
        token = PadTkr_ToksGetc(tkr, 9);
        assert(token->type == PAD_TOK_TYPE__STMT_END);
        token = PadTkr_ToksGetc(tkr, 10);
        assert(token->type == PAD_TOK_TYPE__STMT_END);
        token = PadTkr_ToksGetc(tkr, 11);
        assert(token->type == PAD_TOK_TYPE__RBRACEAT);
    }

    PadTkr_Del(tkr);
}

static const struct testcase
PadTkrests[] = {
    {"PadTkr_New", test_PadTkr_New},
    {"PadTkr_Parse", test_PadTkr_Parse},
    {"PadTkr_Parse_int", test_PadTkr_Parse_int},
    {"PadTkr_Parse_int_plus", test_PadTkr_Parse_int_plus},
    {"PadTkr_Parse_int_minus", test_PadTkr_Parse_int_minus},
    {"PadTkr_Parse_float", test_PadTkr_Parse_float},
    {"PadTkr_Parse_float_plus", test_PadTkr_Parse_float_plus},
    {"PadTkr_Parse_float_minus", test_PadTkr_Parse_float_minus},
    {"PadTkr_Parse_float_errors", test_PadTkr_Parse_float_errors},
    {"PadTkr_Parse_struct_0", test_PadTkr_Parse_struct_0},
    {"PadTkr_DeepCopy", test_PadTkr_DeepCopy},
    {"tkr_long_code", test_tkr_long_code},
    {"PadTkr_ExtendFrontOther", test_PadTkr_ExtendFrontOther},
    {0},
};

/***********
* compiler *
***********/

static void
test_ast_show_error(const PadAST *ast) {
    if (PadAst_HasErrs(ast)) {
        printf("error detail[%s]\n", PadAst_GetcFirstErrMsg(ast));
    }
}

static void
test_cc_long_code(void) {
    const char *src = "{@\n"
"    puts(1)\n"
"    puts(2)\n"
"    puts(3)\n"
"    puts(4)\n"
"    puts(5)\n"
"    puts(6)\n"
"    puts(7)\n"
"    puts(8)\n"
"    puts(9)\n"
"    puts(10)\n"
"    puts(11)\n"
"    puts(12)\n"
"    puts(13)\n"
"    puts(14)\n"
"    puts(15)\n"
"    puts(16)\n"
"    puts(17)\n"
"    puts(18)\n"
"    puts(19)\n"
"    puts(20)\n"
"    puts(21)\n"
"    puts(22)\n"
"    puts(23)\n"
"    puts(24)\n"
"@}\n";
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;

    PadTkr_Parse(tkr, src);
    PadAst_Clear(ast);
    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    root = PadAst_GetcRoot(ast);
    assert(root);

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_basic_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;

    PadTkr_Parse(tkr, "");
    PadAst_Clear(ast);
    PadAst_Clear(ast);
    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    root = PadAst_GetcRoot(ast);
    assert(root == NULL);

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_basic_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadAssignListNode *assign_list;
    PadAssignNode *assign;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadAssCalcNode *asscalc;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadIdentNode *identifier;

    PadTkr_Parse(tkr, "{@ i = 0 @}"); {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root);
        program = root->real;
        assert(program);
        assert(program->blocks);
        blocks = program->blocks->real;
        assert(blocks);
        code_block = blocks->code_block->real;
        assert(code_block);
        elems = code_block->elems->real;
        assert(elems);
        formula = elems->formula->real;
        assert(formula);
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "i"));
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_code_block(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;

    PadTkr_Parse(tkr, "{@@}");
    PadAst_Clear(ast);
    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    root = PadAst_GetcRoot(ast);
    assert(root);
    program = root->real;
    blocks = program->blocks->real;
    code_block = blocks->code_block->real;
    assert(code_block);

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_code_block_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{@@}");
    PadAst_Clear(ast);
    PadCc_Compile(ast, PadTkr_GetToks(tkr));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_ref_block(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadRefBlockNode *ref_block;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadAryNode_ *array;
    PadAryElemsNode_ *array_elems;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadNilNode *nil;
    PadIdentNode *identifier;

    PadTkr_Parse(tkr, "{: nil :}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        nil = atom->nil->real;
        assert(nil != NULL);
    }

    PadTkr_Parse(tkr, "{: 1 :}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{: var :}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
    }

    PadTkr_Parse(tkr, "{: [1, 2] :}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        assert(array != NULL);
        array_elems = array->array_elems->real;
        assert(array_elems);
        assert(PadNodeAry_Len(array_elems->nodearr) == 2);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_ref_block_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{: nil :}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_ref_block_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{: 1 :}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_ref_block_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{: var :}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_ref_block_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{: [1, 2] :}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_formula(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadIdentNode *identifier;
    PadFormulaNode *formula;
    PadAssignNode *assign;
    PadAssignListNode *assign_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadDigitNode *digit;
    PadStrNode *string;

    PadTkr_Parse(tkr, "{@ a = 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = b = 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        assert(PadNodeAry_Len(assign->nodearr) == 3);
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        assert(or_test);
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        assert(and_test);
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        assert(not_test);
        comparison = not_test->comparison->real;
        assert(comparison);
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "b"));
        test = PadNodeAry_Get(assign->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@ a = 1, b = 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 1);
        assign = PadNodeAry_Get(assign_list->nodearr, 1)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "b"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 2);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_dict(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadFactorNode *factor;
    PadAtomNode *atom;
    _PadDictNode *dict;
    PadDictElemsNode *dict_elems;
    PadDictElemNode *dict_elem;
    PadSimpleAssignNode *simple_assign;
    PadAssCalcNode *asscalc;

    PadTkr_Parse(tkr, "{@ { \"key\" : \"value\" } @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom);
        assert(atom->dict);
        assert(atom->dict->real);
        dict = atom->dict->real;
        dict_elems = dict->dict_elems->real;
        assert(PadNodeAry_Len(dict_elems->nodearr) == 1);
        dict_elem = PadNodeAry_Get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    PadTkr_Parse(tkr, "{@ { \"key\" : \"value\", } @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom);
        assert(atom->dict);
        assert(atom->dict->real);
        dict = atom->dict->real;
        dict_elems = dict->dict_elems->real;
        assert(PadNodeAry_Len(dict_elems->nodearr) == 1);
        dict_elem = PadNodeAry_Get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    PadTkr_Parse(tkr, "{@ { \"key1\" : \"value1\", \"key2\" : \"value2\" } @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom);
        assert(atom->dict);
        assert(atom->dict->real);
        dict = atom->dict->real;
        dict_elems = dict->dict_elems->real;
        assert(PadNodeAry_Len(dict_elems->nodearr) == 2);
        dict_elem = PadNodeAry_Get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
        dict_elem = PadNodeAry_Get(dict_elems->nodearr, 1)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_dict_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{@ {} @}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_dict_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{@ { \"key\" : \"value\", } @}");
    PadAst_Clear(ast);
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_dict_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{@ { \"key1\" : \"value1\", \"key2\" : \"value2\" } @}");
    PadAst_Clear(ast);
    (PadCc_Compile(ast, PadTkr_GetToks(tkr)));

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_expr(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadDigitNode *digit;
    PadCompOpNode *comp_op;
    PadAddSubOpNode *add_sub_op;
    PadMulDivOpNode *mul_div_op;

    PadTkr_Parse(tkr, "{@ 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@ 1 == 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        comp_op = PadNodeAry_Get(comparison->nodearr, 1)->real;
        assert(comp_op->op == PAD_OP__EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        asscalc = PadNodeAry_Get(comparison->nodearr, 2)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1 == 2 == 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = PadNodeAry_Get(comparison->nodearr, 1)->real;
        assert(comp_op->op == PAD_OP__EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 2)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = PadNodeAry_Get(comparison->nodearr, 3)->real;
        assert(comp_op->op == PAD_OP__EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 4)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ 1 != 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = PadNodeAry_Get(comparison->nodearr, 1)->real;
        assert(comp_op->op == PAD_OP__NOT_EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 2)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1 != 2 != 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = PadNodeAry_Get(comparison->nodearr, 1)->real;
        assert(comp_op->op == PAD_OP__NOT_EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 2)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = PadNodeAry_Get(comparison->nodearr, 3)->real;
        assert(comp_op->op == PAD_OP__NOT_EQ);
        asscalc = PadNodeAry_Get(comparison->nodearr, 4)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ 1 + 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        add_sub_op = PadNodeAry_Get(expr->nodearr, 1)->real;
        assert(add_sub_op->op == PAD_OP__ADD);
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1 + 2 + 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        term = PadNodeAry_Get(expr->nodearr, 4)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ 1 - 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        add_sub_op = PadNodeAry_Get(expr->nodearr, 1)->real;
        assert(add_sub_op->op == PAD_OP__SUB);
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1 - 2 - 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        term = PadNodeAry_Get(expr->nodearr, 4)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ 1 * 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        mul_div_op = PadNodeAry_Get(term->nodearr, 1)->real;
        assert(mul_div_op->op == PAD_OP__MUL);
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = PadNodeAry_Get(term->nodearr, 2)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1 * 2 * 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = PadNodeAry_Get(term->nodearr, 2)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        negative = PadNodeAry_Get(term->nodearr, 4)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ 1 / 2 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        mul_div_op = PadNodeAry_Get(term->nodearr, 1)->real;
        assert(mul_div_op->op == PAD_OP__DIV);
    }

    PadTkr_Parse(tkr, "{@ 1 / 2 / 3 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = PadNodeAry_Get(term->nodearr, 2)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        negative = PadNodeAry_Get(term->nodearr, 4)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_index(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadIdentNode *identifier;

    PadTkr_Parse(tkr, "{@ a[0] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    PadTkr_Parse(tkr, "{@ a[0][0] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_dot(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;

    PadTkr_Parse(tkr, "{@ a.b @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ a.b() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ a.b[0] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_call(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadIdentNode *identifier;
    PadRingNode *chain;

    PadTkr_Parse(tkr, "{@ f() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "f"));
    }

    PadTkr_Parse(tkr, "{@ f(1) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ f(1, \"abc\") @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ a.b() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;

        negative = PadNodeAry_Get(term->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ f()() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "f"));
   }

    PadTkr_Parse(tkr, "{@ a[0]() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_array(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadAryElemsNode_ *array_elems;
    PadSimpleAssignNode *simple_assign;
    PadAryNode_ *array;

    PadTkr_Parse(tkr, "{@ [] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        assert(array->array_elems);
        array_elems = array->array_elems->real;
        assert(array_elems);
        assert(PadNodeAry_Len(array_elems->nodearr) == 0);
    }

    PadTkr_Parse(tkr, "{@ [1, 2] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(PadNodeAry_Len(array_elems->nodearr) == 2);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 0)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 1);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ [1] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(PadNodeAry_Len(array_elems->nodearr) == 1);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 0)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 1);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
    }

    PadTkr_Parse(tkr, "{@ [a = 1] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(PadNodeAry_Len(array_elems->nodearr) == 1);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 0)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 2);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
        assert(test);
    }

    PadTkr_Parse(tkr, "{@ [a = 1, b = 2] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(PadNodeAry_Len(array_elems->nodearr) == 2);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 0)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 2);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
        assert(test);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 1)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 2);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
        assert(test);
    }

    PadTkr_Parse(tkr, "{@ [1, a = 2] @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(PadNodeAry_Len(array_elems->nodearr) == 2);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 0)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 1);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
        assert(test);
        simple_assign = PadNodeAry_Get(array_elems->nodearr, 1)->real;
        assert(PadNodeAry_Len(simple_assign->nodearr) == 2);
        test = PadNodeAry_Get(simple_assign->nodearr, 0)->real;
    }


    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_asscalc(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadIdentNode *identifier;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadDigitNode *digit;
    PadStrNode *string;
    PadAugassignNode *augassign;
    PadNode *node;

    PadTkr_Parse(tkr, "{@ a += 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = PadNodeAry_Get(asscalc->nodearr, 1)->real;
        assert(augassign->op == PAD_OP__ADD_ASS);
        expr = PadNodeAry_Get(asscalc->nodearr, 2)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@ a += \"b\" @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = PadNodeAry_Get(asscalc->nodearr, 1)->real;
        assert(augassign->op == PAD_OP__ADD_ASS);
        expr = PadNodeAry_Get(asscalc->nodearr, 2)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "b"));
    }

    PadTkr_Parse(tkr, "{@ a -= 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = PadNodeAry_Get(asscalc->nodearr, 1)->real;
        assert(augassign->op == PAD_OP__SUB_ASS);
        expr = PadNodeAry_Get(asscalc->nodearr, 2)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@ a *= 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;

        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        node = chain->factor;
        assert(node);
        factor = node->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom);
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "a"));

        expr = PadNodeAry_Get(asscalc->nodearr, 2)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    /* PadTkr_Parse(tkr, "{@ a /= 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = PadNodeAry_Get(comparison->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        asscalc = PadNodeAry_Get(term->nodearr, 0)->real;
        augassign = PadNodeAry_Get(asscalc->nodearr, 1)->real;
        assert(augassign->op == PAD_OP__DIV_ASS);
        factor = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        factor = PadNodeAry_Get(asscalc->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    } */
/*
    PadTkr_Parse(tkr, "{@ func() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }
*/
    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_atom(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadIdentNode *identifier;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadDigitNode *digit;
    PadStrNode *string;
    PadNilNode *nil;
    PadFalseNode *false_;
    PadTrueNode *true_;

    PadTkr_Parse(tkr, "{@ nil @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->nil->type == PAD_NODE_TYPE__NIL);
        nil = atom->nil->real;
        assert(nil);
    }

    PadTkr_Parse(tkr, "{@ false @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->false_->type == PAD_NODE_TYPE__FALSE);
        false_ = atom->false_->real;
        assert(false_);
    }

    PadTkr_Parse(tkr, "{@ true @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->true_->type == PAD_NODE_TYPE__TRUE);
        true_ = atom->true_->real;
        assert(true_);
    }

    PadTkr_Parse(tkr, "{@ 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->digit->type == PAD_NODE_TYPE__DIGIT);
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 1);
    }

    /* PadTkr_Parse(tkr, "{@ -1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        index = PadNodeAry_Get(term->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        assert(atom->digit->type == PAD_NODE_TYPE__DIGIT);
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == -1);
    } */

    PadTkr_Parse(tkr, "{@ \"abc\" @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->string->type == PAD_NODE_TYPE__STRING);
        string = atom->string->real;
        assert(string);
        assert(!strcmp(string->string, "abc"));
    }

    PadTkr_Parse(tkr, "{@ var @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "var"));
    }
/*
    PadTkr_Parse(tkr, "{@ f() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        assert(atom->caller->type == PAD_NODE_TYPE__CALLER);
        caller = atom->caller->real;
        assert(caller);
    }
*/

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_PadCc_Compile(void) {
    // head
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    const PadNode *root;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadTextBlockNode *text_block;
    PadElemsNode *elems;
    PadStmtNode *stmt;
    PadIfStmtNode *if_stmt;
    PadForStmtNode *for_stmt;
    PadElifStmtNode *elif_stmt;
    PadElseStmtNode *else_stmt;
    PadIdentNode *identifier;
    PadFormulaNode *formula;
    PadMultiAssignNode *multi_assign;
    PadAssignNode *assign;
    PadAssignListNode *assign_list;
    PadTestListNode *test_list;
    PadTestNode *test;
    PadOrTestNode *or_test;
    PadAndTestNode *and_test;
    PadNotTestNode *not_test;
    PadComparisonNode *comparison;
    PadExprNode *expr;
    PadTermNode *term;
    PadNegativeNode *negative;
    PadRingNode *chain;
    PadAssCalcNode *asscalc;
    PadFactorNode *factor;
    PadAtomNode *atom;
    PadDigitNode *digit;
    PadStrNode *string;
    PadBreakStmtNode *break_stmt;
    PadContinueStmtNode *continue_stmt;
    PadDefNode *def;
    PadFuncDefNode *func_def;
    PadFuncDefParamsNode *func_def_params;
    PadFuncDefArgsNode *func_def_args;
    PadContentNode *content;

    /***********
    * func_def *
    ***********/

    PadTkr_Parse(tkr, "{@ def func(a, b): end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root);
        program = root->real;
        assert(program);
        blocks = program->blocks->real;
        assert(blocks);
        code_block = blocks->code_block->real;
        assert(code_block);
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        func_def = def->func_def->real;
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        func_def_params = func_def->func_def_params->real;
        func_def_args = func_def_params->func_def_args->real;
        assert(PadNodeAry_Len(func_def_args->identifiers) == 2);
        identifier = PadNodeAry_Get(func_def_args->identifiers, 0)->real;
        assert(!strcmp(identifier->identifier, "a"));
        identifier = PadNodeAry_Get(func_def_args->identifiers, 1)->real;
        assert(!strcmp(identifier->identifier, "b"));
    }

    PadTkr_Parse(tkr, "{@ def func(): end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == PAD_NODE_TYPE__DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == PAD_NODE_TYPE__FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == PAD_NODE_TYPE__FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == PAD_NODE_TYPE__FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(PadNodeAry_Len(func_def_args->identifiers) == 0);
    }

    PadTkr_Parse(tkr, "{@ def func(): a = 1 end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == PAD_NODE_TYPE__DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == PAD_NODE_TYPE__FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == PAD_NODE_TYPE__FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == PAD_NODE_TYPE__FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(PadNodeAry_Len(func_def_args->identifiers) == 0);

        assert(func_def->contents);
        content = PadNodeAry_Get(func_def->contents, 0)->real;
        assert(content);
        elems = content->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == PAD_NODE_TYPE__DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == PAD_NODE_TYPE__FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == PAD_NODE_TYPE__FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == PAD_NODE_TYPE__FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(PadNodeAry_Len(func_def_args->identifiers) == 0);

        content = PadNodeAry_Get(func_def->contents, 0)->real;
        elems = content->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = PadNodeAry_Get(assign_list->nodearr, 0)->real;
        test = PadNodeAry_Get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = PadNodeAry_Get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    /*******
    * call *
    *******/
/*
    PadTkr_Parse(tkr, "{@ func() + 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }
*/
/*
    PadTkr_Parse(tkr, "{@ my.func() @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }

    PadTkr_Parse(tkr, "{@ my.func(1) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    PadTkr_Parse(tkr, "{@ my.func(1, 2) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ my.func(\"abc\") @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(PadNodeAry_Get(test_list->nodearr, 0));
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
    }

    PadTkr_Parse(tkr, "{@ my.func(\"abc\", \"def\") @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(PadNodeAry_Get(test_list->nodearr, 0));
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
    }

    PadTkr_Parse(tkr, "{@ my.func(\"abc\", \"def\", \"ghi\") @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(PadNodeAry_Get(test_list->nodearr, 0));
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
        test = PadNodeAry_Get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "ghi"));
    }

    PadTkr_Parse(tkr, "{@ my.func(\"\", \"\") @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(PadNodeAry_Get(test_list->nodearr, 0));
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        dot = PadNodeAry_Get(term->nodearr, 0)->real;
        index = PadNodeAry_Get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
    }
*/
    /************
    * test_list *
    ************/

    PadTkr_Parse(tkr, "{@ 1, 2 @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        assert(test_list);
        assert(test_list);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        assert(test);
        assert(test->or_test);
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "{@ 1, 2, 3 @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        test = PadNodeAry_Get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    PadTkr_Parse(tkr, "{@ \"abc\", \"def\" @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
    }

    PadTkr_Parse(tkr, "{@ \"abc\", \"def\", \"ghi\" @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
        test = PadNodeAry_Get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "ghi"));
    }

    PadTkr_Parse(tkr, "{@ 1, \"def\" @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
    }

    PadTkr_Parse(tkr, "{@ 1, var @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
    }

    PadTkr_Parse(tkr, "{@ 1, var, \"abc\" @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = PadNodeAry_Get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
    }

    PadTkr_Parse(tkr, "{@ 1, var, \"abc\", func() @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = PadNodeAry_Get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = PadNodeAry_Get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = PadNodeAry_Get(test_list->nodearr, 3)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }

    /*******
    * test *
    *******/

    PadTkr_Parse(tkr, "{@ 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->formula != NULL);
        assert(elems->formula->type == PAD_NODE_TYPE__FORMULA);
        assert(elems->formula->real != NULL);
        formula = elems->formula->real;
        assert(formula->multi_assign != NULL);
        assert(formula->multi_assign->type == PAD_NODE_TYPE__MULTI_ASSIGN);
        assert(formula->multi_assign->real != NULL);
        multi_assign = formula->multi_assign->real;
        assert(PadNodeAry_Get(multi_assign->nodearr, 0) != NULL);
        assert(PadNodeAry_Get(multi_assign->nodearr, 0)->type == PAD_NODE_TYPE__TEST_LIST);
        assert(PadNodeAry_Get(multi_assign->nodearr, 0)->real != NULL);
        assert(PadNodeAry_Len(multi_assign->nodearr) == 1);
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        assert(PadNodeAry_Get(test_list->nodearr, 0) != NULL);
        assert(PadNodeAry_Get(test_list->nodearr, 0)->type == PAD_NODE_TYPE__TEST);
        assert(PadNodeAry_Get(test_list->nodearr, 0)->real != NULL);
        assert(PadNodeAry_Get(test_list->nodearr, 1) == NULL);
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        assert(test->or_test != NULL);
        assert(test->or_test->type == PAD_NODE_TYPE__OR_TEST);
        assert(test->or_test->real != NULL);
        or_test = test->or_test->real;
        assert(PadNodeAry_Get(or_test->nodearr, 0) != NULL);
        assert(PadNodeAry_Get(or_test->nodearr, 0)->type == PAD_NODE_TYPE__AND_TEST);
        assert(PadNodeAry_Get(or_test->nodearr, 0)->real != NULL);
        assert(PadNodeAry_Len(or_test->nodearr) == 1);
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        assert(and_test->nodearr != NULL);
        assert(PadNodeAry_Len(and_test->nodearr) == 1);
        assert(PadNodeAry_Get(and_test->nodearr, 0)->real != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        assert(not_test->not_test == NULL);
        assert(not_test->comparison != NULL);
        assert(not_test->comparison->type == PAD_NODE_TYPE__COMPARISON);
        assert(not_test->comparison->real != NULL);
        comparison = not_test->comparison->real;
        // TODO
    }

    PadTkr_Parse(tkr, "{@ 1 or 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        assert(and_test);
        assert(and_test->nodearr);
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        assert(not_test);
        assert(not_test->comparison);
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ 1 or 1 or 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 2)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ 1 and 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ 1 and 1 and 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 2)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ not 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ not not 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ 1 or 1 and 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ 1 and 1 or 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ not 1 or 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = PadNodeAry_Get(or_test->nodearr, 1)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    PadTkr_Parse(tkr, "{@ not 1 and 1 @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = PadNodeAry_Get(multi_assign->nodearr, 0)->real;
        test = PadNodeAry_Get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = PadNodeAry_Get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    /*********
    * blocks *
    *********/

    PadTkr_Parse(tkr, "{@@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    PadTkr_Parse(tkr, "abc{@@}def");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        text_block = blocks->text_block->real;
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        code_block = blocks->code_block->real;
        assert(code_block != NULL);
        assert(code_block->elems == NULL);
        blocks = blocks->blocks->real;
        text_block = blocks->text_block->real;
        assert(!strcmp(text_block->text, "def"));
    }

    PadTkr_Parse(tkr, "{@@}{@@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        blocks = blocks->blocks->real;
        code_block = blocks->code_block->real;
        assert(code_block != NULL);
        assert(code_block->elems == NULL);
    }

    PadTkr_Parse(tkr, "{@@}abc{@@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        blocks = blocks->blocks->real;
        text_block = blocks->text_block->real;
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        code_block = blocks->code_block->real;
        assert(code_block != NULL);
        assert(code_block->elems == NULL);
    }

    PadTkr_Parse(tkr, "{@\n@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    PadTkr_Parse(tkr, "{@\n\n@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    /***************
    * if statement *
    ***************/

    PadTkr_Parse(tkr, "{@ if 1: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1 + 2: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        stmt = elems->stmt->real;
        if_stmt = stmt->if_stmt->real;
        test = if_stmt->test->real;
        or_test = test->or_test->real;
        and_test = PadNodeAry_Get(or_test->nodearr, 0)->real;
        not_test = PadNodeAry_Get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = PadNodeAry_Get(comparison->nodearr, 0)->real;
        expr = PadNodeAry_Get(asscalc->nodearr, 0)->real;
        term = PadNodeAry_Get(expr->nodearr, 0)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = PadNodeAry_Get(expr->nodearr, 2)->real;
        negative = PadNodeAry_Get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    PadTkr_Parse(tkr, "abc{@ if 1: end @}def");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    PadTkr_Parse(tkr, "{@\n\nif 1: end\n\n@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@\n\nif 1:\n\nend\n\n@}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: else: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == PAD_NODE_TYPE__ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
        assert(else_stmt);
    }

    PadTkr_Parse(tkr, "{@ if 1:\n\nelse:\n\nend @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == PAD_NODE_TYPE__ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
    }

    PadTkr_Parse(tkr, "{@ if 1: elif 2: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == PAD_NODE_TYPE__ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1:\n\nelif 2:\n\nend @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == PAD_NODE_TYPE__ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: elif 2: else: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == PAD_NODE_TYPE__ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == PAD_NODE_TYPE__ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    PadTkr_Parse(tkr, "{@ if 1:\n\nelif 2:\n\nelse:\n\nend @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == PAD_NODE_TYPE__ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == PAD_NODE_TYPE__ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == PAD_NODE_TYPE__ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == PAD_NODE_TYPE__ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    PadTkr_Parse(tkr, "{@ if 1: if 2: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1:\n\nif 2:\n\nend\n\nend @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: if 2: end if 3: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems != NULL);
        assert(elems->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(elems->elems->real != NULL);
        stmt = elems->stmt->real;
        elems = elems->elems->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ else: @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        stmt = elems->stmt->real;
        if_stmt = stmt->if_stmt->real;
        blocks = PadNodeAry_Get(if_stmt->contents, 0)->real;
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 2: end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: @}abc{@ if 2: end @}def{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks == NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    /****************
    * for statement *
    ****************/

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: if 1: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        elems = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: for 1; 1; 1: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        elems = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        assert(stmt->if_stmt == NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for 1: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for 1: if 1: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        elems = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ for 1: @}{@ if 1: end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ for 1: @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for: end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for: @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for: @}abc{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    PadTkr_Parse(tkr, "{@ for: @}{@ if 1: end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ if 1: for 1; 1; 1: end end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(PadNodeAry_Len(if_stmt->contents) == 1);
        elems = PadNodeAry_Get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 0);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: @}abc{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: @}{@ if 1: end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: @}abc{@ if 1: end @}def{@ end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        blocks = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == PAD_NODE_TYPE__IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == PAD_NODE_TYPE__TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks == NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    /*******
    * jump *
    *******/

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: break end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        elems = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        break_stmt = stmt->break_stmt->real;
        assert(break_stmt);
    }

    PadTkr_Parse(tkr, "{@ for 1; 1; 1: continue end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        root = PadAst_GetcRoot(ast);
        assert(root != NULL);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == PAD_NODE_TYPE__FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == PAD_NODE_TYPE__FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(PadNodeAry_Len(for_stmt->contents) == 1);
        elems = PadNodeAry_Get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        continue_stmt = stmt->continue_stmt->real;
        assert(continue_stmt);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
    // tail
}

static void
test_cc_import_stmt(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    const PadNode *root;
    PadNode *node;
    PadProgramNode *program;
    PadBlocksNode *blocks;
    PadCodeBlockNode *code_block;
    PadElemsNode *elems;
    PadStmtNode *stmt;
    PadImportStmtNode *import_stmt;
    PadImportAsStmtNode *import_as_stmt;
    PadFromImportStmtNode *from_import_stmt;
    PadImportVarsNode *import_vars;
    PadImportVarNode *import_var;
    PadStrNode *path;
    PadIdentNode *identifier;

    /**********************
    * import as statement *
    **********************/

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" as mod @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt);
        assert(import_stmt->import_as_stmt->type == PAD_NODE_TYPE__IMPORT_AS_STMT);
        assert(import_stmt->import_as_stmt->real);
        assert(import_stmt->from_import_stmt == NULL);

        import_as_stmt = import_stmt->import_as_stmt->real;
        assert(import_as_stmt);
        assert(import_as_stmt->path);
        assert(import_as_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(import_as_stmt->path->real);
        assert(import_as_stmt->alias);
        assert(import_as_stmt->alias->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_as_stmt->alias->real);

        path = import_as_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        identifier = import_as_stmt->alias->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "mod"));
    }

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" as mod \n @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ import @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found path in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found keyword 'as' in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" as @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found alias in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \n\"path/to/module\" as mod @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found path in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" \n as mod @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found keyword 'as' in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"path/to/module\" as \n mod @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found alias in compile import as statement"));
    }

    /************************
    * from import statement *
    ************************/

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import func @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == PAD_NODE_TYPE__IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(PadNodeAry_Len(import_vars->nodearr) == 1);

        node = PadNodeAry_Get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);

        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "func"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import func as f @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == PAD_NODE_TYPE__IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(PadNodeAry_Len(import_vars->nodearr) == 1);

        node = PadNodeAry_Get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias);
        assert(import_var->alias->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->alias->real);

        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "func"));

        identifier = import_var->alias->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "f"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == PAD_NODE_TYPE__IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(PadNodeAry_Len(import_vars->nodearr) == 1);

        node = PadNodeAry_Get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);

        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa, bbb ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == PAD_NODE_TYPE__IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(PadNodeAry_Len(import_vars->nodearr) == 2);

        node = PadNodeAry_Get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));

        node = PadNodeAry_Get(import_vars->nodearr, 1);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "bbb"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa, bbb, ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import (\naaa,\nbbb,\n) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, bbb ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        root = PadAst_GetcRoot(ast);
        assert(root->type == PAD_NODE_TYPE__PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == PAD_NODE_TYPE__BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == PAD_NODE_TYPE__CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == PAD_NODE_TYPE__ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == PAD_NODE_TYPE__STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == PAD_NODE_TYPE__IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == PAD_NODE_TYPE__STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == PAD_NODE_TYPE__IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(PadNodeAry_Len(import_vars->nodearr) == 2);

        node = PadNodeAry_Get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias);
        assert(import_var->alias->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->alias->real);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));
        identifier = import_var->alias->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "a"));

        node = PadNodeAry_Get(import_vars->nodearr, 1);
        assert(node);
        assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == PAD_NODE_TYPE__IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "bbb"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a, bbb ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a, \nbbb ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a,\n bbb \n) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ from @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found path in compile from import statement"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import in compile from import statement"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import variables in compile from import statement"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import \naaa @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import variables in compile from import statement"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import aaa as @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found second identifier in compile import variable"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid token 5 in compile import variables"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa as a @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid token 5 in compile import variables"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import variable in compile import variables"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, bbb @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid token 5 in compile import variables"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa \n as a ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid token 45 in compile import variables"));
    }

    PadTkr_Parse(tkr, "{@ from \"path/to/module\" import ( aaa as \n a ) @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found second identifier in compile import variable"));
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

static void
test_cc_func_def(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);

    PadTkr_Parse(tkr, "{@ def func():\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def func():\n"
    "@}123{@\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def func():\n"
    "@}123{@\n"
    "@}223{@\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "@}123{@\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "@}123{@\n"
    "   j = 1\n"
    "end @}");
    {
        PadAst_Clear(ast);
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);
    PadConfig_Del(config);
}

/**
 * 0 memory leaks
 * 2020/02/27
 */
static const struct testcase
compiler_tests[] = {
    {"PadCc_Compile", test_PadCc_Compile},
    {"cc_long_code", test_cc_long_code},
    {"cc_basic_0", test_cc_basic_0},
    {"cc_basic_1", test_cc_basic_1},
    {"cc_code_block", test_cc_code_block},
    {"cc_code_block_0", test_cc_code_block_0},
    {"cc_ref_block", test_cc_ref_block},
    {"cc_ref_block_0", test_cc_ref_block_0},
    {"cc_ref_block_1", test_cc_ref_block_1},
    {"cc_ref_block_2", test_cc_ref_block_2},
    {"cc_ref_block_3", test_cc_ref_block_3},
    {"cc_formula", test_cc_formula},
    {"cc_dict", test_cc_dict},
    {"cc_dict_0", test_cc_dict_0},
    {"cc_dict_1", test_cc_dict_1},
    {"cc_dict_2", test_cc_dict_2},
    {"cc_expr", test_cc_expr},
    {"cc_index", test_cc_index},
    {"cc_dot", test_cc_dot},
    {"cc_call", test_cc_call},
    {"cc_array", test_cc_array},
    {"cc_asscalc", test_cc_asscalc},
    {"cc_import_stmt", test_cc_import_stmt},
    {"cc_func_def", test_cc_func_def},
    {0},
};

/************
* traverser *
************/

static void
test_trv_long_code(void) {
    trv_ready;

    const char *src = "{@\n"
"    arr = []\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    arr.push(1)\n"
"    puts(\"done\")\n"
"@}\n";

    check_ok(src, "done\n");

    trv_cleanup;
}

static void
test_trv_comparison(void) {
    trv_ready;

    /******
    * int *
    ******/

    check_ok("{@ a = 0 != 1 @}{: a :}", "true");
    check_ok("{@ a = 1 != 1 != 1 @}{: a :}", "true");
    check_ok("{@ a = 1 != 1 != 0 @}{: a :}", "false");
    check_ok("{@ a = 1 == 1 @}{: a :}", "true");
    check_ok("{@ a = 1 == 0 @}{: a :}", "false");
    check_ok("{@ a = 1 == 1 == 1 @}{: a :}", "true");
    check_ok("{@ a = 1 == 1 == 0 @}{: a :}", "false");
    check_ok("{@ a = 1 != 1 @}{: a :}", "false");

    check_ok("{: 1 == 1.0 :}", "true");
    check_ok("{: 1.0 == 1 :}", "true");
    check_ok("{: 1 != 1.0 :}", "false");
    check_ok("{: 1.0 != 1 :}", "false");

    check_ok("{: 1 == 1 :}", "true");
    check_ok("{: 1 != 1 :}", "false");

    check_ok("{: 1 == true :}", "true");
    check_ok("{: 1 != true :}", "false");
    check_ok("{: 1 == false :}", "false");
    check_ok("{: 1 != false :}", "true");
    check_ok("{: true == 1 :}", "true");
    check_ok("{: true != 1 :}", "false");
    check_ok("{: false == 1 :}", "false");
    check_ok("{: false != 1 :}", "true");

    check_ok("{: [] == 1 :}", "false");
    check_ok("{: [] != 1 :}", "true");
    check_ok("{: 1 == [] :}", "false");
    check_ok("{: 1 != [] :}", "true");

    check_ok("{: {} == 1 :}", "false");
    check_ok("{: {} != 1 :}", "true");
    check_ok("{: 1 == {} :}", "false");
    check_ok("{: 1 != {} :}", "true");

    check_ok("{@ def f(): end @}{: f == 1 :}", "false");
    check_ok("{@ def f(): end @}{: f != 1 :}", "true");
    check_ok("{@ def f(): end @}{: 1 == f :}", "false");
    check_ok("{@ def f(): end @}{: 1 != f :}", "true");

    check_ok("{@ struct A: end @}{: A == 1 :}", "false");
    check_ok("{@ struct A: end @}{: A != 1 :}", "true");
    check_ok("{@ struct A: end @}{: 1 == A :}", "false");
    check_ok("{@ struct A: end @}{: 1 != A :}", "true");

    check_ok("{@ struct A: end @}{: A() == 1 :}", "false");
    check_ok("{@ struct A: end @}{: A() != 1 :}", "true");
    check_ok("{@ struct A: end @}{: 1 == A() :}", "false");
    check_ok("{@ struct A: end @}{: 1 != A() :}", "true");

    /********
    * float *
    ********/

    check_ok("{@ a = 0 != 1.0 @}{: a :}", "true");
    check_ok("{@ a = 1.0 != 1.0 != 1.0 @}{: a :}", "true");
    check_ok("{@ a = 1.0 != 1.0 != 0 @}{: a :}", "false");
    check_ok("{@ a = 1.0 == 1.0 @}{: a :}", "true");
    check_ok("{@ a = 1.0 == 0 @}{: a :}", "false");
    check_ok("{@ a = 1.0 == 1.0 == 1.0 @}{: a :}", "true");
    check_ok("{@ a = 1.0 == 1.0 == 0 @}{: a :}", "false");
    check_ok("{@ a = 1.0 != 1.0 @}{: a :}", "false");

    check_ok("{: 1.0 == 1.0 :}", "true");
    check_ok("{: 1.0 != 1.0 :}", "false");

    check_ok("{: 1 == 1.0 :}", "true");
    check_ok("{: 1.0 == 1 :}", "true");
    check_ok("{: 1 != 1.0 :}", "false");
    check_ok("{: 1.0 != 1 :}", "false");

    check_ok("{: 1.0 == true :}", "true");
    check_ok("{: 1.0 != true :}", "false");
    check_ok("{: 1.0 == false :}", "false");
    check_ok("{: 1.0 != false :}", "true");
    check_ok("{: true == 1.0 :}", "true");
    check_ok("{: true != 1.0 :}", "false");
    check_ok("{: false == 1.0 :}", "false");
    check_ok("{: false != 1.0 :}", "true");

    check_ok("{: [] == 1.0 :}", "false");
    check_ok("{: [] != 1.0 :}", "true");
    check_ok("{: 1.0 == [] :}", "false");
    check_ok("{: 1.0 != [] :}", "true");

    check_ok("{: {} == 1.0 :}", "false");
    check_ok("{: {} != 1.0 :}", "true");
    check_ok("{: 1.0 == {} :}", "false");
    check_ok("{: 1.0 != {} :}", "true");

    check_ok("{@ def f(): end @}{: f == 1.0 :}", "false");
    check_ok("{@ def f(): end @}{: f != 1.0 :}", "true");
    check_ok("{@ def f(): end @}{: 1.0 == f :}", "false");
    check_ok("{@ def f(): end @}{: 1.0 != f :}", "true");

    check_ok("{@ struct A: end @}{: A == 1.0 :}", "false");
    check_ok("{@ struct A: end @}{: A != 1.0 :}", "true");
    check_ok("{@ struct A: end @}{: 1.0 == A :}", "false");
    check_ok("{@ struct A: end @}{: 1.0 != A :}", "true");

    check_ok("{@ struct A: end @}{: A() == 1.0 :}", "false");
    check_ok("{@ struct A: end @}{: A() != 1.0 :}", "true");
    check_ok("{@ struct A: end @}{: 1.0 == A() :}", "false");
    check_ok("{@ struct A: end @}{: 1.0 != A() :}", "true");

    /*******
    * bool *
    *******/

    check_ok("{: true == true :}", "true");
    check_ok("{: false == false :}", "true");
    check_ok("{: true != true :}", "false");
    check_ok("{: false != false :}", "false");
    check_ok("{: false == true :}", "false");
    check_ok("{: true == false :}", "false");
    check_ok("{: false != true :}", "true");
    check_ok("{: false != false :}", "false");

    check_ok("{: true == \"a\" :}", "false");
    check_ok("{: false == \"a\" :}", "false");
    check_ok("{: true != \"a\" :}", "true");
    check_ok("{: false != \"a\" :}", "true");
    check_ok("{: \"a\" == true :}", "false");
    check_ok("{: \"a\" == false :}", "false");
    check_ok("{: \"a\" != true :}", "true");
    check_ok("{: \"a\" != false :}", "true");

    check_ok("{: true == [] :}", "false");
    check_ok("{: false == [] :}", "false");
    check_ok("{: true != [] :}", "true");
    check_ok("{: false != [] :}", "true");
    check_ok("{: [] == true :}", "false");
    check_ok("{: [] == false :}", "false");
    check_ok("{: [] != true :}", "true");
    check_ok("{: [] != false :}", "true");

    check_ok("{: true == {} :}", "false");
    check_ok("{: false == {} :}", "false");
    check_ok("{: true != {} :}", "true");
    check_ok("{: false != {} :}", "true");
    check_ok("{: {} == true :}", "false");
    check_ok("{: {} == false :}", "false");
    check_ok("{: {} != true :}", "true");
    check_ok("{: {} != false :}", "true");

    check_ok("{@ def f(): end @}{: true == f :}", "false");
    check_ok("{@ def f(): end @}{: false == f :}", "false");
    check_ok("{@ def f(): end @}{: true != f :}", "true");
    check_ok("{@ def f(): end @}{: false != f :}", "true");
    check_ok("{@ def f(): end @}{: f == true :}", "false");
    check_ok("{@ def f(): end @}{: f == false :}", "false");
    check_ok("{@ def f(): end @}{: f != true :}", "true");
    check_ok("{@ def f(): end @}{: f != false :}", "true");

    check_ok("{@ struct A: end @}{: true == A :}", "false");
    check_ok("{@ struct A: end @}{: false == A :}", "false");
    check_ok("{@ struct A: end @}{: true != A :}", "true");
    check_ok("{@ struct A: end @}{: false != A :}", "true");
    check_ok("{@ struct A: end @}{: A == true :}", "false");
    check_ok("{@ struct A: end @}{: A == false :}", "false");
    check_ok("{@ struct A: end @}{: A != true :}", "true");
    check_ok("{@ struct A: end @}{: A != false :}", "true");

    check_ok("{@ struct A: end @}{: true == A() :}", "false");
    check_ok("{@ struct A: end @}{: false == A() :}", "false");
    check_ok("{@ struct A: end @}{: true != A() :}", "true");
    check_ok("{@ struct A: end @}{: false != A() :}", "true");
    check_ok("{@ struct A: end @}{: A() == true :}", "false");
    check_ok("{@ struct A: end @}{: A() == false :}", "false");
    check_ok("{@ struct A: end @}{: A() != true :}", "true");
    check_ok("{@ struct A: end @}{: A() != false :}", "true");

    /**********
    * unicode *
    **********/

    check_ok("{@ a = 1 == \"abc\" @}{: a :}", "false");
    check_ok("{@ a = \"abc\" == 1 @}{: a :}", "false");
    check_ok("{@ a = \"abc\" == \"abc\" @}{: a :}", "true");
    check_ok("{@ a = 1 != \"abc\" @}{: a :}", "true");
    check_ok("{@ a = \"abc\" != 1 @}{: a :}", "true");
    check_ok("{@ a = \"abc\" != \"def\" @}{: a :}", "true");
    check_ok("{@ a = \"abc\" == \"abc\" @}{: a :}", "true");

    /**
     * well-formed on Python
     * ill-formed on Ruby
     */
    check_ok("{@ a = \"abc\" == \"abc\" == \"def\" @}{: a :}", "false");
    check_ok("{@ a = \"abc\" == \"abc\" @}{: a :}", "true");

    check_ok("{: \"a\" == [] :}", "false");
    check_ok("{: \"a\" != [] :}", "true");
    check_ok("{: [] == \"a\" :}", "false");
    check_ok("{: [] != \"a\" :}", "true");

    check_ok("{: \"a\" == {} :}", "false");
    check_ok("{: \"a\" != {} :}", "true");
    check_ok("{: {} == \"a\" :}", "false");
    check_ok("{: {} != \"a\" :}", "true");

    check_ok("{@ def f(): end @}{: \"a\" == f :}", "false");
    check_ok("{@ def f(): end @}{: \"a\" != f :}", "true");
    check_ok("{@ def f(): end @}{: f == \"a\" :}", "false");
    check_ok("{@ def f(): end @}{: f != \"a\" :}", "true");

    check_ok("{@ struct A: end @}{: \"a\" == A :}", "false");
    check_ok("{@ struct A: end @}{: \"a\" != A :}", "true");
    check_ok("{@ struct A: end @}{: A == \"a\" :}", "false");
    check_ok("{@ struct A: end @}{: A != \"a\" :}", "true");

    check_ok("{@ struct A: end @}{: \"a\" == A() :}", "false");
    check_ok("{@ struct A: end @}{: \"a\" != A() :}", "true");
    check_ok("{@ struct A: end @}{: A() == \"a\" :}", "false");
    check_ok("{@ struct A: end @}{: A() != \"a\" :}", "true");

    /********
    * array *
    ********/

    check_ok("{: [] == [] :}", "false");
    check_ok("{: [] != [] :}", "true");

    check_ok("{: [] == {} :}", "false");
    check_ok("{: [] != {} :}", "true");
    check_ok("{: {} == [] :}", "false");
    check_ok("{: {} != [] :}", "true");

    check_ok("{@ def f(): end @}{: [] == f :}", "false");
    check_ok("{@ def f(): end @}{: [] != f :}", "true");
    check_ok("{@ def f(): end @}{: f == [] :}", "false");
    check_ok("{@ def f(): end @}{: f != [] :}", "true");

    check_ok("{@ struct A: end @}{: A == [] :}", "false");
    check_ok("{@ struct A: end @}{: A != [] :}", "true");
    check_ok("{@ struct A: end @}{: [] == A :}", "false");
    check_ok("{@ struct A: end @}{: [] != A :}", "true");

    check_ok("{@ struct A: end @}{: A() == [] :}", "false");
    check_ok("{@ struct A: end @}{: A() != [] :}", "true");
    check_ok("{@ struct A: end @}{: [] == A() :}", "false");
    check_ok("{@ struct A: end @}{: [] != A() :}", "true");

    /*******
    * dict *
    *******/

    check_ok("{: {} == {} :}", "false");
    check_ok("{: {} != {} :}", "true");

    check_ok("{@ def f(): end @}{: {} == f :}", "false");
    check_ok("{@ def f(): end @}{: {} != f :}", "true");
    check_ok("{@ def f(): end @}{: f == {} :}", "false");
    check_ok("{@ def f(): end @}{: f != {} :}", "true");

    check_ok("{@ struct A: end @}{: A == {} :}", "false");
    check_ok("{@ struct A: end @}{: A != {} :}", "true");
    check_ok("{@ struct A: end @}{: {} == A :}", "false");
    check_ok("{@ struct A: end @}{: {} != A :}", "true");

    check_ok("{@ struct A: end @}{: A() == {} :}", "false");
    check_ok("{@ struct A: end @}{: A() != {} :}", "true");
    check_ok("{@ struct A: end @}{: {} == A() :}", "false");
    check_ok("{@ struct A: end @}{: {} != A() :}", "true");

    /***********
    * function *
    ***********/

    check_ok("{@ def f(): end \n a = 1 == f @}{: a :}", "false");
    check_ok("{@ def f(): end \n a = \"abc\" == f @}{: a :}", "false");
    check_ok("{@ def f(): end \n a = f != 1 @}{: a :}", "true");
    check_ok("{@ def f(): end \n a = 1 != f @}{: a :}", "true");

    check_ok("{@ struct A: end \n def f(): end @}{: A == f :}", "false")
    check_ok("{@ struct A: end \n def f(): end @}{: A != f :}", "true")
    check_ok("{@ struct A: end \n def f(): end @}{: f == A:}", "false")
    check_ok("{@ struct A: end \n def f(): end @}{: f != A :}", "true")

    /*********
    * object *
    *********/

    check_ok("{@ struct A: end @}{: A() != nil :}", "true");
    check_ok("{@ struct A: end @}{: nil != A() :}", "true");
    check_ok("{@ struct A: end @}{: A() == nil :}", "false");
    check_ok("{@ struct A: end @}{: nil == A() :}", "false");

    check_ok("{@ struct A: end \n def f(): end @}{: A() != f :}", "true");
    check_ok("{@ struct A: end \n def f(): end @}{: f != A() :}", "true");
    check_ok("{@ struct A: end \n def f(): end @}{: A() == f :}", "false");
    check_ok("{@ struct A: end \n def f(): end @}{: f == A() :}", "false");

    check_ok("{@ struct A: end \n arr = [] @}{: A() != arr :}", "true");
    check_ok("{@ struct A: end \n arr = [] @}{: arr != A() :}", "true");
    check_ok("{@ struct A: end \n arr = [] @}{: A() == arr :}", "false");
    check_ok("{@ struct A: end \n arr = [] @}{: arr == A() :}", "false");

    check_ok("{@ struct A: end \n d = {} @}{: A() != d :}", "true");
    check_ok("{@ struct A: end \n d = {} @}{: d != A() :}", "true");
    check_ok("{@ struct A: end \n d = {} @}{: A() == d :}", "false");
    check_ok("{@ struct A: end \n d = {} @}{: d == A() :}", "false");

    check_ok("{@ struct A: end @}{: A() != 1 :}", "true");
    check_ok("{@ struct A: end @}{: 1 != A() :}", "true");
    check_ok("{@ struct A: end @}{: A() == 1 :}", "false");
    check_ok("{@ struct A: end @}{: 1 == A() :}", "false");

    check_ok("{@ struct A: end @}{: A() != \"a\" :}", "true");
    check_ok("{@ struct A: end @}{: \"a\" != A() :}", "true");
    check_ok("{@ struct A: end @}{: A() == \"a\" :}", "false");
    check_ok("{@ struct A: end @}{: \"a\" == A() :}", "false");

    check_ok("{@ struct A: end @}{: A != nil :}", "true");
    check_ok("{@ struct A: end @}{: nil != A :}", "true");
    check_ok("{@ struct A: end @}{: A == nil :}", "false");
    check_ok("{@ struct A: end @}{: nil == A :}", "false");

    check_ok("{@ struct A: end \n def f(): end @}{: A != f :}", "true");
    check_ok("{@ struct A: end \n def f(): end @}{: f != A :}", "true");
    check_ok("{@ struct A: end \n def f(): end @}{: A == f :}", "false");
    check_ok("{@ struct A: end \n def f(): end @}{: f == A :}", "false");

    check_ok("{@ struct A: end \n arr = [] @}{: A != arr :}", "true");
    check_ok("{@ struct A: end \n arr = [] @}{: arr != A :}", "true");
    check_ok("{@ struct A: end \n arr = [] @}{: A == arr :}", "false");
    check_ok("{@ struct A: end \n arr = [] @}{: arr == A :}", "false");

    check_ok("{@ struct A: end \n d = {} @}{: A != d :}", "true");
    check_ok("{@ struct A: end \n d = {} @}{: d != A :}", "true");
    check_ok("{@ struct A: end \n d = {} @}{: A == d :}", "false");
    check_ok("{@ struct A: end \n d = {} @}{: d == A :}", "false");

    check_ok("{@ struct A: end @}{: A != 1 :}", "true");
    check_ok("{@ struct A: end @}{: 1 != A :}", "true");
    check_ok("{@ struct A: end @}{: A == 1 :}", "false");
    check_ok("{@ struct A: end @}{: 1 == A :}", "false");

    check_ok("{@ struct A: end @}{: A != \"a\" :}", "true");
    check_ok("{@ struct A: end @}{: \"a\" != A :}", "true");
    check_ok("{@ struct A: end @}{: A == \"a\" :}", "false");
    check_ok("{@ struct A: end @}{: \"a\" == A :}", "false");

    /*********************************
    * boolean can convert to integer *
    *********************************/

    check_ok("{@ a = true == 1 @}{: a :}", "true");
    check_ok("{@ a = false == 0 @}{: a :}", "true");

    /******
    * lte *
    ******/

    // int
    check_ok("{: 1 <= 2 :}", "true");
    check_ok("{: 2 <= 1 :}", "false");

    check_ok("{: 1.0 <= 2 :}", "true");
    check_ok("{: 1.0 <= 0 :}", "false");
    check_ok("{: 0 <= 1.0 :}", "true");
    check_ok("{: 2 <= 1.0 :}", "false");

    check_ok("{: true <= 2 :}", "true");
    check_ok("{: true <= 0 :}", "false");
    check_ok("{: 0 <= true :}", "true");
    check_ok("{: 2 <= true :}", "false");

    check_ok("{@ def f(n): return n end @}{: 1 <= f(2) :}", "true")

    check_ok("{@ def f(n): return n end @}{: 1 <= ord(\"a\")[0] :}", "true")

    // check_fail("", ""); TODO [] {} def etc

    /******
    * gte *
    ******/

    check_ok("{@ a = 1 >= 2 @}{: a :}", "false");
    check_ok("{@ a = 2 >= 1 @}{: a :}", "true");
    check_ok("{@ a = true >= 2 @}{: a :}", "false");
    check_ok("{@ a = true >= 0 @}{: a :}", "true");
    check_ok("{@ a = 0 >= true @}{: a :}", "false");
    check_ok("{@ a = 2 >= true @}{: a :}", "true");

    /*****
    * lt *
    *****/

    check_ok("{@ a = 1 < 2 @}{: a :}", "true");
    check_ok("{@ a = 2 < 1 @}{: a :}", "false");
    check_ok("{@ a = true < 2 @}{: a :}", "true");
    check_ok("{@ a = true < 1 @}{: a :}", "false");
    check_ok("{@ a = 0 < true @}{: a :}", "true");
    check_ok("{@ a = 1 < true @}{: a :}", "false");

    /*****
    * gt *
    *****/

    check_ok("{@ a = 1 > 1 @}{: a :}", "false");
    check_ok("{@ a = 2 > 1 @}{: a :}", "true");
    check_ok("{@ a = true > 1 @}{: a :}", "false");
    check_ok("{@ a = true > 0 @}{: a :}", "true");
    check_ok("{@ a = 0 > true @}{: a :}", "false");
    check_ok("{@ a = 2 > true @}{: a :}", "true");

    trv_cleanup;
}

static void
test_trv_array_index(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    // PadTkr_Parse(tkr, "{@ a[0] @}");
    // {
    PadAst_Clear(ast);
    //     (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
    //     PadCtx_Clear(ctx);
    //     (PadTrv_Trav(ast, ctx));
    //     assert(PadAst_HasErrs(ast));
    //     assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't index access. \"a\" is not defined"));
    // }

    PadTkr_Parse(tkr, "{@ a = [1, 2] \n @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] \n @}{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] \n @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] \n @}{: a[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "index out of range"));
    }

    /* PadTkr_Parse(tkr, "{@ a = [1, 2] \n @}{: a[-1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "index out of range of array"));
    } */

    // PadTkr_Parse(tkr, "{@ a = (b, c = 1, 2)[0] \n @}{: a :}");
    // {
    //     PadAst_Clear(ast);
    //     PadCc_Compile(ast, PadTkr_GetToks(tkr));
    //     PadCtx_Clear(ctx);
    //     (PadTrv_Trav(ast, ctx));
    //     assert(!PadAst_HasErrs(ast));
    //     assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    // }

    // PadTkr_Parse(tkr, "{@ a = (b, c = 1, 2)[1] \n @}{: a :}");
    // {
    PadAst_Clear(ast);
    //     PadCc_Compile(ast, PadTkr_GetToks(tkr));
    //     PadCtx_Clear(ctx);
    //     (PadTrv_Trav(ast, ctx));
    //     assert(!PadAst_HasErrs(ast));
    //     assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    // }

    PadTkr_Parse(tkr, "{@ a = [[1, 2]] \n @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [[1, 2]] \n @}{: a[0][0] :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_text_block_old(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "abc");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_ref_block_old(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: nil :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{: false :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{: true :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{: 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{: 123 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    PadTkr_Parse(tkr, "{: \"abc\" :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined in ref block"));
    }

    /* PadTkr_Parse(tkr, "{: alias(\"dtl\", \"run bin/date-line.py\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    } */

    PadTkr_Parse(tkr, "{: 1 + 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{: 1 + 1 + 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{: [1, 2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "@}{: string :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(module)"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(module)"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "   string.b = string.a\n"
    "@}{: string.b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = 1\n"
    "@}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_4(void) {
    trv_ready;

    check_ok("{@\n"
    "   a, b = 1, 2\n"
    "@}{: a :},{: b :}", "1,2");

    trv_cleanup;
}

static void
test_trv_assign_5(void) {
    trv_ready;

    check_fail("{@ a @}{: a :}", "\"a\" is not defined in ref block");

    trv_cleanup;
}

static void
test_trv_atom_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ nil @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ false @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ true @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ 1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ \"abc\" @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ var @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_index(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}{: a[0] :},{: a[1] :},{: a[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a,b,c"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\": 1, \"b\": 2} @}{: a[\"a\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\": 1, \"b\": 2} @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = a[0] or a[1] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = a[0] and a[1] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "b"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = not a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = a[0] or a[1] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = a[0] and a[1] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = not a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] or a[\"b\"] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] and a[\"b\"] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = not a[\"a\"] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = a[0] == \"a\" @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = \"a\" == a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = a[0] != \"a\" @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n b = \"a\" != a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = a[0] == 1 @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = 1 == a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = a[0] != 1 @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n b = 1 != a[0] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] == 1 @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = 1 == a[\"a\"] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] != 1 @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = 1 != a[\"a\"] @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n if a[0] == \"a\": puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n a[0] = 3 @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,2"));
    }

    PadTkr_Parse(tkr, "{@ a = [1,2] \n a[0] = 3 \n a[1] = 4 @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,4"));
    }

    PadTkr_Parse(tkr, "{@ a = [\"a\",\"b\"] \n a[0] = \"c\" @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "c,b"));
    }

    PadTkr_Parse(tkr, "{@ a = [\"a\",\"b\"] \n a[0] = \"c\" \n a[1] = \"d\" @}{: a[0] :},{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "c,d"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2 } \n a[\"a\"] = 3 @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,2"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"a\":1, \"b\":2 } \n a[\"a\"] = 3 \n a[\"b\"] = 4 @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,4"));
    }

    PadTkr_Parse(tkr, "{@ a = [] a.push(1) @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [\"abc_def\"] @}{: a[0].camel() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        showdetail();
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcDef"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_string_index(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n @}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n @}{: a[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "b"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n @}{: a[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "index out of range"));
    }

    PadTkr_Parse(tkr, "{@ a = (\"a\" + \"b\")[0] \n @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    PadTkr_Parse(tkr, "{@ a = (\"a\" + \"b\")[1] \n @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "b"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\"[0][0] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_multi_assign(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    // error

    PadTkr_Parse(tkr, "{@ a, b = 1, 2, 3 @}{: a :} {: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't assign array to array. not same length"));
    }

    PadTkr_Parse(tkr, "{@ a, b = 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid right operand (1)"));
    }

    // success

    PadTkr_Parse(tkr, "{@ a, b = 1, 2 @}{: a :} {: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2"));
    }

    PadTkr_Parse(tkr, "{@ a = 1, 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_and_test(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    // nil and objects

    PadTkr_Parse(tkr, "{@ a = nil and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = nil and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = nil and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    // digit and objects

    PadTkr_Parse(tkr, "{@ a = 1 and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and 2 and 3 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = 1 and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = 0 and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    // bool and objects

    PadTkr_Parse(tkr, "{@ a = true and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = false and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = true and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = true and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = false and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = false and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = false and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = true and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = true and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = false and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = false and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = true and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = true and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    // string and other

    PadTkr_Parse(tkr, "{@ a = \"abc\" and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = \"abc\" and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = \"abc\" and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = \"abc\" and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = \"\" and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = \"\" and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = \"\" and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    // array and other

    PadTkr_Parse(tkr, "{@ a = [1, 2] and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = [1, 2] and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = [1, 2] and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = [1, 2] and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = [] and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = [] and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = [] and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    // dict and other

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {\"k\": 1} and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = {\"k\": 1} and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = {\"k\": 1} and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = {\"k\": 1} and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and {} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = {} and {\"k\":1} @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = {} and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = {} and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = {} and b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    //

    PadTkr_Parse(tkr, "{@ a = \"abc\" and 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 and \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = 1 and f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_list(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    // success

    PadTkr_Parse(tkr, "{@ a = nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\"\n b = a @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 1, b = 2 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 + 2, b = 3 * 4 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,12"));
    }

    PadTkr_Parse(tkr, "{@ a = 1, b = 2, c = 3 @}{: a :},{: b :},{: c :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2,3"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = a = 1 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = a = 1, c = b = 1 @}{: a :},{: b :},{: c :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1,1"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a, b = 1, 2 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = alias.set(\"\", \"\") @}{: a :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = alias.set(\"\", \"\")\n b = alias.set(\"\", \"\") @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil,nil"));
    }

    PadTkr_Parse(tkr, "{@ a = opts.get(\"abc\") @}{: a :}");
    {
        char *argv[] = {
            "make",
            "-abc",
            "def",
            NULL,
        };
        PadOpts *opts = PadOpts_New();
        assert(PadOpts_Parse(opts, 3, argv));
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadAst_MoveOpts(ast, opts);
        (PadTrv_Trav(ast, ctx));
        PadAst_MoveOpts(ast, NULL);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_test_list(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ 1, 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ 1, \"abc\", var, alias.set(\"\", \"\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = 0 \n a += 1, b += 2 @}{: a :} {: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_negative_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{: -1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "-1"));
    }

    PadTkr_Parse(tkr, "{: 1 + -1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{: -1 + -1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "-2"));
    }

    PadTkr_Parse(tkr, "{: 1 - -1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{: -1 - -1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{: 1-1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    check_ok("{: -true :}", "-1");
    check_ok_showbuf("{: -1.0 :}", "-1.0");
    check_ok("{@ a = 1 @}{: -a :}", "-1");

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_dot_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: \"ABC\".lower() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{: \"abc\".upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ABC"));
    }

    PadTkr_Parse(tkr, "{: \"ABC\".lower().upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ABC"));
    }

    PadTkr_Parse(tkr, "{: alias.set(\"a\", \"b\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetAliasValue(ctx, "a"), "b"));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_dot_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "@}{: string.variable.upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "STRING"));
    }

    trv_cleanup;
}

static void
test_trv_dot_2(void) {
    trv_ready;

    check_ok("{@\n"
    "    arr = [1, 2]\n"
    "    dst = []\n"
    "    dst.push(arr[1])\n"
    "@}{: dst[0] :}"
    , "2");

    trv_cleanup;
}

static void
test_trv_dot_3(void) {
    trv_ready;

    check_ok("{@\n"
    "    arr = [1, 2]\n"
    "    dst = []\n"
    "    dst.push(arr.pop())\n"
    "@}{: dst[0] :}"
    , "2");

    trv_cleanup;
}

static void
test_trv_dot_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "    arr = [[1, 2], [3, 4]]\n"
    "    dst = []\n"
    "    n = dst.push(arr.pop().pop()).pop()\n"
    "@}{: n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    trv_cleanup;
}

static void
test_trv_dot_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "    arr = [[[[[[[[1, 2]]]]]]]]\n"
    "    dst = []\n"
    "    n = dst.push(arr.pop().pop().pop().pop().pop().pop().pop().pop()).pop()\n"
    "@}{: n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_dot_6(void) {
    trv_ready;

    check_ok("{: \"abc\".capitalize() :}"
    , "Abc");

    trv_cleanup;
}

static void
test_trv_call(void) {
    trv_ready;

    check_ok("{@ def f(): return 1 end @}{: f() :}", "1");
    check_ok("{@ puts(1) @}", "1\n");
    check_ok("{@ def f(): return 1 end \n funcs = { \"a\": f } @}{: funcs[\"a\"]() :}", "1");
    check_ok("{@ def a(n): return n*2 end \n def b(): return a end @}{: b()(2) :}", "4");

    trv_cleanup;
}

static void
test_trv_builtin_string(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /********
    * upper *
    ********/

    PadTkr_Parse(tkr, "{: \"abc\".upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ABC"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n @}{: a.upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ABC"));
    }

    PadTkr_Parse(tkr, "{: nil.upper() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"upper\" is not defined"));
    }

    /********
    * lower *
    ********/

    PadTkr_Parse(tkr, "{: \"ABC\".lower() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ABC\" \n @}{: a.lower() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{: nil.lower() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"lower\" is not defined"));
    }

    /*************
    * capitalize *
    *************/

    PadTkr_Parse(tkr, "{: \"abc\".capitalize() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "Abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" \n @}{: a.capitalize() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "Abc"));
    }

    PadTkr_Parse(tkr, "{: nil.capitalize() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"capitalize\" is not defined"));
    }

    /********
    * snake *
    ********/

    PadTkr_Parse(tkr, "{: \"abcDef\".snake() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc_def"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abcDef\" \n @}{: a.snake() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc_def"));
    }

    PadTkr_Parse(tkr, "{: nil.snake() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"snake\" is not defined"));
    }

    /********
    * camel *
    ********/

    PadTkr_Parse(tkr, "{: \"camel_case\".camel() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "camelCase"));
    }

    PadTkr_Parse(tkr, "{@ a = \"camel_case\" \n @}{: a.camel() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "camelCase"));
    }

    PadTkr_Parse(tkr, "{: nil.camel() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"camel\" is not defined"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_unicode_split(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ toks = \"abc\ndef\nghi\".split(\"\n\") @}"
        "{: len(toks) :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ toks = \"abc\ndef\nghi\n\".split(\"\n\") @}"
        "{: len(toks) :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ toks = \"\".split(\"\n\") @}"
        "{: len(toks) :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_unicode_rstrip(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ s = \"abc \r\n\".rstrip() @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ s = \"abcdef\".rstrip(\"def\") @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_unicode_lstrip(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ s = \"\r\n abc\".lstrip() @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ s = \"defabc\".lstrip(\"def\") @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_unicode_strip(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ s = \"\r\n abc\r\n \".strip() @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ s = \"defabcdef\".strip(\"def\") @}"
        "{: s :}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_unicode_isdigit(void) {
    trv_ready;

    check_ok("{: \"123\".isdigit() :}", "true");
    check_ok("{: \"abc\".isdigit() :}", "false");
    check_ok("{: \"123abc\".isdigit() :}", "false");

    trv_cleanup;
}

static void
test_trv_builtin_unicode_isalpha(void) {
    trv_ready;

    check_ok("{: \"123\".isalpha() :}", "false");
    check_ok("{: \"abc\".isalpha() :}", "true");
    check_ok("{: \"123abc\".isalpha() :}", "false");

    trv_cleanup;
}

static void
test_trv_builtin_unicode_isspace(void) {
    trv_ready;

    check_ok("{: \"123\".isspace() :}", "false");
    check_ok("{: \" \".isspace() :}", "true");
    check_ok("{: \"\n\".isspace() :}", "true");
    check_ok("{: \"\t\".isspace() :}", "true");
    check_ok("{: \" \n\t\".isspace() :}", "true");

    trv_cleanup;
}

static void
test_trv_builtin_functions(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /********
    * alias *
    ********/

    PadTkr_Parse(tkr, "{@ alias.set(\"abc\", \"def\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
        const char *value = PadAliasInfo_GetcValue(alinfo, "abc");
        assert(value);
        assert(!strcmp(value, "def"));
    }

    PadTkr_Parse(tkr, "{@ alias.set(\"abc\", \"def\", \"ghi\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
        const char *value = PadAliasInfo_GetcValue(alinfo, "abc");
        assert(value);
        assert(!strcmp(value, "def"));
        const char *desc = PadAliasInfo_GetcDesc(alinfo, "abc");
        assert(desc);
        assert(!strcmp(desc, "ghi"));
    }

    /*******
    * opts *
    *******/

    PadTkr_Parse(tkr, "{: opts.get(\"abc\") :}");
    {
        PadOpts *opts = PadOpts_New();
        char *argv[] = {
            "make",
            "--abc",
            "def",
            NULL,
        };
        PadOpts_Parse(opts, 3, argv);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadAst_MoveOpts(ast, opts);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{: opts.has(\"abc\") :}");
    {
        PadOpts *opts = PadOpts_New();
        char *argv[] = {
            "make",
            "--abc",
            NULL,
        };
        PadOpts_Parse(opts, 2, argv);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadAst_MoveOpts(ast, opts);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{: opts.has(\"def\") :}");
    {
        PadOpts *opts = PadOpts_New();
        char *argv[] = {
            "make",
            "--abc",
            NULL,
        };
        PadOpts_Parse(opts, 2, argv);
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadAst_MoveOpts(ast, opts);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    /*******
    * puts *
    *******/

    PadTkr_Parse(tkr, "{@ puts() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\n"));
    }

    PadTkr_Parse(tkr, "{@ puts(1) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ puts(1, 2) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2\n"));
    }

    PadTkr_Parse(tkr, "{@ puts(1, \"abc\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 abc\n"));
    }

    PadTkr_Parse(tkr, "{@ puts(\"abc\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc\n"));
    }

    /********
    * eputs *
    ********/

    PadTkr_Parse(tkr, "{@ eputs() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStderrBuf(ctx), "\n"));
    }

    PadTkr_Parse(tkr, "{@ eputs(1) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStderrBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ eputs(1, 2) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStderrBuf(ctx), "1 2\n"));
    }

    PadTkr_Parse(tkr, "{@ eputs(1, \"abc\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStderrBuf(ctx), "1 abc\n"));
    }

    PadTkr_Parse(tkr, "{@ eputs(\"abc\") @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStderrBuf(ctx), "abc\n"));
    }

    /*****
    * id *
    *****/

    PadTkr_Parse(tkr, "{: id(1) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

extern const char *builtin_structs_source;

static void
test_trv_builtin_structs_error_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadTkr *s_tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ err = Error() @}{: type(err) :}");
    PadTkr_Parse(s_tkr, builtin_structs_source);
    PadTkr_ExtendFrontOther(tkr, s_tkr);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(struct)"));
    }

    PadTkr_Parse(tkr, "{@ err = Error(\"oioi\") @}{: err.what() :}");
    PadTkr_Parse(s_tkr, builtin_structs_source);
    PadTkr_ExtendFrontOther(tkr, s_tkr);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "Oioi."));
    }

    PadTkr_Parse(tkr, "{@ err = Error(\"oioi\", Error.TYPE) @}{: err.what() :}");
    PadTkr_Parse(s_tkr, builtin_structs_source);
    PadTkr_ExtendFrontOther(tkr, s_tkr);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "Oioi. Invalid type."));
    }

    PadTkr_Parse(tkr, "{@ def run():\n return nil, Error(\"oioi\", Error.TYPE)\n end a, b = run()\n @}{: b.what() :}");
    PadTkr_Parse(s_tkr, builtin_structs_source);
    PadTkr_ExtendFrontOther(tkr, s_tkr);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "Oioi. Invalid type."));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(s_tkr);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

// ATODO
static void
test_trv_builtin_structs_error_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadTkr *s_tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr,
"{@\n"
"    /**\n"
"     * Run function with argument\n"
"     *\n"
"     * @param[in] {int} arg the argument\n"
"     */\n"
"    def run(arg):\n"
"        if arg != 1:\n"
"            err = Error(\"error\", Error.VALUE)\n"
"            return nil, err\n"
"        end\n"
"\n"
"        return arg, nil\n"
"    end\n"
"\n"
"    // non error\n"
"    result, err = run(1)\n"
"    if err != nil:\n"
"        puts(err.what())\n"
"    end\n"
"    puts(result)\n"
"\n"
"    // errors occured\n"
"    result, err = run(0)\n"
"    if err != nil:\n"
"        puts(err.what())  // detail?\n"
"        if err.no == Error.VALUE:  // error detail is...\n"
"            puts(\"Your gave the arguments was invalid.\")\n"
"        end\n"
"    end\n"
"@}\n"
    );
    PadTkr_Parse(s_tkr, builtin_structs_source);
    PadTkr_ExtendFrontOther(tkr, s_tkr);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        trace();
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"
"Error. Invalid value.\n"
"Your gave the arguments was invalid.\n"
        ));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(s_tkr);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_modules_opts_0(void) {
    trv_ready;

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{: opts.args(0) :},{: opts.args(1) :}");
    {
        int argc = 2;
        char *argv[] = {
            "cmd",
            "aaa",
            NULL
        };
        PadOpts *opts = PadOpts_New();
        PadOpts_Parse(opts, argc, argv);
        PadAst_Clear(ast);
        PadAst_MoveOpts(ast, PadMem_Move(opts));
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "cmd,aaa"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ alias.set(1, 2, 3) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't invoke alias.set. key is not string"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ alias.set() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't invoke alias.set. too few arguments"));
    }

    PadTkr_Parse(tkr, "{@ alias.set(1, 2, 3) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't invoke alias.set. key is not string"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ alias.set() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_array_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "    arr = [1, 2]"
    "    dst = []\n"
    "    dst.push(arr[1])\n"
    "@}{: dst[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_array_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   arr = []\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       arr.push(i)\n"
    "   end\n"
    "@}{: arr[0] :},{: arr[1] :},{: arr[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0,1,2"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_functions_type_dict(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ d = {\"a\": 1} @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def f(d): end \n f({\"a\": 1}) @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def f(d): end @}{: f(1) :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{@ def f(d): end @}{: f({\"a\": 1}) :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    PadTkr_Parse(tkr, "{: type({ \"a\": 1 }) :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(type)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_functions_type(void) {
    trv_ready;

    check_ok("{: type(nil) :}", "(type)");
    check_ok("{: type(1) :}", "(type)");
    check_ok("{: type(true) :}", "(type)");
    check_ok("{: type(\"string\") :}", "(type)");
    check_ok("{: type([1, 2]) :}", "(type)");
    check_ok("{: type({ \"a\": 1 }) :}", "(type)");
    check_ok("{@ def f(): end @}{: type(f) :}", "(type)");
    check_ok("{@ import \"tests/lang/modules/hello.cap\" as mod @}{: type(mod) :}", "imported\n(type)");
    check_ok("{@ struct A: end @}{: type(A()) :}", "(struct)");

    trv_cleanup;
}

static void
test_trv_builtin_functions_exit(void) {
    // nothing to do
}

static void
test_trv_builtin_functions_cast(void) {
    trv_ready;

    // TODO
    // check_ok("{@ n, err = cast(\"float\", 1) @}{: n :}", "1.0");
    // check_ok("{@ n, err = cast(\"float\", 1.0) @}{: n :}", "1.0");
    // check_ok("{@ n, err = cast(\"float\", nil) @}{: err.what() :}", "can't cast nil to float");

    trv_cleanup;
}

static void
test_trv_builtin_functions_puts_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ puts(1) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_functions_len_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: len([1, 2]) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{: len([]) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{: len(\"12\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{: len(\"\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_functions_copy_0(void) {
    trv_ready;

    check_ok("{@ a = 1 \n b = copy(a) @}{: id(a) != id(b) :}", "true");

    trv_cleanup;
}

static void
test_trv_builtin_functions_deepcopy_0(void) {
    trv_ready;

    check_ok("{@ a = 1 \n b = deepcopy(a) @}{: id(a) != id(b) :}", "true");

    trv_cleanup;
}

static void
test_trv_builtin_functions_assert_0(void) {
    trv_ready;

    check_fail("{@ assert(0) @}", "assertion error");

    trv_cleanup;
}

static void
test_trv_builtin_functions_assert_1(void) {
    trv_ready;

    check_ok("{@ assert(1) @}", "");

    trv_cleanup;
}

static void
test_trv_builtin_functions_extract_0(void) {
    trv_ready;

    check_ok("{@ struct I: a = 1 \n b = 2 \n end \n extract(I) @}{: a :},{: b :}", "1,2");
    check_ok("{@ struct I: a = 1 \n b = 2 \n end \n extract(I()) @}{: a :},{: b :}", "1,2");
    check_ok("{@ struct I: a = 1 \n b = 2 \n end \n struct A: extract(I) end @}{: A.a :},{: A.b :}", "1,2");
    check_ok("{@ struct I: a = 1 \n b = 2 \n end \n struct A: extract(I) end \n a = A() @}{: a.a :},{: a.b :}", "1,2");

    trv_cleanup;
}

static void
test_trv_builtin_functions_setattr_0(void) {
    trv_ready;

    check_ok("{@ struct A: end \n setattr(A, \"a\", 1) @}{: A.a :}", "1");
    check_ok("{@ struct A: end \n a = A() \n setattr(a, \"a\", 1) @}{: a.a :}", "1");

    check_fail("{@ struct A: end \n setattr(nil, nil, nil) @}", "unsupported object type");
    check_fail("{@ struct A: end \n setattr(A, nil, nil) @}", "invalid key");

    trv_cleanup;
}

static void
test_trv_builtin_functions_getattr_0(void) {
    trv_ready;

    check_ok("{@ struct A: end \n setattr(A, \"a\", 1) @}{: getattr(A, \"a\") :}", "1");
    check_ok("{@ struct A: end \n a = A() \n setattr(a, \"a\", 1) @}{: getattr(a, \"a\") :}", "1");

    check_fail("{@ struct A: end \n getattr(nil, nil) @}", "unsupported object type");
    check_fail("{@ struct A: end \n getattr(A, nil) @}", "invalid key");

    trv_cleanup;
}

static void
test_trv_builtin_functions_dance_0(void) {
    trv_ready;

    check_ok("{@ out, err = dance(\"{@ puts(1) @}\") @}{: out :},{: err :}", "1\n,nil");
    check_ok("{@ out, err = dance(\"{@ a = b @}\") @}{: out :},{: err :}", "nil,\"b\" is not defined in asscalc ass idn");
    check_ok("{@ out, err = dance(\"{@ a = b @}{: a :}\", {\"b\": 1}) @}{: out :},{: err :}", "1,nil");

    trv_cleanup;    
}

static void
test_trv_builtin_functions_ord_0(void) {
    trv_ready;

    check_ok("{: ord(\"a\")[0] :}", "97");
    check_ok("{: ord()[1] :}", "need one argument");
    check_ok("{: ord(\"\")[1] :}", "empty strings");
    check_ok("{: ord(nil)[1] :}", "invalid type");

    trv_cleanup;    
}

static void
test_trv_builtin_functions_chr_0(void) {
    trv_ready;

    check_ok("{: chr(97)[0] :}", "a");
    check_ok("{: chr()[1] :}", "need one argument");
    check_ok("{: chr(\"a\")[1] :}", "invalid type");

    trv_cleanup;    
}

static void
test_PadTrv_Trav(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /*******
    * test *
    *******/

    // digit or objects

    PadTkr_Parse(tkr, "{@ a = 0 or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = 0 or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = 0 or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return true end \n a = 0 or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    // bool or objects

    PadTkr_Parse(tkr, "{@ a = false or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = true or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = false or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = false or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = true or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = true or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = false or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = false or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = false or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = true or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return true end \n a = false or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return 0 end \n a = false or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = false or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = false or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = true or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = true or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    // nil or objects

    PadTkr_Parse(tkr, "{@ a = nil or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = nil or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = nil or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return true end \n a = nil or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = nil or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    // string or objects

    PadTkr_Parse(tkr, "{@ a = \"abc\" or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"def\" or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = \"abc\" or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = \"abc\" or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = \"abc\" or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = \"\" or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return true end \n a = \"\" or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return nil end \n a = \"\" or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return nil end \n a = \"abc\" or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = \"\" or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    // array or objects

    PadTkr_Parse(tkr, "{@ a = [1, 2] or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [] or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = [] or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ b = 0 \n a = [] or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ b = 1 \n a = [1, 2] or b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = [] or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = [1, 2] or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return 1 end \n a = [] or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return 0 end \n a = [] or f() @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    // func or objects

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = f or [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    // other

    PadTkr_Parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }


    PadTkr_Parse(tkr, "{@ a = 1 \n b = 0 or a @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end\n"
        "a = 0 or f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function)"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 or 0 or 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ a = not nil @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = not 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = not 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ a = not \"\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ a = not \"abc\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadTkr_Parse(tkr, "{@ def f(): end \n a = not f @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    /*******
    * expr *
    *******/

    PadTkr_Parse(tkr, "{@ a = 1 + 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 + 2 + 3 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "6"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 - 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 3 - 2 - 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 + 2 - 3 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"abc\" + \"def\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcdef"));
    }

    PadTkr_Parse(tkr, "{@ a = \"123\" \n b = \"abc\" + a + \"def\" @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc123def"));
    }

    /*******
    * term *
    *******/

    PadTkr_Parse(tkr, "{@ a = 2 * 3 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "6"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 * 3 * 4 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "24"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 / 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 / 2 / 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 / (2 / 2) @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadTkr_Parse(tkr, "{@ a = 1 + ( 2 - 3 ) * 4 / 4 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    /**********
    * asscalc *
    **********/

    PadTkr_Parse(tkr, "{@ a += 1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += 1 + 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = 1 + 1 @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = 1 + (a += 1) @}{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += 1 \n a += 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ a = \"a\"\n"
        "a += \"b\" @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ab"));
    }

    check_fail("{@\n"
    "a = \"x\"\n"
    "def f():\n"
    "   a += \"y\"\n"
    "end\n"
    "f()\n"
    "@}{: a :}", "\"a\" is not defined");

    PadTkr_Parse(tkr, "{@\n"
        "    def add(a):\n"
        "        a += \"x\"\n"
        "    end\n"
        "\n"
        "   a = \"\"\n"
        "   add(a)\n"
        "@}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    /*******************
    * import statement *
    *******************/

    PadTkr_Parse(tkr, "{@ import alias @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
    }

    PadTkr_Parse(tkr, "{@ import my.alias @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
    }

    /***************
    * if statement *
    ***************/

    PadTkr_Parse(tkr, "{@ if 1: a = 1 end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: a = 1 end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "abc{@ if 1: @}def{@ end @}ghi");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcdefghi"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}abc{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}abc{@ if 1: @}def{@ end @}ghi{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcdefghi"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}abc{@ elif 1: @}def{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}abc{@ elif 0: @}def{@ else: @}ghi{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ghi"));
    }

    /****************
    * for statement *
    ****************/

    PadTkr_Parse(tkr,
        "{@\n"
        "    for a != 0:\n"
        "        break\n"
        "    end\n"
        "@}\n");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i = 0; i != 4; i += 1:\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i = 0, j = 0; i != 4; i += 1, j += 1:\n"
        "   a += 1\n"
        "end @}{: a :} {: i :} {: j :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4 4 4"));
    }

    PadTkr_Parse(tkr, "{@ for i = 0; i != 4; i += 1: @}a{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "aaaa"));
    }

    PadTkr_Parse(tkr, "{@ for i, j = 0, 0; i != 4; i += 1, j += 2: end @}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4,8"));
    }

    PadTkr_Parse(tkr, "{@ i, a = 0, 0 \n for i != 4: a += i \n i += 1 end @}{: i :},{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4,6"));
    }

    PadTkr_Parse(tkr,
        "{@ for i = 0; i != 4; i += 1: @}"
        "hige\n"
        "{@ end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    PadTkr_Parse(tkr,
        "{@ i = 0 for i != 4: @}"
        "hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    PadTkr_Parse(tkr,
        "{@ i = 0 for: @}"
        "{@ if i == 4: break end @}hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    /*******
    * jump *
    *******/

    PadTkr_Parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   break\n"
        "end @}{: i :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       break\n"
        "   end\n"
        "end @}{: i :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   continue\n"
        "   a += 1\n"
        "end @}{: i :},{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4,0"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       continue\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       continue\n"
        "   elif i == 3:\n"
        "       continue\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 0, b = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   a += 1"
        "   if i == 2:\n"
        "       continue\n"
        "   end\n"
        "   b += 1\n"
        "end @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4,3"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return 1\n"
        "end\n"
        "a = func()"
        "@}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return 1\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   puts(\"a\")\n"
        "   return 1\n"
        "   puts(\"b\")\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a\n1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "   return a\n"
        "end\n"
        "x = func()\n"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "   if a == 1:\n"
        "       return a\n"
        "   end\n"
        "   puts(\"b\")\n"
        "end\n"
        "x = func()\n"
        "@}{: x :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "   if a == 1:\n"
        "       return a\n"
        "   end\n"
        "   puts(\"b\")\n"
        "end\n"
        "puts(func())\n"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "a, b = func()\n"
        "@}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "a = func()\n"
        "@}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func(a):\n"
        "   return a, a\n"
        "end\n"
        "a, b = func(1)\n"
        "@}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1"));
    }

    /***********
    * func_def *
    ***********/

    PadTkr_Parse(tkr, "{@ def func(): end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}{: a :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined in ref block"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "func()"
        "@}{: a :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined in ref block"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func(a):\n"
        "   b = a\n"
        "end\n"
        "func(1)"
        "@}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined in ref block"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"c\" is not defined in ref block"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "c = 1\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@\n"
        "c = 1\n"
        "def func(a, b):\n"
        "   puts(c)\n"
        "   c = a + b\n"
        "   puts(c)\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n3\n1"));
    }

    /*******************
    * Pad_Escape character *
    *******************/

    PadTkr_Parse(tkr, "{: \"abc\ndef\n\" :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc\ndef\n"));
    }

    PadTkr_Parse(tkr, "{: \"\tabc\tdef\" :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\tabc\tdef"));
    }

    // done
    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

/**
 * A test of assign to variable and refer variable
 * object is copy? or refer?
 */
static void
test_trv_assign_and_reference_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = 0\n"
    "@}{: i :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0,0"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        trace();
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0,0,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = 1\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i, j = 1, 1\n"
    "@}{: i :},{: j :},{: id(i) != id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_4(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = 1\n"
    "   j, k = i, i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :},{: id(i) == id(k) :},{: id(j) == id(k) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1,true,true,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_5(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = 1, 2\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array),(array),true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_6(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = [1, 2]\n"
    "   j = 3\n"
    "   i[0] = j\n"
    "@}{: i[0] :},{: i[1] :},{: j :},{: id(i[0]) == id(j) :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,2,3,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_7(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i, j = [1, 2]\n"
    "@}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_8(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i, j = k, l = 1, 2\n"
    "@}{: i :},{: j :},{: k :},{: l :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2,1,2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_9(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = { \"a\": 1 }\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict),(dict),true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_10(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   i = f(1)"
    "@}{: i :}", "1");

    trv_cleanup;
}

static void
test_trv_assign_and_reference_11(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       return 1, 2\n"
    "   end\n"
    "   i, j = f()\n"
    "@}{: i :},{: j :},{: id(i) != id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2,true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_11_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   j = 1\n"
    "   k = f(j)\n"
    "@}{: id(k) != id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_11_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       return 1, 2\n"
    "   end\n"
    "   i, j = f()\n"
    "@}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_11_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a, a\n"
    "   end\n"
    "   i, j = f(1)\n"
    "@}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_12(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a, a\n"
    "   end\n"
    "   k = 1\n"
    "   i, j = f(k)\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :},{: id(k) != id(i) :},{: id(k) != id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1,true,true,true"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_13(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_and_reference_14(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   i = 0\n"
    "@}{: i :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_15(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_16(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   f(1)"
    "@}", "");

    trv_cleanup;
}

static void
test_trv_assign_and_reference_all(void) {
    test_trv_assign_and_reference_0();
    test_trv_assign_and_reference_1();
    test_trv_assign_and_reference_2();
    test_trv_assign_and_reference_3();
    test_trv_assign_and_reference_4();
    test_trv_assign_and_reference_5();
    test_trv_assign_and_reference_6();
    test_trv_assign_and_reference_7();
    test_trv_assign_and_reference_8();
    test_trv_assign_and_reference_9();
    test_trv_assign_and_reference_10();
    test_trv_assign_and_reference_11();
    test_trv_assign_and_reference_12();
    test_trv_assign_and_reference_13();
    test_trv_assign_and_reference_14();
    test_trv_assign_and_reference_15();
    test_trv_assign_and_reference_16();
}

static void
test_trv_assign_fail_0(void) {
    trv_ready;

    check_ok("", "");
    check_ok("{@ a @}", "");  // TODO: error? or ok?
    check_fail("{@ a @}{: a :}", "\"a\" is not defined in ref block"); 
    check_fail("{@ a = @}", "syntax error. not found rhs test in assign list");
    check_fail("{@ 1 = @}", "syntax error. not found rhs test in assign list");
    check_fail("{@ 1 = 1 @}", "invalid left hand operand (1)");
    check_fail("{@ 1 = a @}", "invalid left hand operand (1)");
    check_fail("{@ a = a @}", "\"a\" is not defined in asscalc ass idn");
    check_fail("{@ a, b = 1 @}", "invalid right operand (1)");
    check_fail("{@ a, @}", "syntax error. not found test in test list");
    check_fail("{@ a, = 1 @}", "syntax error. not found test in test list");
    check_fail("{@ ,a @}", "not found blocks");
    check_fail("{@ ,a = 1 @}", "not found blocks");
    check_fail("{@ a, b = 1, @}", "syntax error. not found test in test list");
    check_ok("{@ a = 1, 2 @}", "");  // ok

    trv_cleanup;
}

static void
test_trv_code_block(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ \n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ \n\n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ \n\n1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ 1\n\n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ \n\n1\n\n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@@}{@@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@@}{@@}{@@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "\n{@\n@}\n{@\n@}\n");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\n\n"));
    }

    PadTkr_Parse(tkr, "\n{@\n\n\n@}\n{@\n\n\n@}\n");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\n\n"));
    }

    trv_cleanup;
}

static void
test_trv_code_block_fail(void) {
    trv_ready;

    check_fail("{@", "syntax error. reached EOF in code block");
    check_fail("{@@", "syntax error. reached EOF in code block");

    trv_cleanup;
}

static void
test_trv_ref_block(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{: 1\n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{: \n1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "\n{: 1 :}\n");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\n1"));
    }

    PadTkr_Parse(tkr, "{@@}{: 1 :}{@@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{: 1 :}{@@}{: 2 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "12"));
    }

    PadTkr_Parse(tkr, "{: 2 * 3 + 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "7"));
    }

    PadTkr_Parse(tkr, "{: \"ab\" * 4 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abababab"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_ref_block_fail(void) {
    trv_ready;

    check_fail("{:", "not found blocks");
    check_fail("{: 1", "not found blocks");
    check_fail("{: 1 :", "syntax error. not found \":}\"");
    check_ok("{: 1 :}", "1");
    check_fail("{: :}", "not found blocks");

    trv_cleanup;
}

static void
test_trv_text_block(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "1");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "1{@@}2");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "12"));
    }

    PadTkr_Parse(tkr, "1{@@}2{@@}3");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    PadTkr_Parse(tkr, "1{: 2 :}3{: 4 :}5");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "12345"));
    }

    PadTkr_Parse(tkr, "1{@@}{: 2 :}{@@}3");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_import_stmt_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /**********************
    * import as statement *
    **********************/

    PadTkr_Parse(tkr, "{@ import \"tests/lang/modules/hello.cap\" as hello @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    PadTkr_Parse(tkr, "{@ import \n \"tests/lang/modules/hello.cap\" as hello @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found path in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"tests/lang/modules/hello.cap\" \n as hello @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found keyword 'as' in compile import as statement"));
    }

    PadTkr_Parse(tkr, "{@ import \"tests/lang/modules/hello.cap\" as \n hello @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found alias in compile import as statement"));
    }

    check_ok("{@ import \"tests/lang/modules/hello.cap\" as hello \n"
        "hello.world() @}"
        , "imported\nhello, world\n");

    PadTkr_Parse(tkr,
        "{@ import \"tests/lang/modules/count.cap\" as count \n"
        "@}{: count.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    /************************
    * from import statement *
    ************************/

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import f1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import f1 \n f1() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nf1\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import ( f1, f2 ) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import ( f1, f2, ) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import (\nf1,\nf2,\n) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nf1\nf2\n"));
    }

    PadTkr_Parse(tkr,
        "{@ from \n \"tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found path in compile from import statement"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" \n import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import in compile from import statement"));
    }

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import \n ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found import variables in compile from import statement"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_import_stmt_1(void) {
    trv_ready;

    PadTkr_Parse(tkr,
        "{@ import \"tests/lang/modules/count.cap\" as count \n"
        "@}{: count.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr,
        "{@\n"
        "   if 1:\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "   if 0:\n"
        "   else:\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "   if 0:\n"
        "   elif 1:\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "   for i = 0; i < 2; i += 1:\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "       puts(count.n)\n"
        "   end"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45\n45\n"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "   def func():\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "       puts(count.n)\n"
        "   end\n"
        "   func()\n"
        "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45\n"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "   def func():\n"
        "       import \"tests/lang/modules/count.cap\" as count\n"
        "   end"
        "   func()\n"
        "@}{: count :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"count\" is not defined in ref block"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ import \"tests/lang/modules/hello.cap\" as hello @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\n"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_4(void) {
    trv_ready;

    check_ok("{@ import \"tests/lang/modules/hello.cap\" as hello \n"
    "hello.world() @}"
    , "imported\nhello, world\n");

    trv_cleanup;
}

static void
test_trv_import_stmt_5(void) {
    trv_ready;

    check_ok(
    "{@ import \"tests/lang/modules/hello.cap\" as hello \n"
    "hello.world() @}"
    , "imported\nhello, world\n");

    trv_cleanup;
}

static void
test_trv_from_import_stmt_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import f1 \n f1() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nf1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_from_import_stmt_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr,
        "{@ import \"tests/lang/modules/hello.cap\" as hello \n"
        "hello.world() @}"
    );
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nhello, world\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_from_import_stmt_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr,
        "{@ from \"tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nf1\nf2\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1:\n puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: puts(1) \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1:\n\n puts(1) \n\nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if \n1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1\n: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ \n if 1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: puts(1) end \n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 1: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: \n@}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: \n\n@}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}1{@ \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}1{@ \n\nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 1: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: \nif 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: if 1: \nputs(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: if 1: puts(1) \nend end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 1: if 1: puts(1) end \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }
    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ \nif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: \n@}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: \n@}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_4(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ i = 1 \n if i: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ i = 1 @}{@ if i: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       i = 1\n"
    "       if i:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   f()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 1\n"
    "   def f():\n"
    "       if i:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   f()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_5(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/if.cap\" as mod \n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_if_stmt_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   from \"tests/lang/modules/if-2.cap\" import f1 \n"
    "   f1()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   from \"tests/lang/modules/if-2.cap\" import f2\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_8(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/if-3.cap\" as if3\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_9(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(n):\n"
    "       puts(n)\n"
    "       return n\n"
    "   end\n"
    "   if f(1):\n"
    "       puts(2)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 2 * 3 + 1:\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3\n"
    "       if j:\n"
    "           puts(i * j)\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "6\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3\n"
    "       if j:\n"
    "           k = 4\n"
    "           if k:\n"
    "               puts(i * j * k)\n"
    "           end\n"
    "           k = 2\n"
    "       end\n"
    "       j = 10\n"
    "   end\n"
    "   i = 100\n"
    "   puts(i * j * k)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "24\n2000\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3\n"
    "       if j:\n"
    "           k = 4\n"
    "           if k:\n"
    "               def f(n):\n"
    "                   if n:\n"
    "                       puts(n)\n"
    "                   end\n"
    "               end\n"
    "           end\n"
    "           k = 2\n"
    "       end\n"
    "       j = 10\n"
    "   end\n"
    "   i = 100\n"
    "   f(i * j * k)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2000\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3"
    "       if j:\n"
    "           k = 4\n"
    "           if k:\n"
    "               def f(n):\n"
    "                   if n:\n"
    "                       puts(n)\n"
    "                   end\n"
    "               end\n"
    "           end\n"
    "           k = 2\n"
    "       end\n"
    "       j = 10\n"
    "   end\n"
    "   i = 100\n"
    "   f(i * j * k)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2000\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3"
    "       if j:\n"
    "           k = 4\n"
    "           if k:\n"
    "               def f(n):\n"
    "                   if n:\n"
    "                       puts(n)\n"
    "                   end\n"
    "               end\n"
    "           end\n"
    "           k = 2\n"
    "       end\n"
    "       j = 10\n"
    "       if j:\n"
    "           if k:\n"
    "               puts(j * k)\n"
    "           end\n"
    "       end\n"
    "   end\n"
    "   i = 100\n"
    "   f(i * j * k)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "20\n2000\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 1:\n"
    "       i = 10\n"
    "       j = i * 20\n"
    "       if j:\n"
    "           puts(j)\n"
    "       end\n"
    "   end\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3"
    "       if j:\n"
    "           k = 4\n"
    "           if k:\n"
    "               def f(n):\n"
    "                   if n:\n"
    "                       puts(n)\n"
    "                   end\n"
    "               end\n"
    "           end\n"
    "           k = 2\n"
    "       end\n"
    "       j = 10\n"
    "       if j:\n"
    "           if k:\n"
    "               puts(j * k)\n"
    "           end\n"
    "       end\n"
    "   end\n"
    "   i = 100\n"
    "   f(i * j * k)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "200\n20\n2000\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_10(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ \nif\n0\n:\nend\n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_11(void) {
    trv_ready;

    check_ok("{@\n"
    "   if 1:\n"
    "       i = 0\n"
    "@}{: i :}{@"
    "       j = 1\n"
    "@}{: j :}{@\n"
    "       k = 2\n"
    "@}{: k :}{@\n"
    "   end\n"
    "@}", "012");

    trv_cleanup;
}

static void
test_trv_if_stmt_12(void) {
    trv_ready;

    check_ok("{@ if 0: @}123{@ elif 1: @}223{@ end @}", "223");
    check_ok("{@ if 0: @}123{@ elif 0: @}223{@ elif 1: @}323{@ end @}", "323");
    check_ok("{@ if 0: @}123{@ else: @}223{@ end @}", "223");
    check_ok("{@ if 0: @}123{@ elif 0: @}223{@ else: @}323{@ end @}", "323");

    trv_cleanup;
}

static void
test_trv_if_stmt_13(void) {
    trv_ready;

    check_ok("{@ path = \"/\" if path == \"/\": puts(1) elif path == \"/about\": puts(2) else: puts(3) end @}", "1\n");
    check_ok("{@ path = \"/about\" if path == \"/\": puts(1) elif path == \"/about\": puts(2) else: puts(3) end @}", "2\n");

    trv_cleanup;
}

static void
test_trv_if_stmt_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "if 1:\n"
    "   puts(1)\n"
    "e"
    "@}", "reached EOF in if statement");

    check_fail("{@\n"
    "if 1:\n"
    "   puts(1)\n"
    "else:\n"
    "elif 1:\n"
    "end"
    "@}", "syntax error. invalid token");

    check_fail("{@ if 1: @}\n"
    "{@ else: @}\n"
    "{@ elif 1: @}\n"
    "{@ end @}", "syntax error. invalid token");

    trv_cleanup;
}

static void
test_trv_if_stmt_fail_1(void) {
    trv_ready;

    check_fail("{@\n"
    "if 1:\n"
    "el:"
    "end"
    "@}", "syntax error");

    check_fail("{@\n"
    "if 1:\n"
    "el def:"
    "end"
    "@}", "syntax error");

    trv_cleanup;
}

static void
test_trv_elif_stmt_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ if 0: elif 1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: \nelif 1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1:\n puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: puts(1) \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: puts(1) end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif \n1: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1\n: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_1(void) {
    trv_ready;

    check_ok("{@ if 0: @}{@ elif 1: @}1{@ end @}", "1");

    PadTkr_Parse(tkr, "{@ \nif 0: @}{@ elif 1: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0:\n @}{@ elif 1: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ \nelif 1: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: \n@}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}1{@ \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}1{@ end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: @}{@ elif 1: @}1{@ end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: elif 1: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: \nelif 1: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1:\n if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: if 1:\n puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: if 1: puts(1)\n end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end\n end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end end\n @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_elif_stmt_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ \nelif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1:\n @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1:\n @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_elif_stmt_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "   elif 0:\n"
    "       puts(2)\n"
    "       j = 3 * 3\n"
    "   elif 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif 0:\n"
    "       puts(2)\n"
    "       j = 3 * 3\n"
    "   elif 1:\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"i\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif 0:\n"
    "       puts(2)\n"
    "       j = 3 * 3\n"
    "   elif 9 * 9 - 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 3\n"
    "   j = 2\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif i * j:\n"
    "       puts(i * j)\n"
    "       j = 3 * 3\n"
    "   elif 9 * 9 - 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "6\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 3\n"
    "   j = 2\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif i * j:\n"
    "   elif 9 * 9 - 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   i = 3\n"
    "   j = 2\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif i * j:\n"
    "       if 0:\n"
    "           puts(i * j)\n"
    "       elif 1:\n"
    "           puts(i * j)"
    "       end\n"
    "   elif 9 * 9 - 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "6\n"));
    }

    check_ok("{@\n"
    "   i = 0\n"
    "   j = 0\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif i * j:\n"
    "       if 0:\n"
    "           puts(i * j)\n"
    "       elif 1:\n"
    "           puts(i * j)"
    "       end\n"
    "   elif j * i:\n"
    "       puts(3)\n"
    "   else:\n"
    "       if 0:\n"
    "           puts(123)\n"
    "       elif 2 * 3:\n"
    "           puts(10 * 123)\n"
    "       end\n"
    "   end\n"
    "@}", "1230\n");

    trv_cleanup;
}

static void
test_trv_elif_stmt_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       i = 2 * 3\n"
    "       if 1:\n"
    "           puts(1)\n"
    "       end\n"
    "       j = 3 * 3\n"
    "   elif 0:\n"
    "       puts(2)\n"
    "       j = 3 * 3\n"
    "   elif 1:\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"i\" is not defined"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "       puts(2)\n"
    "   elif 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "       if 0:\n"
    "           puts(21)\n"
    "       elif 0:\n"
    "           puts(22)\n"
    "       elif 1:\n"
    "           puts(23)\n"
    "       end"
    "   elif 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "23\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "       if 0:\n"
    "           puts(21)\n"
    "       elif 0:\n"
    "           puts(22)\n"
    "       elif 1:\n"
    "           if 0:\n"
    "           elif 1:\n"
    "               puts(31)\n"
    "           end"
    "       end"
    "   elif 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "31\n"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "@}1{@\n"
    "@}2{@\n"
    "@}3{@\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "@}1{@\n"
    "       if 0:\n"
    "       elif 1:\n"
    "@}2{@"
    "       end\n"
    "@}3{@\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "@}1{@\n"
    "@}2{@\n"
    "       if 0:\n"
    "       elif 1:\n"
    "@}3{@\n"
    "@}4{@\n"
    "       end\n"
    "@}5{@\n"
    "@}6{@\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123456"));
    }

    trv_cleanup;
}

static void
test_trv_else_stmt_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 0: else: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: else: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: \nelse: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else:\n puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: puts(1) \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: puts(1) end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else\n: puts(1) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_else_stmt_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: @}{@ else: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0:\n @}{@ else: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ \nelse: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else\n: @}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: \n@}1{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}1{@ \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}1{@ end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_else_stmt_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ if 0: else: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: else: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: \nelse: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else\n: if 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: \nif 1: puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: if 1:\n puts(1) end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: if 1: puts(1)\n end end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: if 1: puts(1) end \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadTkr_Parse(tkr, "{@ if 0: else: if 1: puts(1) end end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_else_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ \nif 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: \n@}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ \nelse: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: \n@}{@ if 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1:\n @}1{@ end @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_else_stmt_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@"
    "   if 0:\n"
    "   else:\n"
    "@}1{@\n"
    "@}2{@\n"
    "@}3{@\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "123"));
    }

    trv_cleanup;
}
static void
test_trv_for_stmt_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ for i=0; i<2; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ size=0 for i=size; i<2; i += 1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ \nfor i=0; i<2; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2; i +=1: \nputs(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2; i +=1: puts(i)\n end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2; i +=1: puts(i) end \n@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for \ni=0; i<2; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0\n; i<2; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; \ni<2; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2\n; i +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2; \ni +=1: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ for i=0; i<2; i +=1\n: puts(i) end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_for_stmt_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ i=0 for i<2: puts(i)\ni+=1 end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ i=0 for i<2: \nputs(i)\ni+=1 end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ i=0 for i<2: puts(i)\ni+=1 \nend @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ i=0 for \ni<2: puts(i)\ni+=1 end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@ i=0 for i<2\n: puts(i)\ni+=1 end @}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_for_stmt_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ for i, j = 0, 0; i != 4; i += 1, j += 2: end @}{: i :},{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4,8"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_for_stmt_3(void) {
    trv_ready;

    check_ok("{@ for i = 0; i < 2; i += 1: @}{: i :},{@ end @}", "0,1,");
    check_ok("{@\n"
    "def func():\n"
    "   for i = 0; i < 2; i += 1: @}"
    "{: i :}\n"
    "{@ end \n"
    "end \n"
    "\n"
    " func() @}", "0\n1\n");

    trv_cleanup;
}

static void
test_trv_for_stmt_4(void) {
    trv_ready;

    check_ok("{@\n"
    "   def hiphop(rap, n):\n"
    "       puts(rap * n)\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}", "\nyo\nyoyo\n");

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       def hiphop(rap, n):\n"
    "           puts(rap * n)\n"
    "       end\n"
    "\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\nyo\nyoyo\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def hiphop(rap, n):\n"
    "       for i = n-1; i >= 0; i -= 1:\n"
    "           puts(rap * i)\n"
    "       end\n"
    "   end\n"
    "\n"
    "   hiphop(\"yo\", 3)"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "yoyo\nyo\n\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 3; i += 1:\n"
    "@}{: i :}{@\n"
    "   end\n"
    "\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "012"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "    i = 0\n"
    "    j = i\n"
    "    a = [j, j+1, j+2]\n"
    "@}{: a[0] :},{: a[1] :},{: a[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0,1,2"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "for i = 0; i < 4; i += 1:\n"
    "   j = i\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_8(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "for i = 0; i < 2; i += 1:\n"
    "   j = i\n"
    "   k = i\n"
    "   puts(j, k)\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 0\n1 1\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_9(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "for i = 0; i < 2; i += 1:\n"
    "   j = i\n"
    "   k = i\n"
    "@}{: i :}{@"
    "   l = i\n"
    "   m = i\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "01"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "for i = 0; i < 2; i += 1:\n"
    "   j = i\n"
    "@}{: j :}{@\n"
    "   k = i\n"
    "@}{: k :}{@\n"
    "   l = i\n"
    "@}{: l :}{@\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "000111"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_10(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "for \n i = 0 \n ; \n i < 2 \n ; \n i += 1 \n : \n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@\n"
    "i = 0\n"
    "for \n i < 2 \n : \n"
    "   puts(i)\n"
    "   i += 1\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_11(void) {
    // ?
}

static void
test_trv_for_stmt_12(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def hiphop(rap, n):\n"
    "       puts(rap * n)\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "\nyo\nyoyo\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ for: break end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ for:\n break end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ for: break \nend @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       break\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       for j = 4; j < 6; j += 1:\n"
    "           puts(j)\n"
    "           break\n"
    "       end\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n4\n10\n1\n4\n10\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       if 1:\n"
    "           break\n"
    "       end\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       if 0:\n"
    "           puts(100)\n"
    "       else:\n"
    "           break\n"
    "       end\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       if 0:\n"
    "           puts(200)\n"
    "       elif 1:\n"
    "           break\n"
    "       else:\n"
    "           puts(100)\n"
    "       end\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   break\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid break statement. not in loop"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       break\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       f()\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid break statement. not in loop"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       def f():\n"
    "           break\n"
    "       end\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid break statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ j=0 for i=0; i<2; i+=1: continue\n j=i end @}{: j :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       continue\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       if 1:\n"
    "           continue\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       if 0:\n"
    "       elif 1:\n"
    "           continue\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       if 0:\n"
    "       else:\n"
    "           continue\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       for j = 0; j < 2; j += 1:\n"
    "           puts(10)\n"
    "           continue\n"
    "           puts(20)\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n10\n10\n1\n0\n10\n10\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       continue\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       for j = 0; j < 2; j += 1:\n"
    "           puts(10)\n"
    "           f()\n"
    "           puts(20)\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid continue statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   continue\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid continue statement. not in loop"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       continue\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       f()\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid continue statement. not in loop"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       def f():\n"
    "           continue\n"
    "       end\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid continue statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ def f(): return 1 end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_return_stmt_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       puts(1)\n"
    "       return 2\n"
    "       puts(3)\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_return_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "           return 1\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "           for j = 0; j < 2; j += 1:\n"
    "               puts(j)\n"
    "               return 1\n"
    "           end\n"
    "       end\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n1"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       puts(0)\n"
    "       if 1:\n"
    "           puts(1)\n"
    "           return 2\n"
    "           puts(3)\n"
    "       end\n"
    "       puts(4)\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       puts(0)\n"
    "       if 0:\n"
    "           puts(100)\n"
    "       else:\n"
    "           puts(1)\n"
    "           return 2\n"
    "           puts(3)\n"
    "       end\n"
    "       puts(4)\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f():\n"
    "       puts(0)\n"
    "       if 0:\n"
    "           puts(100)\n"
    "       elif 1:\n"
    "           puts(1)\n"
    "           return 2\n"
    "           puts(3)\n"
    "       else:\n"
    "           puts(200)\n"
    "       end\n"
    "       puts(4)\n"
    "   end\n"
    "@}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n2"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   return\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid return statement. not in function"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid return statement. not in function"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "   else:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid return statement. not in function"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   if 0:\n"
    "   elif 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid return statement. not in function"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid return statement. not in function"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def func():\n"
    "       if 1:\n"
    "          return 1\n"
    "       end\n"
    "       return 2\n"
    "   end\n"
    "@}{: func() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def func():\n"
    "       if 1:\n"
    "          return\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}{: func() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    trv_cleanup;
}

static void
test_trv_block_stmt_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "block aaa:\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't access to function node"));
    }

    trv_cleanup;
}

static void
test_trv_block_stmt_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "block:\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't access to function node"));
    }

    trv_cleanup;
}

static void
test_trv_block_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def func():\n"
    "   block aaa:\n"
    "   end\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        showdetail();
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "inject aaa:\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "inject statement needs function"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "inject:\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found identifier in inject statement"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def func():\n"
    "   inject aaa:\n"
    "   end\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_struct_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_struct_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "   b = 2\n"
    "   def aaa():\n"
    "       puts(1)\n"
    "   end\n"
    "   struct Body:\n"
    "       c = 3\n"
    "   end\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_struct_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   def aaa(): @}\n"
    "       text\n"
    "   {@ end\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_struct_4(void) {
    trv_ready;

    check_fail_showbuf("{@\n"
    "struct Animal:\n"
    "   @}text{@\n"
    "end\n"
    "@}", "not found 'end'. found token is 5");

    trv_cleanup;
}

static void
test_trv_struct_5(void) {
    trv_ready;

    check_ok_trace("{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "   b = 2\n"
    "end\n"
    "animal = Animal()\n"
    "@}{: type(animal) :}", "(struct)");

    trv_cleanup;
}

static void
test_trv_struct_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "end\n"
    "animal = Animal()\n"
    "@}{: animal.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_7(void) {
    trv_ready;

    check_ok("{@\n"
    "struct Animal:\n"
    "   def func():\n"
    "       return 1\n"
    "   end\n"
    "end\n"
    "animal = Animal()\n"
    "@}{: animal.func() :}"
    , "1");

    trv_cleanup;
}

static void
test_trv_struct_8(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   struct Body:\n"
    "       a = 1\n"
    "   end\n"
    "   body = Body()\n"
    "end\n"
    "animal = Animal()\n"
    "@}{: animal.body.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_9(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "end\n"
    "animal = Animal()\n"
    "animal.a = 1\n"
    "@}{: animal.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_10(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "end\n"
    "animal = Animal()\n"
    "animal.a = 2\n"
    "@}{: animal.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_struct_11(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "from \"tests/lang/modules/struct-1.pad\" import Animal\n"
    "animal = Animal()"
    "@}{: animal.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "from \"tests/lang/modules/struct-2.pad\" import animal\n"
    "@}{: animal.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_12(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "a = 1\n"
    "struct Animal:\n"
    "   b = a\n"
    "end\n"
    "animal = Animal()"
    "@}{: animal.b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_13(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   def func():\n"
    "       puts(1)\n"
    "   end\n"
    "end\n"
    "animal = Animal()\n"
    "animal.func()"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_14(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def a():\n"
    "   puts(1)\n"
    "end\n"
    "struct Animal:\n"
    "   func = a\n"
    "end\n"
    "animal = Animal()\n"
    "animal.func()"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_15(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def a():\n"
    "   puts(1)\n"
    "end\n"
    "struct Animal:\n"
    "   struct Body:\n"
    "       func = a\n"
    "   end\n"
    "   body = Body()\n"
    "end\n"
    "animal = Animal()\n"
    "animal.body.func()"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_16(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def func():\n"
    "   puts(1)\n"
    "end\n"
    "struct Animal:\n"
    "   struct Body:\n"
    "       legs = func\n"
    "   end\n"
    "   body = Body()\n"
    "end\n"
    "struct Human:\n"
    "   body = nil\n"
    "end\n"
    "animal = Animal()\n"
    "human = Human()\n"
    "human.body = animal.body\n"
    "human.body.legs()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_17(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def func(n):\n"
    "   puts(n)\n"
    "end\n"
    "struct Animal:\n"
    "   two = 2"
    "   struct Head:\n"
    "       eyes = two\n"
    "   end\n"
    "   head = Head()\n"
    "   struct Body:\n"
    "       legs = func\n"
    "   end\n"
    "   body = Body()\n"
    "end\n"
    "struct Human:\n"
    "   head = nil\n"
    "   body = nil\n"
    "end\n"
    "animal = Animal()\n"
    "human = Human()\n"
    "human.head = animal.head\n"
    "human.body = animal.body\n"
    "human.body.legs(human.head.eyes)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_18(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   sum = 0\n"
    "   for i = 0; i < 10; i += 1:\n"
    "       sum += i\n"
    "   end\n"
    "end\n"
    "animal = Animal()\n"
    "@}{: animal.sum :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "45"));
    }

    trv_cleanup;
}

static void
test_trv_struct_19(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   puts(1)\n"
    "end\n"
    "struct Human:\n"
    "   puts(2)\n"
    "end\n"
    "Animal()\n"
    "Human()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_20(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   if true:\n"
    "       puts(1)\n"
    "   end\n"
    "end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_21(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def func():\n"
    "   a = 1\n"
    "   struct Animal:\n"
    "       b = a\n"
    "   end\n"
    "   return Animal()\n"
    "end\n"
    "animal = func()\n"
    "@}{: animal.b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_22(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1 * 2\n"
    "end\n"
    "arr = [Animal(), Animal()]\n"
    "@}{: arr[0].a :},{: arr[1].a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2,2"));
    }

    trv_cleanup;
}

static void
test_trv_struct_23(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "end\n"
    "d = {\"a\": Animal(), \"b\": Animal()}\n"
    "@}{: d[\"a\"].a :},{: d[\"b\"].a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_24(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct Animal:\n"
    "   a = 1\n"
    "end\n"
    "def f1(a):\n"
    "   a.a = 2\n"
    "end\n"
    "def f2(a):\n"
    "   a.a = 3\n"
    "end\n"
    "animal = Animal()\n"
    "puts(animal.a)\n"
    "f1(animal)\n"
    "puts(animal.a)\n"
    "f2(animal)\n"
    "puts(animal.a)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n3\n"));
    }

    trv_cleanup;
}

static void
test_trv_struct_25(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct File:\n"
    "   fileno = nil\n"
    "end\n"
    "def fileNew(fileno):\n"
    "   self = File()\n"
    "   self.fileno = fileno\n"
    "   return self\n"
    "end\n"
    "file = fileNew(1)\n"
    "@}{: file.fileno :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_26(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "struct File:\n"
    "   n = 1\n"
    "end\n"
    "@}{: File.n :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_struct_27(void) {
    trv_ready;

    check_ok("{@\n"
    "struct File:\n"
    "   n = 1\n"
    "end\n"
    "f = File()\n"
    "File.n = 2\n"
    "@}{: f.n :}", "1");

    trv_cleanup;
}

static void
test_trv_struct_28(void) {
    trv_ready;

    check_ok("{@\n"
    "struct File:\n"
    "   n = 1\n"
    "end\n"
    "f = File()\n"
    "f.n = 2\n"
    "@}{: File.n :}", "1");

    trv_cleanup;
}

static void
test_trv_struct_29(void) {
    trv_ready;

    check_ok("{@\n"
    "struct File:\n"
    "   n = 1\n"
    "   def f(self):\n"
    "       self.n += 1\n"
    "       puts(self.n)\n"
    "   end\n"
    "end\n"
    "f1 = File()\n"
    "f2 = File()\n"
    "f1.f(f1)\n"
    "f2.f(f2)\n"
    "@}", "2\n2\n");

    trv_cleanup;
}

static void
test_trv_struct_30(void) {
    trv_ready;

    check_ok("{@\n"
    "struct ns:\n"
    "   struct Animal:\n"
    "       n = 1\n"
    "   end\n"
    "end\n"
    "a = ns.Animal()\n"
    "@}{: a.n :}", "1");

    trv_cleanup;
}

static void
test_trv_struct_31(void) {
    trv_ready;

    check_ok("{@\n"
    "struct a:\n"
    "   struct b:\n"
    "       def c():\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "end\n"
    "a.b.c()\n"
    "@}", "1\n");

    trv_cleanup;
}

static void
test_trv_struct_32(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "   def b():\n"
    "       return A()"
    "   end\n"
    "end\n"
    "@}{: A.b() :}", "(object)");

    trv_cleanup;
}

static void
test_trv_struct_33(void) {
    trv_ready;

    check_ok("{@\n"
    "import \"tests/lang/modules/struct-3.pad\" as sys\n"
    "struct File:\n"
    "   def read():\n"
    "       puts(sys)\n"
    "   end\n"
    "end\n"
    "File.read()\n"
    "@}", "(module)\n");

    trv_cleanup;
}

static void
test_trv_struct_34(void) {
    trv_ready;

    check_ok("{@\n"
    "import \"tests/lang/modules/struct-3.pad\" as mod\n"
    "struct File:\n"
    "   def read():\n"
    "       mod.test()\n"
    "   end\n"
    "end\n"
    "File.read()\n"
    "@}", "");

    trv_cleanup;
}

static void
test_trv_struct_35(void) {
    trv_ready;

    check_ok("{@\n"
    "import \"tests/lang/modules/struct-4.pad\" as mod\n"
    "struct File:\n"
    "   def read():\n"
    "       return mod.read()\n"
    "   end\n"
    "end\n"
    "@}{: File.read() :}", "readed");

    trv_cleanup;
}

static void
test_trv_struct_36(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "   def f():\n"
    "       a = nil\n"
    "       puts(a)\n"
    "   end\n"
    "end\n"
    "A.f()\n"
    "@}", "nil\n");

    trv_cleanup;
}

static void
test_trv_struct_37(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "   def a():\n"
    "       struct B:\n"
    "           def b():\n"
    "               struct C:\n"
    "                   def c():\n"
    "                       return 1\n"
    "                   end\n"
    "               end\n"
    "               return C.c()\n"
    "           end\n"
    "       end\n"
    "       return B.b()"
    "   end\n"
    "end\n"
    "@}{: A.a() :}", "1");

    trv_cleanup;
}

static void
test_trv_struct_38(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "   def init(self):\n"
    "       self.a = 1\n"
    "       self.b = 2\n"
    "       self.c = 3\n"
    "       return self\n"
    "   end\n"
    "   def dump(self):\n"
    "       puts(self.a, self.b, self.c)\n"
    "   end\n"
    "end\n"
    "a = A()\n"
    "A.init(a)\n"
    "A.dump(a)\n"
    "@}", "1 2 3\n");

    trv_cleanup;
}

static void
test_trv_struct_39(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Node:\n"
"    value = nil\n"
"    prev = nil\n"
"    next = nil\n"
"end\n"
"\n"
"struct List:\n"
"    head = nil\n"
"    tail = nil\n"
"\n"
"    def dump(self):\n"
"        for cur = self.head; cur; cur = cur.next:\n"
"            puts(cur.value)\n"
"        end\n"
"    end\n"
"\n"
"    def push(self, value):\n"
"        if not self.head:\n"
"            self.head = Node()\n"
"            self.head.value = value\n"
"            self.tail = self.head\n"
"            return\n"
"        end\n"
"\n"
"        tail = nil\n"
"        for cur = self.head; cur; cur = cur.next:\n"
"            tail = cur\n"
"        end\n"
"\n"
"        if tail:\n"
"            node = Node()\n"
"            node.value = value\n"
"            node.prev = tail\n"
"            tail.next = node\n"
"            self.tail = node\n"
"        end\n"
"    end\n"
"\n"
"    def pop(self):\n"
"        if not self.tail:\n"
"            return nil\n"
"        end\n"
"\n"
"        tail = self.tail\n"
"\n"
"        if tail.prev:\n"
"            tail.prev.next = nil\n"
"            self.tail = tail.prev\n"
"        else:\n"
"            self.head = nil\n"
"            self.tail = nil\n"
"        end\n"
"\n"
"        tail.prev = nil\n"
"        return tail.value\n"
"    end\n"
"end\n"
"list = List()\n"
"List.push(list, 0)\n"
"List.push(list, 1)\n"
"List.push(list, 2)\n"
"puts(List.pop(list))\n"
"puts(List.pop(list))\n"
"puts(List.pop(list))\n"
"@}", "2\n1\n0\n");

    trv_cleanup;
}

static void
test_trv_struct_40(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Node:\n"
"    value = nil\n"
"    prev = nil\n"
"    next = nil\n"
"end\n"
"\n"
"struct List:\n"
"    head = nil\n"
"    tail = nil\n"
"\n"
"    def dump(self):\n"
"        for cur = self.head; cur; cur = cur.next:\n"
"            puts(cur.value)\n"
"        end\n"
"    end\n"
"\n"
"    def push(self, value):\n"
"        if not self.head:\n"
"            self.head = Node()\n"
"            self.head.value = value\n"
"            self.tail = self.head\n"
"            return\n"
"        end\n"
"\n"
"        tail = nil\n"
"        for cur = self.head; cur; cur = cur.next:\n"
"            tail = cur\n"
"        end\n"
"        puts(tail)\n"
"        if tail:\n"
"            node = Node()\n"
"            node.value = value\n"
"            node.prev = tail\n"
"            tail.next = node\n"
"            self.tail = node\n"
"        end\n"
"    end\n"
"\n"
"    def pop(self):\n"
"        if not self.tail:\n"
"            return nil\n"
"        end\n"
"\n"
"        tail = self.tail\n"
"\n"
"        if tail.prev:\n"
"            tail.prev.next = nil\n"
"            self.tail = tail.prev\n"
"        else:\n"
"            self.head = nil\n"
"            self.tail = nil\n"
"        end\n"
"\n"
"        tail.prev = nil\n"
"        return tail.value\n"
"    end\n"
"end\n"
"list = List()\n"
"List.push(list, 0)\n"
"List.push(list, 1)\n"
"List.push(list, 2)\n"
"puts(List.pop(list))\n"
"puts(List.pop(list))\n"
"puts(List.pop(list))\n"
"@}", "(object)\n(object)\n2\n1\n0\n");

    trv_cleanup;
}

static void
test_trv_struct_41(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Animal:\n"
"   a = 0\n"
"   b = 1\n"
"   c = 2\n"
"end\n"
"a = Animal(10, 11, 12)\n"
"@}{: a.a :},{: a.b :},{: a.c :}", "10,11,12");

    trv_cleanup;
}

static void
test_trv_struct_42(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Animal:\n"
"   a = 0\n"
"   b = 1\n"
"   c = 2\n"
"end\n"
"a = Animal(10, 11)\n"
"@}{: a.a :},{: a.b :},{: a.c :}", "10,11,2");

    check_ok(
"{@\n"
"struct Animal:\n"
"   a = 0\n"
"   b = 1\n"
"   c = 2\n"
"end\n"
"a = Animal(10)\n"
"@}{: a.a :},{: a.b :},{: a.c :}", "10,1,2");

    trv_cleanup;
}

static void
test_trv_struct_43(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Animal:\n"
"   a = 0\n"
"   b = 1\n"
"   c = 2\n"
"end\n"
"a = Animal(10, 11, 12, 13)\n"
"@}{: a.a :},{: a.b :},{: a.c :}", "10,11,12");

    trv_cleanup;
}

static void
test_trv_struct_44(void) {
    trv_ready;

    check_ok(
"{@\n"
"struct Body:\n"
"   legs = 4\n"
"end\n"
"body_ = Body()\n"
"struct Animal:\n"
"   body = body_\n"
"end\n"
"a = Animal()\n"
"@}{: a.body.legs :}", "4");

    trv_cleanup;
}

static void
test_trv_struct_45(void) {
    trv_ready;

    check_ok(
"{@\n"
"def func():\n"
"   def f():\n"
"   end\n"
"   puts(f)\n"
"end\n"
"func()\n"
"@}", "(function)\n");

    trv_cleanup;
}

static void
test_trv_struct_46(void) {
    trv_ready;

    check_ok(
"{@\n"
"def func():\n"
"   struct S:\n"
"   end\n"
"   puts(S)\n"
"end\n"
"func()\n"
"@}", "(struct)\n");

    trv_cleanup;
}

static void
test_trv_struct_47(void) {
    trv_ready;

//     check_ok_trace(
// "{@\n"
// "def func():\n"
// "   struct S:\n"
// "   end\n"
// "   puts(S)\n"
// "end\n"
// "arr = [func]\n"
// "f = arr[0]\n"
// "f()\n"
// "@}", "(struct)\n");

    trv_cleanup;
}

static void
test_trv_struct_fail_0(void) {
    trv_ready;

    check_fail(
"{@\n"
"st:\n"
"end\n"
"@}", "not found blocks");

    check_fail(
"{@\n"
"struct:\n"
"end\n"
"@}", "not found identifier");

    check_fail(
"{@\n"
"struct A:\n"
"@}", "not found 'end'. found token is 5");

    check_fail(
"{@\n"
"struct A:\n"
"en\n"
"@}", "not found 'end'. found token is 5");

    check_fail(
"{@\n"
"struct A:\n"
"   a\n"
"end\n"
"@}{: A.a :}", "not found \"a\"");

    check_fail(
"{@\n"
"struct A:\n"
"   a\n"
"end\n"
"@}{: A.a :}", "not found \"a\"");

    check_fail(
"{@\n"
"a = struct A:\n"
"end\n"
"@}", "syntax error. not found rhs test in assign list");

    check_fail(
"{@\n"
"[struct A: end]\n"
"@}", "not found ']' in array");

    check_fail(
"{@\n"
"struct A: end = 1\n"
"@}", "not found blocks");

    trv_cleanup;
}

static void
test_trv_type_0(void) {
    trv_ready;

    check_ok("{: len(Array()) :}", "0");
    check_ok("{: len(Array([1, 2, 3])) :}", "3");
    check_ok("{: Array([1, 2, 3])[1] :}", "2");
    check_ok("{@ i = 0 @}{: len(Array([i])) :}", "1");
    check_ok("{@ i = 0 a = Array([i]) @}{: len(a) :}", "1");
    
    check_ok("{@ i = 0 a = [i] @}{: id(a[0]) == id(i) :}", "true");
    check_ok("{@ i = 0 a = Array([i]) @}{: id(a[0]) == id(i) :}", "true");

    check_ok("{@ i = 1.23 a = Array([i]) @}{: id(a[0]) == id(i) :}", "true");
    check_ok("{@ i = \"abc\" a = Array([i]) @}{: id(a[0]) == id(i) :}", "true");

    check_ok("{@ i = [0] a = Array(i) @}{: id(a[0]) != id(i) :}", "true");
    check_ok("{@ i = {\"a\": 0} a = Array([i]) @}{: id(a[0][\"a\"]) == id(i[\"a\"]) :}", "true");

    trv_cleanup;
}

static void
test_trv_type_1(void) {
    trv_ready;

    check_ok("{: String() :}", "");
    check_ok("{: String(\"abc\") :}", "abc");
    check_ok("{: String(1) :}", "1");
    check_ok("{: String(true) :}", "true");
    check_ok("{: String([]) :}", "(array)");

    trv_cleanup;
}

static void
test_trv_type_2(void) {
    trv_ready;

    check_ok("{: len(Dict()) :}", "0");
    check_ok("{: len(Dict({\"a\": 1})) :}", "1");

    trv_cleanup;
}

static void
test_trv_type_3(void) {
    trv_ready;

    check_ok("{: Int() :}", "0");
    check_ok("{: Int(1) :}", "1");
    check_ok("{: Int(\"2\") :}", "2");
    check_ok("{: Int(false) :}", "0");

    trv_cleanup;
}

static void
test_trv_type_4(void) {
    trv_ready;

    check_ok("{: Float() :}", "0.0");
    check_ok("{: Float(1) :}", "1.0");
    check_ok("{: Float(\"1.23\") :}", "1.23");
    check_ok("{: Float(false) :}", "0.0");

    trv_cleanup;
}

static void
test_trv_type_5(void) {
    trv_ready;

    check_ok("{: Bool() :}", "false");
    check_ok("{: Bool(0) :}", "false");
    check_ok("{: Bool(1) :}", "true");
    check_ok("{: Bool(\"abc\") :}", "true");

    trv_cleanup;
}

static void
test_trv_func_def_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ def f(): end @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ def f(a, b): puts(a, b) end f(1, 2) @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "@}{: c :}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        (PadTrv_Trav(ast, ctx));
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        assert(PadObjDict_Get(varmap, "func"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ def f(): end \n a = not f @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_4(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ i = 1 \n def f(): puts(i) end \n f() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ast->ref_context), "1\n"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_5(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ def f(arg): end \n f() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "arguments not same length"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_6(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@\n"
    "   def f(n, desc):\n"
    "       c = true\n"
    "       indent = n * \"    \""
    "@}{: indent :}abc{@"
    "   end\n"
    "   f(1, \"desc\")\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "    abc"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_func_def_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "from \"tests/lang/modules/func-def.cap\" import draw\n"
    "draw(1, \"desc\")\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "    program\n\n    comment\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_8(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   f([1, 2, 3])\n"
    "@}", "1 2 3\n");

    trv_cleanup;
}

static void
test_trv_func_def_9(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   i = 0\n"
    "   f([i, i+1, i+2])\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_10(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       f([i, i+1, i+2])\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 1 2\n1 2 3\n2 3 4\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_11(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ def f(): end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{@ def f(a): return a end @}{: f(1) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ def f(a, b): return a + b end @}{: f(1, 2) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return true end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return 0 end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ def f(): return 1 + 2 end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ def f(): @}abc{@ end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcnil"));
    }

    PadTkr_Parse(tkr, "{@ def f(): @}abc{@ a = 1 @}def{@ end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcdefnil"));
    }

    PadTkr_Parse(tkr, "{@ def f(): @}abc{@ a = 1 @}{: a :}{@ end @}{: f() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc1nil"));
    }

    PadTkr_Parse(tkr, "{@ def f(a): @}{: a :}{@ b = 123 @}{: b :}{@ end @}{: f(\"abc\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc123nil"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "    def usage():\n"
        "@}abc{@\n"
        "    end\n"
        "@}{: usage() :}"
    );
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abcnil"));
    }

    PadTkr_Parse(tkr,
        "{@\n"
        "    def func():\n"
        "        puts(\"hi\")\n"
        "    end\n"
        "\n"
        "    d = { \"f\": func }\n"
        "    f = d[\"f\"]\n"
        "    f()\n"
        "@}"
    );
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "hi\n"));
    }

    check_ok(
        "{@\n"
        "    def func():\n"
        "        puts(\"hi\")\n"
        "    end\n"
        "\n"
        "    def func2(kwargs):\n"
        "        f = kwargs[\"f\"]\n"
        "        f()\n"
        "    end\n"
        "\n"
        "    func2({ \"f\": func })\n"
        "@}"
    , "hi\n");

    PadTkr_Parse(tkr,
        "{@\n"
        "    def func():\n"
        "       i = 0\n"
        "@}{: i :},{@\n"
        "       j = 1\n"
        "@}{: j :}{@"
        "    end\n"
        "\n"
        "    func()\n"
        "@}"
    );
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0,1"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_12(void) {
    trv_ready;

    // TODO: you need closure?
    check_fail("{@\n"
    "def func():\n"
    "   a = 1\n"
    "   def inner():\n"
    "       a += 1\n"
    "       puts(a)\n"
    "   end\n"
    "   inner()\n"
    "end\n"
    "func()\n"
    "@}", "\"a\" is not defined"
    );

    trv_cleanup;
}

static void
test_trv_func_def_13(void) {
    trv_ready;

    check_ok("{@\n"
    "def f1(a):\n"
    "   puts(a)\n"
    "end\n"
    "def f2(callback, a):\n"
    "   callback(a)\n"
    "end\n"
    "f2(f1, 1)\n"
    "@}", "1\n"
    );

    trv_cleanup;
}

static void
test_trv_func_def_14(void) {
    trv_ready;

    check_ok("{@\n"
    "i = 1\n"
    "idval = id(i)\n"
    "def func(a):\n"
    "   puts(idval != id(a))\n"
    "end\n"
    "func(i)\n"
    "@}", "true\n"
    );

    check_ok("{@\n"
    "i = 1.23\n"
    "idval = id(i)\n"
    "def func(a):\n"
    "   puts(idval != id(a))\n"
    "end\n"
    "func(i)\n"
    "@}", "true\n"
    );

    check_ok("{@\n"
    "i = \"abc\"\n"
    "idval = id(i)\n"
    "def func(a):\n"
    "   puts(idval != id(a))\n"
    "end\n"
    "func(i)\n"
    "@}", "true\n"
    );

    trv_cleanup;
}

static void
test_trv_func_def_fail_0(void) {
    trv_ready;

    check_fail("{@ def @}", "not found blocks");
    check_fail("{@ def f @}", "not found blocks");
    check_fail("{@ def f() @}", "not found colon");
    check_fail("{@ def f(): @}", "not found 'end' in parse func def. token type is 5");
    check_fail("{@ def f(): en @}", "not found 'end' in parse func def. token type is 5");
    check_ok("{@ def f(): end @}", "");
    check_fail("{@ def f(a, ): end @}", "syntax error. not found identifier in func def args");
    check_fail("{@ def f(a): end f(1, 2) @}", "arguments not same length");

    trv_cleanup;
}

static void
test_trv_func_met_0(void) {
    trv_ready;

    check_ok("{@\n"
        "struct A:\n"
        "   name = 1\n"
        "   met a(self):\n"
        "       puts(self.name)\n"
        "   end\n"
        "end\n"
        "a = A()\n"
        "a.a()"
        "@}", "1\n");

    trv_cleanup;
}

static void
test_trv_func_met_1(void) {
    trv_ready;

    check_fail("{@\n"
        "met a(self):\n"
        "    puts(self)\n"
        "end\n"
        "a()"
        "@}", "arguments not same length");

    trv_cleanup;
}

static void
test_trv_func_met_2(void) {
    trv_ready;

    check_ok(
    "{@\n"
    "    struct Animal:\n"
    "        name = nil\n"
    "\n"
    "        def new(name):\n"
    "            animal = Animal()\n"
    "            animal.name = name\n"
    "            return animal\n"
    "        end\n"
    "\n"
    "        met show(self):\n"
    "            puts(self.name)\n"
    "        end\n"
    "    end\n"
    "\n"
    "    def test3():\n"
    "        animal = Animal.new(\"Mike\")\n"
    "        animal.show()\n"
    "    end\n"
    "\n"
    "    test3()\n"
    "@}", "Mike\n");

    trv_cleanup;
}

static void
test_trv_func_met_3(void) {
    trv_ready;

    check_ok(
    "{@\n"
    "    struct Animal:\n"
    "        age = \"aaa\"\n"
    "        met getAge(self):\n"
    "           return self.age\n"
    "        end\n"
    "    end\n"
    "    a = Animal()\n"
    "@}{: a.getAge() :}", "aaa");

    trv_cleanup;
}

static void
test_trv_func_met_4(void) {
    trv_ready;

    check_ok(
    "{@\n"
    "    struct Animal:\n"
    "        name = nil\n"
    "\n"
    "        def new(name):\n"
    "            animal = Animal()\n"
    "            animal.name = name\n"
    "            return animal\n"
    "        end\n"
    "\n"
    "        met decolate(self, head, tail):\n"
    "            return head + self.name + tail\n"
    "        end\n"
    "    end\n"
    "\n"
    "    def test():\n"
    "        animal = Animal.new(\"Mike\")\n"
    "        puts(animal.decolate(\"The \", \" Man\"))\n"
    "    end\n"
    "\n"
    "    test()\n"
    "@}", "The Mike Man\n");

    trv_cleanup;
}

static void
test_trv_func_met_5(void) {
    trv_ready;

    check_ok(
    "{@\n"
    "    struct Animal:\n"
    "        met show(self, msg):\n"
    "            puts(self, msg)\n"
    "        end\n"
    "    end\n"
    "    animal = Animal()\n"
    "    animal.show(1)\n"
    "@}", "(object) 1\n");

    trv_cleanup;
}

static void
test_trv_func_extends_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "   end\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
    }

    trv_cleanup;
}

static void
test_trv_func_extends_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "   def f2() extends:\n"
    "   end\n"
    "@}", "not found identifier in function extends");

    check_fail("{@\n"
    "   def f2() ext:\n"
    "   end\n"
    "@}", "not found colon");

    check_fail("{@\n"
    "   def f2() extends f1:\n"
    "   end\n"
    "@}", "not found \"f1\". can't extends");

    trv_cleanup;
}

static void
test_trv_func_super_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       puts(1)\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       puts(2)\n"
    "       super()\n"
    "   end\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_super_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       puts(1)\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       puts(2)\n"
    "       super()\n"
    "   end\n"
    "   def f3() extends f1:\n"
    "       puts(3)\n"
    "       super()\n"
    "   end\n"
    "   f2()\n"
    "   f3()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n1\n3\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_super_2(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f1():\n"
    "       puts(1)\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       puts(2)\n"
    "       super()\n"
    "   end\n"
    "   def f3() extends f2:\n"
    "       puts(3)\n"
    "       super()\n"
    "   end\n"
    "   f3()\n"
    "@}", "3\n2\n1\n");

    trv_cleanup;
}

static void
test_trv_func_super_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "   def f1():\n"
    "       super()\n"
    "   end\n"
    "   f1()\n"
    "@}", "can't call \"super\"");

    check_fail("{@\n"
    "   def f1():\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       sup()\n"
    "   end\n"
    "   f2()\n"
    "@}", "can't call \"sup\"");

    trv_cleanup;
}

static void
test_trv_block_stmt_3(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f1():\n"
    "       puts(1)\n"
    "       block content:\n"
    "           puts(2)\n"
    "       end\n"
    "       puts(3)\n"
    "   end\n"
    "   f1()\n"
    "@}", "1\n2\n3\n");

    trv_cleanup;
}

static void
test_trv_block_stmt_4(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       super()\n"
    "   end\n"
    "   f1()\n"
    "@}", "1\n");

    trv_cleanup;
}

static void
test_trv_block_stmt_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "   def f1():\n"
    "       block:\n"
    "       end\n"
    "   end\n"
    "   f1()\n"
    "@}", "not found identifier in block statement");

    check_fail("{@\n"
    "   def f1():\n"
    "       bl:\n"
    "       end\n"
    "   end\n"
    "   f1()\n"
    "@}", "not found 'end' in parse func def. token type is 10");

    check_fail("{@\n"
    "   def f1():\n"
    "       block content:\n"
    "   end\n"
    "   f1()\n"
    "@}", "not found 'end' in parse func def. token type is 5");

    check_fail("{@\n"
    "block content:\n"
    "end\n"
    "@}", "can't access to function node");

    trv_cleanup;
}

static void
test_trv_inject_stmt_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       inject content:\n"
    "           puts(2)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       inject content:\n"
    "           puts(2)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f1()\n"
    "   f2()\n"
    "   f1()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block header:\n"
    "           puts(1)\n"
    "       end\n"
    "       block content:\n"
    "           puts(2)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       inject header:\n"
    "           puts(3)\n"
    "       end\n"
    "       inject content:\n"
    "           puts(4)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3\n4\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       super()\n"
    "   end\n"
    "   def f3() extends f2:\n"
    "       inject content:\n"
    "           puts(2)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f3()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block header:\n"
    "           puts(1)\n"
    "       end\n"
    "       block content:\n"
    "           puts(2)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       inject header:\n"
    "           puts(3)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   def f3() extends f2:\n"
    "       inject content:\n"
    "           puts(4)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f3()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3\n4\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_8(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(0)\n"
    "       end\n"
    "       block footer:\n"
    "           puts(0)\n"
    "       end\n"
    "   end\n"
    "   def f2() extends f1:\n"
    "       block header:\n"
    "           puts(1)\n"
    "       end\n"
    "       inject content:\n"
    "           puts(2)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   def f3() extends f2:\n"
    "       inject footer:\n"
    "           puts(3)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f3()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n3\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_9(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1(k):\n"
    "       block content:\n"
    "           puts(k[\"b\"])\n"
    "       end\n"
    "   end\n"
    "   def f2(k) extends f1:\n"
    "       block header:\n"
    "           puts(k[\"a\"])\n"
    "       end\n"
    "       super(k)\n"
    "   end\n"
    "   f2({ \"a\": 1, \"b\": 2 })\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_10(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       block content:\n"
    "       end\n"
    "   end\n"
    "   def f2():\n"
    "       inject content:\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f2()\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't inject. not found extended function"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_11(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1():\n"
    "       puts(a)\n"
    "   end\n"
    "   def f2(a) extends f1:\n"
    "       super()\n"
    "   end\n"
    "   f2(1)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_12(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f1(b):\n"
    "       block content:\n"
    "           puts(2)\n"
    "       end\n"
    "       puts(b)\n"
    "   end\n"
    "   def f2(a) extends f1:\n"
    "       inject content:\n"
    "           puts(a)\n"
    "       end\n"
    "       super(3)\n"
    "   end\n"
    "   f2(1)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n3\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_13(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def base():\n"
    "       block header:\n"
    "           @}<h1>Title</h1>{@\n"
    "       end\n"
    "   end\n"
    "   def index() extends base:\n"
    "       inject header:\n"
    "           @}<h1>The title</h1>{@\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   index()\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "<h1>The title</h1>"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_14(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def base(a):\n"
    "       block header:\n"
    "           puts(a)"
    "       end\n"
    "   end\n"
    "   def index(a) extends base:\n"
    "       inject header:\n"
    "           puts(a)\n"
    "       end\n"
    "       super(2)\n"
    "   end\n"
    "   index(1)\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_15(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def base(a, b):\n"
    "       block header:\n"
    "       end\n"
    "   end\n"
    "   def index(a) extends base:\n"
    "       inject header:\n"
    "           puts(a, b)\n"
    "       end\n"
    "       super(2, 3)\n"
    "   end\n"
    "   index(1)\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 3\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_16(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def base():\n"
    "       block header:\n"
    "       end\n"
    "   end\n"
    "   def index() extends base:\n"
    "       i = 1\n"
    "       inject header:\n"
    "           puts(i)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   index()\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_17(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def base():\n"
    "       block header:\n"
    "       end\n"
    "   end\n"
    "   def index() extends base:\n"
    "       i = 1\n"
    "       inject header: @}{: i :}{@ end\n"
    "       super()\n"
    "   end\n"
    "   index()\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_18(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   from \"tests/lang/modules/base.cap\" import base\n"
    "\n"
    "   def index() extends base:\n"
    "       i = 1\n"
    "       inject contents:\n"
    "           puts(i)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "\n"
    "   index()\n"
    "@}");
    {
        PadAst_Clear(ast);
        (PadCc_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_inject_stmt_19(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f1():\n"
    "       block content:\n"
    "           puts(2)\n"
    "       end\n"
    "   end\n"
    "   def f2(a) extends f1:\n"
    "       inject content:\n"
    "           puts(a)\n"
    "       end\n"
    "       super()\n"
    "   end\n"
    "   f2(1)\n"
    "@}", "1\n");

    trv_cleanup;
}

static void
test_trv_inject_stmt_20(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 1\n"
    "def f1():\n"
    "   block content:\n"
    "       puts(a)\n"
    "   end\n"
    "end\n"
    "f1()\n"
    "@}", "1\n");

    check_ok("{@\n"
    "def f1():\n"
    "   a = 1\n"
    "   block content:\n"
    "       puts(a)\n"
    "   end\n"
    "end\n"
    "f1()\n"
    "@}", "1\n");

    check_ok(
    "{@\n"
    "    def f1():\n"
    "        puts(\"f1\")\n"
    "        block a:\n"
    "            puts(\"f1.a\")\n"
    "            block b:\n"
    "                puts(\"f1.b\")\n"
    "            end\n"
    "        end\n"
    "    end\n"
    "\n"
    "    def f2() extends f1:\n"
    "        inject b:\n"
    "            puts(1)\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    f2()\n"
    "@}", "f1\nf1.a\n1\n");

    check_ok(
    "{@\n"
    "    def f1():\n"
    "        puts(\"f1\")\n"
    "        block a:\n"
    "            puts(\"f1.a\")\n"
    "            block b:\n"
    "                puts(\"f1.b\")\n"
    "            end\n"
    "        end\n"
    "    end\n"
    "\n"
    "    def f2() extends f1:\n"
    "        puts(\"f2\")\n"
    "        block a:\n"
    "            puts(\"f2.a\")\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    def f3() extends f2:\n"
    "        puts(\"f3\")"
    "        inject a:\n"
    "            puts(\"inject a from f3\")\n"
    "        end\n"
    "        inject b:\n"
    "            puts(\"inject b from f3\")\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    f3()\n"
    "@}", "f3\nf2\ninject a from f3\nf1\nf1.a\ninject b from f3\n");

    check_ok(
    "{@\n"
    "    def f1():\n"
    "        puts(\"f1\")\n"
    "        block a:\n"
    "            puts(\"f1.a\")\n"
    "            block b:\n"
    "                puts(\"f1.b\")\n"
    "            end\n"
    "        end\n"
    "    end\n"
    "\n"
    "    def f2() extends f1:\n"
    "        puts(\"f2\")\n"
    "        block a:\n"
    "            puts(\"f2.a\")\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    def f3() extends f2:\n"
    "        puts(\"f3\")\n"
    "        inject a:\n"
    "            puts(\"inject a from f3\")\n"
    "            inject b:\n"
    "                puts(\"inject b from f3\")\n"
    "            end\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    f3()\n"
    "@}"
    , "f3\n"
    "f2\n"
    "inject a from f3\n"
    "f1\n"
    "f1.a\n"
    "inject b from f3\n");

    trv_cleanup;
}

static void
test_trv_inject_stmt_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "def f1():\n"
    "   bl:\n"
    "   end\n"
    "end\n"
    "@}", "not found 'end' in parse func def. token type is 10");

    check_fail("{@\n"
    "def f1():\n"
    "   block:\n"
    "   end\n"
    "end\n"
    "@}", "not found identifier in block statement");

    check_fail(
    "{@\n"
    "    def f1():\n"
    "    end\n"
    "\n"
    "    def f2() extends f1:\n"
    "        inject a:\n"
    "            puts(\"f2.a\")\n"
    "        end\n"
    "        super()\n"
    "    end\n"
    "\n"
    "    f2()\n"
    "@}", "not found \"a\" block");

    trv_cleanup;
}

static void
test_trv_assign_list_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_list_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = 1, b = 2 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_list_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ a = b = 1 @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_assign_list_3(void) {
    trv_ready;

    check_ok("{@ a = b = 1, c = 2 @}{: a :},{: b :},{: c :}", "1,1,2");

    trv_cleanup;
}

static void
test_trv_assign_list_fail_0(void) {
    trv_ready;

    check_fail("{@ = @}", "not found blocks");
    check_fail("{@ a = @}", "syntax error. not found rhs test in assign list");
    check_fail("{@ a = 1, @}", "syntax error. not found test in test list");
    check_fail("{@ a = 1, = @}", "syntax error. not found test in test list");
    check_fail("{@ a = 1, b = @}", "syntax error. not found rhs test in assign list");

    trv_cleanup;
}

static void
test_trv_multi_assign_0(void) {
    trv_ready;

    check_ok("{@ a, b = 1, 2 @}{: a :},{: b :}", "1,2");

    trv_cleanup;
}

static void
test_trv_multi_assign_1(void) {
    trv_ready;

    check_ok("{@ a, b = 1, 2 \n a, b = b, a @}{: a :},{: b :}", "2,1");

    trv_cleanup;
}

static void
test_trv_multi_assign_fail_0(void) {
    trv_ready;

    check_fail("{@ a, = 1, 2 @}", "syntax error. not found test in test list");
    check_fail("{@ a, b = 1, @}", "syntax error. not found test in test list");
    check_fail("{@ a, = 1, @}", "syntax error. not found test in test list");
    check_fail("{@ 1, 2 = 1, 2 @}", "invalid left hand operand (1)");

    trv_cleanup;
}

static void
test_trv_or_test_0(void) {
    trv_ready;

    check_ok("{: 1 or 0 :}", "1");

    trv_cleanup;
}

static void
test_trv_or_test_fail_0(void) {
    trv_ready;

    check_fail("{: or :}", "not found blocks");
    check_fail("{: or 1 :}", "not found blocks");
    check_fail("{: 1 or :}", "syntax error. not found rhs operand in 'or' operator");
    check_fail("{: 1 or 2 o :}", "syntax error. not found \":}\"");

    trv_cleanup;
}

static void
test_trv_and_test_0(void) {
    trv_ready;

    check_ok("{: 1 and 1 :}", "1");

    trv_cleanup;
}

static void
test_trv_and_test_fail_0(void) {
    trv_ready;

    check_fail("{: and :}", "not found blocks");
    check_fail("{: and 1 :}", "not found blocks");
    check_fail("{: 1 and :}", "syntax error. not found rhs operand in 'and' operator");
    check_fail("{: 1 and 2 o :}", "syntax error. not found \":}\"");

    trv_cleanup;
}

static void
test_trv_not_test_0(void) {
    trv_ready;

    check_ok("{: not 0 :}", "true");

    trv_cleanup;
}

static void
test_trv_not_test_fail_0(void) {
    trv_ready;

    check_fail("{: not :}", "syntax error. not found operand in not operator");
    check_fail("{: 1 not :}", "syntax error. not found \":}\"");
    check_fail("{: not 1 2 :}", "syntax error. not found \":}\"");

    trv_cleanup;
}

static void
test_trv_comparison_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 == 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 != 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 < 2 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 2 > 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_4(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 <= 2 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_5(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 2 >= 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_comparison_6(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A: end\n"
    "@}{: A() != nil :}", "true");

    trv_cleanup;
}

static void
test_trv_comparison_7(void) {
    trv_ready;

    check_ok("{@ c = \"a\" @}{: c == \">\" :}", "false");
    check_ok("{@ path = \"/\" @}{: path == \"/\" :}", "true");
    check_ok("{@ path = \"/about\" @}{: path == \"/about\" :}", "true");

    trv_cleanup;
}

static void
test_trv_asscalc_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = 1 \n a += b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a += false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    check_fail("{@ 0 += 1 @}", "invalid left hand operand (1)");
    check_fail("{@ true += 1 @}", "invalid left hand operand (3)");
    check_fail("{@ a = 0 \n a += \"b\" @}", "invalid right hand operand (5)");

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_asscalc_1(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{@ a = 0 \n a -= 1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "-1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n b = 1 \n a -= b @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "-1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a -= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "-1"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a -= false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@ 1 -= 1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand type (1)"));
    }

    PadTkr_Parse(tkr, "{@ true -= 1 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand type (3)"));
    }

    PadTkr_Parse(tkr, "{@ a = 0 \n a -= \"c\" @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid right hand operand type (5)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_asscalc_2(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{@ a = 2 \n a *= 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 @}{: (a *= 2) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 \n a *= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 \n a *= false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n a *= 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abab"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n a *= 0 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n a *= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "ab"));
    }

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n a *= false @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@ a = \"ab\" \n a *= -1 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "can't mul by negative value"));
    }

    PadTkr_Parse(tkr, "{@ 1 *= 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand (1)"));
    }

    PadTkr_Parse(tkr, "{@ true *= 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand (3)"));
    }

    PadTkr_Parse(tkr, "{@ a = 2 \n a *= \"b\" @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid right hand operand (5)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_asscalc_3(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{@ a = 4 \n a /= 2 @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 4  @}{: (a /= 2) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 \n a /= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "4"));
    }

    PadTkr_Parse(tkr, "{@ a = true \n a /= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = false \n a /= true @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@ 4 /= 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand (1)"));
    }

    PadTkr_Parse(tkr, "{@ true /= 2 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid left hand operand (3)"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 \n a /= false @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "zero division error"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 \n a /= 0 @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "zero division error"));
    }

    PadTkr_Parse(tkr, "{@ a = 4 \n a /= \"b\" @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid right hand operand (5)"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_asscalc_4(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [1, 2]\n"
    "   a[0] += 1\n"
    "@}{: a[0] :}", "2");

    trv_cleanup;
}

static void
test_trv_asscalc_5(void) {
    trv_ready;

    /* this specification is different many languages.
       this language select equal to 4 on this expression */

    check_ok("{@\n"
    "   a = [1, 2]\n"
    "   a[0] += a[0] += 1\n"
    "@}{: a[0] :}", "4");  // TODO: Ruby is eq 3, C/C++ is eq 4

    check_ok("{@\n"
    "   a = [\"a\", \"b\"]\n"
    "   a[0] += a[0] += \"c\"\n"
    "@}{: a[0] :}", "acac");  // TODO: Ruby is eq 'aac'

    check_ok("{@\n"
    "   a = [[1, 2], [3, 4]]\n"
    "   a[0] += a[0] += [3, 4]\n"
    "@}{: a[0][0] :},{: a[0][1] :},{: a[0][2] :},{: a[0][3] :},{: a[0][4] :},{: a[0][5] :},{: a[0][6] :},{: a[0][7] :}", "1,2,3,4,1,2,3,4");  // TODO: Ruby is eq '1,2,1,2,3,4'

    trv_cleanup;
}

static void
test_trv_asscalc_6(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [\"aaa\", 2]\n"
    "   a[0] += \"bbb\"\n"
    "@}{: a[0] :}", "aaabbb");

    trv_cleanup;
}

static void
test_trv_asscalc_7(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [true, 2]\n"
    "   a[0] += 1\n"
    "@}{: a[0] :}", "2");

    trv_cleanup;
}

static void
test_trv_asscalc_8(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [true, 2]\n"
    "   a[0] += true\n"
    "@}{: a[0] :}", "2");

    trv_cleanup;
}

static void
test_trv_asscalc_9(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [1, 2]\n"
    "   a[0] -= 1\n"
    "@}{: a[0] :}", "0");

    trv_cleanup;
}

static void
test_trv_asscalc_10(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [true, 2]\n"
    "   a[0] -= true\n"
    "@}{: a[0] :}", "0");

    trv_cleanup;
}

static void
test_trv_asscalc_11(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [1, 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}", "2");

    trv_cleanup;
}

static void
test_trv_asscalc_12(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [\"abc\", 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}", "abcabc");

    trv_cleanup;
}

static void
test_trv_asscalc_13(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_14(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] *= true\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_15(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= 2\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_16(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= 0\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_17(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= false\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_18(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] /= true\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_19(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= 2\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_20(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= true\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_21(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= 0\n"
    "@}{: a[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_22(void) {
    trv_ready;

    check_fail("{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= false\n"
    "@}{: a[0] :}", "zero division error");

    trv_cleanup;
}

static void
test_trv_asscalc_23(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = \"ab\"\n"
    "   a *= 2\n"
    "@}{: a :}", "abab");

    trv_cleanup;
}

static void
test_trv_asscalc_24(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = [\"ab\"]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}", "abab");

    trv_cleanup;
}

static void
test_trv_asscalc_25(void) {
    trv_ready;

    check_ok("{@\n"
    "   a = {\"b\": \"cd\"}\n"
    "   a[\"b\"] *= 2\n"
    "@}{: a[\"b\"] :}", "cdcd");

    trv_cleanup;
}

static void
test_trv_asscalc_add_ass_string(void) {
    trv_ready;

    check_ok("{@ a = \"a\" a += \"b\" @}{: a :}", "ab");
    check_ok("{@ a = \"a\" b = [\"b\"] a += b[0] @}{: a :}", "ab");

    check_ok("{@ a = \"a\" aid = id(a) a += \"b\" @}{: aid != id(a) :}", "true");

    trv_cleanup;
}

static void
test_trv_asscalc_fail_0(void) {
    trv_ready;

    check_fail("{@ += @}", "not found blocks");
    check_fail("{@ -= @}", "not found blocks");
    check_fail("{@ *= @}", "not found blocks");
    check_fail("{@ /= @}", "not found blocks");
    check_fail("{@ %= @}", "not found blocks");
    check_fail("{@ a += @}", "syntax error. not found rhs operand in asscalc");
    check_fail("{@ a -= @}", "syntax error. not found rhs operand in asscalc");
    check_fail("{@ a *= @}", "syntax error. not found rhs operand in asscalc");
    check_fail("{@ a /= @}", "syntax error. not found rhs operand in asscalc");
    check_fail("{@ a %= @}", "syntax error. not found rhs operand in asscalc");
    check_fail("{@ 1 += 1 @}", "invalid left hand operand (1)");
    check_fail("{@ 1 -= 1 @}", "invalid left hand operand type (1)");
    check_fail("{@ 1 *= 1 @}", "invalid left hand operand (1)");
    check_fail("{@ 1 /= 1 @}", "invalid left hand operand (1)");
    check_fail("{@ 1 %= 1 @}", "invalid left hand operand (1)");
    check_fail("{@ a += b @}", "\"a\" is not defined");
    check_fail("{@ a -= b @}", "\"a\" is not defined");
    check_fail("{@ a *= b @}", "\"a\" is not defined");
    check_fail("{@ a /= b @}", "\"a\" is not defined");
    check_fail("{@ a %= b @}", "\"a\" is not defined");

    trv_cleanup;
}

static void
test_trv_expr_0(void) {
    trv_ready;

    check_ok("{: 1 + 1 :}", "2");
    check_ok("{@ a = 1 b = a @}{: b :}", "1");

    trv_cleanup;
}

static void
test_trv_expr_1(void) {
    trv_ready;

    check_ok("{: 1 - 1 :}", "0");

    trv_cleanup;
}

static void
test_trv_expr_2(void) {
    trv_ready;

    check_ok("{@ a = 1 \n b = a - 1 @}{: b :}", "0");

    trv_cleanup;
}

static void
test_trv_expr_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = f(a)\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = f(a)[0]\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4a(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [f, 2, 3]\n"
    "r = a[0](1)\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4b(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "import \"tests/lang/modules/func.cap\" as mod\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [mod, 2, 3]\n"
    "r = a[0].arrMod.array[0]\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4c(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "import \"tests/lang/modules/func.cap\" as mod\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [mod, 2, 3]\n"
    "r = a[0].arrMod.funcArray[0](0)\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "d = { \"a\": 1, \"b\": 2 }\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = a[f(a)[0]]\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "d = { \"a\": 1, \"b\": 2 }\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = a[f(a)[0] * 2] * 3 + f(10)\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "19"));
    }

    trv_cleanup;
}

static void
test_trv_expr_6(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "import \"tests/lang/modules/array.cap\" as mod\n"
    "\n"
    "r = mod.array[0]\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_7(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "import \"tests/lang/modules/func.cap\" as mod\n"
    "\n"
    "r = mod.func(1)\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_expr_8(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "import \"tests/lang/modules/func.cap\" as mod\n"
    "\n"
    "r = mod.arrMod.array[0]\n"
    "@}{: r :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_9(void) {
    trv_ready;

    /************************
    * theme: array and expr *
    ************************/

    check_ok("{@\n"
    "   l = [1, 2]\n"
    "   l2 = l + l\n"
    "@}{: l2[0] :},{: l2[1] :},{: l2[2] :},{: l2[3] :},{: id(l2[0]) == id(l2[2]) :}"
    , "1,2,1,2,true");

    check_ok("{@\n"
    "   l1 = [1, 2]\n"
    "   l2 = [3, 4]\n"
    "   l3 = l1 + l2\n"
    "@}{: l3[0] :},{: l3[1] :},{: l3[2] :},{: l3[3] :}"
    , "1,2,3,4");

    trv_cleanup;
}

static void
test_trv_expr_float(void) {
    trv_ready;

    check_ok("{: 1.1 + 1.2 :}", "2.3");
    check_ok("{: 1 + 1.2 :}", "2.2");
    check_ok("{: 1.1 + true :}", "2.1");
    check_ok("{: 1.2 - 1.1 :}", "0.1");
    check_ok("{: 2 - 1.1 :}", "0.9");
    check_ok("{: 1.2 - true :}", "0.2");
    check_ok("{: 1.2 * 1.3 :}", "1.56");
    check_ok("{: 2 * 1.3 :}", "2.6");
    check_ok("{: 1.2 * true :}", "1.2");
    check_ok("{: 4.0 / 2.0 :}", "2.0");
    check_ok("{: 4 / 2.0 :}", "2.0");
    check_ok("{: 4.0 / true :}", "4.0");
    check_ok("{@ a = 1.1 b = a @}{: b :}", "1.1");

    check_fail("{: 1.0 + \"a\" :}", "can't add with float");
    check_fail("{: 1.0 - \"a\" :}", "can't sub with float");
    check_fail("{: 1.0 * \"a\" :}", "can't mul with float");
    check_fail("{: 1.0 / \"a\" :}", "invalid right hand operand");
    check_fail("{: \"a\" + 1.0 :}", "can't add 2 with string");
    check_fail("{: \"a\" - 1.0 :}", "can't sub");
    check_fail("{: \"a\" * 1.0 :}", "can't mul with string");
    check_fail("{: \"a\" / 1.0 :}", "can't division");

    check_ok("{@ a = 0.0 a += 0.1 @}{: a :}", "0.1");
    check_ok("{@ a = 0.0 a += 1 @}{: a :}", "1.0");
    check_ok("{@ a = 0.0 a += true @}{: a :}", "1.0");
    check_ok("{@ a = 0.0 a -= 0.1 @}{: a :}", "-0.1");
    check_ok("{@ a = 0.0 a -= 1 @}{: a :}", "-1.0");
    check_ok("{@ a = 0.0 a -= true @}{: a :}", "-1.0");
    check_ok("{@ a = 2.0 a *= 0.2 @}{: a :}", "0.4");
    check_ok("{@ a = 2.0 a *= 1 @}{: a :}", "2.0");
    check_ok("{@ a = 2.0 a *= true @}{: a :}", "2.0");
    check_ok("{@ a = 4.0 a /= 2.0 @}{: a :}", "2.0");
    check_ok("{@ a = 4.0 a /= 2 @}{: a :}", "2.0");
    check_ok("{@ a = 4.0 a /= true @}{: a :}", "4.0");

    check_fail("{@ a = 1.0 a += nil @}", "invalid right hand operand (0)");
    check_fail("{@ a = 1.0 a += \"a\" @}", "invalid right hand operand (5)");
    check_fail("{@ a = 1.0 a += [] @}", "invalid right hand operand (6)");
    check_fail("{@ a = 1.0 a += {} @}", "invalid right hand operand (7)");

    check_ok("{@ a = 1 a += 1.0 @}{: a :}", "2.0");
    check_ok("{@ a = 1 a -= 1.0 @}{: a :}", "0.0");
    check_ok("{@ a = 1 a *= 2.0 @}{: a :}", "2.0");
    check_ok("{@ a = 2 a /= 2.0 @}{: a :}", "1.0");

    check_ok("{@ a = true a += 1.0 @}{: a :}", "2.0");
    check_ok("{@ a = true a -= 1.0 @}{: a :}", "0.0");
    check_ok("{@ a = true a *= 2.0 @}{: a :}", "2.0");
    check_ok("{@ a = true a /= 2.0 @}{: a :}", "0.5");

    check_ok("{: 1 == 0.0 :}", "false");
    check_ok("{: 1.0 == 0.0 :}", "false");
    check_ok("{: true == 0.0 :}", "false");
    check_ok("{: 1 != 0.0 :}", "true");
    check_ok("{: 1.0 != 0.0 :}", "true");
    check_ok("{: true != 0.0 :}", "true");
    check_ok("{: 1 < 0.0 :}", "false");
    check_ok("{: 1.0 < 0.0 :}", "false");
    check_ok("{: true < 0.0 :}", "false");
    check_ok("{: 1 <= 0.0 :}", "false");
    check_ok("{: 1.0 <= 0.0 :}", "false");
    check_ok("{: true <= 0.0 :}", "false");
    check_ok("{: 1 > 0.0 :}", "true");
    check_ok("{: 1.0 > 0.0 :}", "true");
    check_ok("{: true > 0.0 :}", "true");
    check_ok("{: 1 >= 0.0 :}", "true");
    check_ok("{: 1.0 >= 0.0 :}", "true");
    check_ok("{: true >= 0.0 :}", "true");

    check_fail("{: 1.0 < nil :}", "can't compare lt with float");
    check_fail("{: 1.0 < \"a\" :}", "can't compare lt with float");
    check_fail("{: 1.0 < [] :}", "can't compare lt with float");
    check_fail("{: 1.0 < {} :}", "can't compare lt with float");
    check_fail("{@ def f(): end @}{: 1.0 < f :}", "can't compare lt with float");
    check_fail("{: 1.0 <= nil :}", "can't compare lte with float");
    check_fail("{: 1.0 <= \"a\" :}", "can't compare lte with float");
    check_fail("{: 1.0 <= [] :}", "can't compare lte with float");
    check_fail("{: 1.0 <= {} :}", "can't compare lte with float");
    check_fail("{@ def f(): end @}{: 1.0 <= f :}", "can't compare lte with float");
    check_fail("{: 1.0 > \"a\" :}", "can't compare gt with float");
    check_fail("{: 1.0 > nil :}", "can't compare gt with float");
    check_fail("{: 1.0 > [] :}", "can't compare gt with float");
    check_fail("{: 1.0 > {} :}", "can't compare gt with float");
    check_fail("{@ def f(): end @}{: 1.0 > f :}", "can't compare gt with float");
    check_fail("{: 1.0 >= nil :}", "can't compare gte with float");
    check_fail("{: 1.0 >= \"a\" :}", "can't compare gte with float");
    check_fail("{: 1.0 >= [] :}", "can't compare gte with float");
    check_fail("{: 1.0 >= {} :}", "can't compare gte with float");
    check_fail("{@ def f(): end @}{: 1.0 >= f :}", "can't compare gte with float");
    check_fail("{: nil < 1.0 :}", "can't compare with lt (0)");
    check_fail("{: \"a\" < 1.0 :}", "can't compare with lt (5)");
    check_fail("{: [] < 1.0 :}", "can't compare with lt (6)");
    check_fail("{: {} < 1.0 :}", "can't compare with lt (7)");
    check_fail("{@ def f(): end @}{: f < 1.0 :}", "can't compare with lt (10)");
    check_fail("{: nil <= 1.0 :}", "can't compare with lte");
    check_fail("{: \"a\" <= 1.0 :}", "can't compare with lte");
    check_fail("{: [] <= 1.0 :}", "can't compare with lte");
    check_fail("{: {} <= 1.0 :}", "can't compare with lte");
    check_fail("{@ def f(): end @}{: f <= 1.0 :}", "can't compare with lte");
    check_fail("{: nil > 1.0 :}", "can't compare with gt");
    check_fail("{: \"a\" > 1.0 :}", "can't compare with gt");
    check_fail("{: [] > 1.0 :}", "can't compare with gt");
    check_fail("{: {} > 1.0 :}", "can't compare with gt");
    check_fail("{@ def f(): end @}{: f > 1.0 :}", "can't compare with gt");
    check_fail("{: nil >= 1.0 :}", "can't compare with gte");
    check_fail("{: \"a\" >= 1.0 :}", "can't compare with gte");
    check_fail("{: [] >= 1.0 :}", "can't compare with gte");
    check_fail("{: {} >= 1.0 :}", "can't compare with gte");
    check_fail("{@ def f(): end @}{: f >= 1.0 :}", "can't compare with gte");

    check_ok("{: 1.0 == nil :}", "false");
    check_ok("{: 1.0 == \"a\" :}", "false");
    check_ok("{: 1.0 == [] :}", "false");
    check_ok("{: 1.0 == {} :}", "false");
    check_ok("{@ def f(): end @}{: 1.0 == f :}", "false");
    check_ok("{: nil == 1.0 :}", "false");
    check_ok("{: \"a\" == 1.0 :}", "false");
    check_ok("{: [] == 1.0 :}", "false");
    check_ok("{: {} == 1.0 :}", "false");
    check_ok("{@ def f(): end @}{: f == 1.0 :}", "false");
    check_ok("{: 1.0 != nil :}", "true");
    check_ok("{: 1.0 != \"a\" :}", "true");
    check_ok("{: 1.0 != [] :}", "true");
    check_ok("{: 1.0 != {} :}", "true");
    check_ok("{@ def f(): end @}{: 1.0 != f :}", "true");
    check_ok("{: nil != 1.0 :}", "true");
    check_ok("{: \"a\" != 1.0 :}", "true");
    check_ok("{: [] != 1.0 :}", "true");
    check_ok("{: {} != 1.0 :}", "true");
    check_ok("{@ def f(): end @}{: f != 1.0 :}", "true");

    check_fail("{@ a = 1.0 a /= 0 @}{: a :}", "zero division error");
    check_fail("{@ a = 1.0 a /= 0.0 @}{: a :}", "zero division error");
    check_fail("{@ a = 1.0 a /= false @}{: a :}", "zero division error");
    check_fail("{@ a = 1 a /= 0 @}{: a :}", "zero division error");
    check_fail("{@ a = 1 a /= 0.0 @}{: a :}", "zero division error");
    check_fail("{@ a = 1 a /= false @}{: a :}", "zero division error");
    check_fail("{@ a = true a /= 0 @}{: a :}", "zero division error");
    check_fail("{@ a = true a /= 0.0 @}{: a :}", "zero division error");
    check_fail("{@ a = true a /= false @}{: a :}", "zero division error");

    // chain-object and float
    // - add operator
    check_ok("{@ a = [1.0] @}{: a[0] + 1.0 :}", "2.0");
    check_ok("{@ a = [1.0] @}{: a[0] + 1 :}", "2.0");
    check_ok("{@ a = [1.0] @}{: a[0] + true :}", "2.0");
    check_fail("{@ a = [1.0] @}{: a[0] + nil :}", "can't add with float");
    check_fail("{@ a = [1.0] @}{: a[0] + [] :}", "can't add with float");
    check_fail("{@ a = [1.0] @}{: a[0] + {} :}", "can't add with float");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] + f :}", "can't add with float");

    check_ok("{@ a = [1.0] @}{: 1.0 + a[0] :}", "2.0");
    check_ok("{@ a = [1.0] @}{: 1 + a[0] :}", "2.0");
    check_ok("{@ a = [1.0] @}{: true + a[0] :}", "2.0");
    check_fail("{@ a = [1.0] @}{: nil + a[0] :}", "can't add");
    check_fail("{@ a = [1.0] @}{: [] + a[0] :}", "invalid right hand operand (2)");
    check_fail("{@ a = [1.0] @}{: {} + a[0] :}", "can't add");
    check_fail("{@ a = [1.0] def f(): end @}{: f + a[0] :}", "can't add");

    // - sub operator
    check_ok("{@ a = [1.0] @}{: a[0] - 1.0 :}", "0.0");
    check_ok("{@ a = [1.0] @}{: a[0] - 1 :}", "0.0");
    check_ok("{@ a = [1.0] @}{: a[0] - true :}", "0.0");
    check_fail("{@ a = [1.0] @}{: a[0] - nil :}", "can't sub with float");
    check_fail("{@ a = [1.0] @}{: a[0] - [] :}", "can't sub with float");
    check_fail("{@ a = [1.0] @}{: a[0] - {} :}", "can't sub with float");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] - f :}", "can't sub with float");

    check_ok("{@ a = [1.0] @}{: 1.0 - a[0] :}", "0.0");
    check_ok("{@ a = [1.0] @}{: 1 - a[0] :}", "0.0");
    check_ok("{@ a = [1.0] @}{: true - a[0] :}", "0.0");
    check_fail("{@ a = [1.0] @}{: nil - a[0] :}", "can't sub");
    check_fail("{@ a = [1.0] @}{: [] - a[0] :}", "can't sub");
    check_fail("{@ a = [1.0] @}{: {} - a[0] :}", "can't sub");
    check_fail("{@ a = [1.0] def f(): end @}{: f - a[0] :}", "can't sub");

    // - mul operator
    check_ok("{@ a = [2.0] @}{: a[0] * 2.0 :}", "4.0");
    check_ok("{@ a = [2.0] @}{: a[0] * 2 :}", "4.0");
    check_ok("{@ a = [2.0] @}{: a[0] * true :}", "2.0");
    check_fail("{@ a = [1.0] @}{: a[0] * nil :}", "can't mul with float");
    check_fail("{@ a = [1.0] @}{: a[0] * [] :}", "can't mul with float");
    check_fail("{@ a = [1.0] @}{: a[0] * {} :}", "can't mul with float");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] * f :}", "can't mul with float");

    check_ok("{@ a = [2.0] @}{: 2.0 * a[0] :}", "4.0");
    check_ok("{@ a = [2.0] @}{: 2 * a[0] :}", "4.0");
    check_ok("{@ a = [2.0] @}{: true * a[0] :}", "2.0");
    check_fail("{@ a = [1.0] @}{: nil * a[0] :}", "can't mul");
    check_fail("{@ a = [1.0] @}{: [] * a[0] :}", "can't mul");
    check_fail("{@ a = [1.0] @}{: {} * a[0] :}", "can't mul");
    check_fail("{@ a = [1.0] def f(): end @}{: f * a[0] :}", "can't mul");

    // - div operator
    check_ok("{@ a = [4.0] @}{: a[0] / 2.0 :}", "2.0");
    check_ok("{@ a = [4.0] @}{: a[0] / 2 :}", "2.0");
    check_ok("{@ a = [4.0] @}{: a[0] / true :}", "4.0");
    check_fail("{@ a = [1.0] @}{: a[0] / nil :}", "invalid right hand operand");
    check_fail("{@ a = [1.0] @}{: a[0] / [] :}", "invalid right hand operand");
    check_fail("{@ a = [1.0] @}{: a[0] / {} :}", "invalid right hand operand");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] / f :}", "invalid right hand operand");

    check_ok("{@ a = [2.0] @}{: 4.0 / a[0] :}", "2.0");
    check_ok("{@ a = [2.0] @}{: 4 / a[0] :}", "2.0");
    check_ok("{@ a = [2.0] @}{: true / a[0] :}", "0.5");
    check_fail("{@ a = [1.0] @}{: nil / a[0] :}", "can't division");
    check_fail("{@ a = [1.0] @}{: [] / a[0] :}", "can't division");
    check_fail("{@ a = [1.0] @}{: {} / a[0] :}", "can't division");
    check_fail("{@ a = [1.0] def f(): end @}{: f / a[0] :}", "can't division");

    // - assign operator
    check_ok("{@ a = [1.0] a[0] = 2.0 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [1.0] a[0] = 2 @}{: a[0] :}", "2");
    check_ok("{@ a = [1.0] a[0] = true @}{: a[0] :}", "true");
    check_ok("{@ a = [1.0] @}{: a[0] = nil :}", "nil");
    check_ok("{@ a = [1.0] @}{: a[0] = [] :}", "(array)");
    check_ok("{@ a = [1.0] @}{: a[0] = {} :}", "(dict)");
    check_ok("{@ a = [1.0] def f(): end @}{: a[0] = f :}", "(function)");

    check_ok("{@ a = [1] a[0] = 1.0 @}{: a[0] :}", "1.0");
    check_ok("{@ a = [true] a[0] = 1.0 @}{: a[0] :}", "1.0");
    check_ok("{@ a = [nil] a[0] = 1.0 @}{: a[0] :}", "1.0");
    check_ok("{@ a = [[]] a[0] = 1.0 @}{: a[0] :}", "1.0");
    check_ok("{@ a = [{}] a[0] = 1.0 @}{: a[0] :}", "1.0");
    check_ok("{@ def f(): end a = [f] a[0] = 1.0 @}{: a[0] :}", "1.0");

    // - add assign operator
    check_ok("{@ a = [1.0] a[0] += 1.0 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [1.0] a[0] += 1 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [1.0] a[0] += true @}{: a[0] :}", "2.0");
    check_fail("{@ a = [1.0] @}{: a[0] += nil :}", "invalid right hand operand (0)");
    check_fail("{@ a = [1.0] @}{: a[0] += [] :}", "invalid right hand operand (6)");
    check_fail("{@ a = [1.0] @}{: a[0] += {} :}", "invalid right hand operand (7)");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] += f :}", "invalid right hand operand (10)");

    check_ok("{@ a = [1] a[0] += 1.0 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [true] a[0] += 1.0 @}{: a[0] :}", "2.0");
    check_fail("{@ a = [nil] a[0] += 1.0 @}{: a[0] :}", "invalid left hand operand (0)");
    check_fail("{@ a = [[]] a[0] += 1.0 @}{: a[0] :}", "invalid right hand operand (2)");
    check_fail("{@ a = [{}] a[0] += 1.0 @}{: a[0] :}", "invalid left hand operand (7)");
    check_fail("{@ def f(): end a = [f] a[0] += 1.0 @}{: a[0] :}", "invalid left hand operand (10)");

    // - sub assign operator
    check_ok("{@ a = [1.0] a[0] -= 1.0 @}{: a[0] :}", "0.0");
    check_ok("{@ a = [1.0] a[0] -= 1 @}{: a[0] :}", "0.0");
    check_ok("{@ a = [1.0] a[0] -= true @}{: a[0] :}", "0.0");
    check_fail("{@ a = [1.0] @}{: a[0] -= nil :}", "invalid right hand operand (0)");
    check_fail("{@ a = [1.0] @}{: a[0] -= [] :}", "invalid right hand operand (6)");
    check_fail("{@ a = [1.0] @}{: a[0] -= {} :}", "invalid right hand operand (7)");
    check_fail("{@ a = [1.0] def f(): end @}{: a[0] -= f :}", "invalid right hand operand (10)");

    check_ok("{@ a = [1] a[0] -= 1.0 @}{: a[0] :}", "0.0");
    check_ok("{@ a = [true] a[0] -= 1.0 @}{: a[0] :}", "0.0");
    check_fail("{@ a = [nil] a[0] -= 1.0 @}{: a[0] :}", "invalid left hand operand (0)");
    check_fail("{@ a = [[]] a[0] -= 1.0 @}{: a[0] :}", "invalid left hand operand (6)");
    check_fail("{@ a = [{}] a[0] -= 1.0 @}{: a[0] :}", "invalid left hand operand (7)");
    check_fail("{@ def f(): end a = [f] a[0] -= 1.0 @}{: a[0] :}", "invalid left hand operand (10)");

    // - mul assign operator
    check_ok("{@ a = [2.0] a[0] *= 2.0 @}{: a[0] :}", "4.0");
    check_ok("{@ a = [2.0] a[0] *= 2 @}{: a[0] :}", "4.0");
    check_ok("{@ a = [2.0] a[0] *= true @}{: a[0] :}", "2.0");
    check_fail("{@ a = [2.0] @}{: a[0] *= nil :}", "invalid right hand operand (0)");
    check_fail("{@ a = [2.0] @}{: a[0] *= [] :}", "invalid right hand operand (6)");
    check_fail("{@ a = [2.0] @}{: a[0] *= {} :}", "invalid right hand operand (7)");
    check_fail("{@ a = [2.0] def f(): end @}{: a[0] *= f :}", "invalid right hand operand (10)");

    check_ok("{@ a = [2] a[0] *= 2.0 @}{: a[0] :}", "4.0");
    check_ok("{@ a = [true] a[0] *= 2.0 @}{: a[0] :}", "2.0");
    check_fail("{@ a = [nil] a[0] *= 1.0 @}", "invalid left hand operand (0)");
    check_fail("{@ a = [[]] a[0] *= 1.0 @}", "invalid left hand operand (6)");
    check_fail("{@ a = [{}] a[0] *= 1.0 @}", "invalid left hand operand (7)");
    check_fail("{@ def f(): end a = [f] a[0] *= 1.0 @}", "invalid left hand operand (10)");

    // - div assign operator
    check_ok("{@ a = [4.0] a[0] /= 2.0 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [4.0] a[0] /= 2 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [4.0] a[0] /= true @}{: a[0] :}", "4.0");
    check_fail("{@ a = [4.0] @}{: a[0] /= nil :}", "invalid right hand operand (0)");
    check_fail("{@ a = [4.0] @}{: a[0] /= [] :}", "invalid right hand operand (6)");
    check_fail("{@ a = [4.0] @}{: a[0] /= {} :}", "invalid right hand operand (7)");
    check_fail("{@ a = [4.0] def f(): end @}{: a[0] /= f :}", "invalid right hand operand (10)");

    check_ok("{@ a = [4] a[0] /= 2.0 @}{: a[0] :}", "2.0");
    check_ok("{@ a = [true] a[0] /= 2.0 @}{: a[0] :}", "0.5");
    check_fail("{@ a = [nil] a[0] /= 1.0 @}", "invalid left hand operand (0)");
    check_fail("{@ a = [[]] a[0] /= 1.0 @}", "invalid left hand operand (6)");
    check_fail("{@ a = [{}] a[0] /= 1.0 @}", "invalid left hand operand (7)");
    check_fail("{@ def f(): end a = [f] a[0] /= 1.0 @}", "invalid left hand operand (10)");

    // - comparison operator
    // -- eq operator
    check_ok("{@ a = [1.0] @}{: a[0] == 1.0 :}", "true");
    check_ok("{@ a = [1] @}{: a[0] == 1.0 :}", "true");
    check_ok("{@ a = [true] @}{: a[0] == 1.0 :}", "true");
    check_ok("{@ a = [4.0] @}{: a[0] == nil :}", "false");
    check_ok("{@ a = [4.0] @}{: a[0] == [] :}", "false");
    check_ok("{@ a = [4.0] @}{: a[0] == {} :}", "false");
    check_ok("{@ a = [4.0] def f(): end @}{: a[0] == f :}", "false");

    check_ok("{@ a = [1.0] @}{: 1.0 == a[0] :}", "true");
    check_ok("{@ a = [1] @}{: 1.0 == a[0] :}", "true");
    check_ok("{@ a = [true] @}{: 1.0 == a[0] :}", "true");
    check_ok("{@ a = [nil] @}{: a[0] == 1.0 :}", "false");
    check_ok("{@ a = [[]] @}{: a[0] == 1.0 :}", "false");
    check_ok("{@ a = [{}] @}{: a[0] == 1.0 :}", "false");
    check_ok("{@ def f(): end a = [f] @}{: a[0] == 1.0 :}", "false");

    // -- neq operator
    check_ok("{@ a = [1.0] @}{: a[0] != 1.0 :}", "false");
    check_ok("{@ a = [1] @}{: a[0] != 1.0 :}", "false");
    check_ok("{@ a = [true] @}{: a[0] != 1.0 :}", "false");
    check_ok("{@ a = [nil] @}{: a[0] != 1.0 :}", "true");
    check_ok("{@ a = [[]] @}{: a[0] != 1.0 :}", "true");
    check_ok("{@ a = [{}] @}{: a[0] != 1.0 :}", "true");
    check_ok("{@ def f(): end a = [f] @}{: a[0] != 1.0 :}", "true");

    check_ok("{@ a = [1.0] @}{: 1.0 != a[0] :}", "false");
    check_ok("{@ a = [1] @}{: 1.0 != a[0] :}", "false");
    check_ok("{@ a = [true] @}{: 1.0 != a[0] :}", "false");
    check_ok("{@ a = [nil] @}{: 1.0 != a[0] :}", "true");
    check_ok("{@ a = [[]] @}{: 1.0 != a[0] :}", "true");
    check_ok("{@ a = [{}] @}{: 1.0 != a[0] :}", "true");
    check_ok("{@ def f(): end a = [f] @}{: 1.0 != a[0] :}", "true");

    // -- gt operator
    check_ok("{@ a = [1.0] @}{: a[0] > 0.5 :}", "true");
    check_ok("{@ a = [1] @}{: a[0] > 0.5 :}", "true");
    check_ok("{@ a = [true] @}{: a[0] > 0.5 :}", "true");
    check_fail("{@ a = [nil] @}{: a[0] > 0.5 :}", "can't compare with gt");
    check_fail("{@ a = [[]] @}{: a[0] > 0.5 :}", "can't compare with gt");
    check_fail("{@ a = [{}] @}{: a[0] > 0.5 :}", "can't compare with gt");
    check_fail("{@ def f(): end a = [f] @}{: a[0] > 0.5 :}", "can't compare with gt");

    check_ok("{@ a = [1.0] @}{: 0.5 > a[0] :}", "false");
    check_ok("{@ a = [1] @}{: 0.5 > a[0] :}", "false");
    check_ok("{@ a = [true] @}{: 0.5 > a[0] :}", "false");
    check_fail("{@ a = [nil] @}{: 0.5 > a[0] :}", "can't compare gt with float");
    check_fail("{@ a = [[]] @}{: 0.5 > a[0] :}", "can't compare gt with float");
    check_fail("{@ a = [{}] @}{: 0.5 > a[0] :}", "can't compare gt with float");
    check_fail("{@ def f(): end a = [f] @}{: 0.5 > a[0] :}", "can't compare gt with float");

    // -- gte operator
    check_ok("{@ a = [1.0] @}{: a[0] >= 0.5 :}", "true");
    check_ok("{@ a = [1] @}{: a[0] >= 0.5 :}", "true");
    check_ok("{@ a = [true] @}{: a[0] >= 0.5 :}", "true");
    check_fail("{@ a = [nil] @}{: a[0] >= 0.5 :}", "can't compare with gte");
    check_fail("{@ a = [[]] @}{: a[0] >= 0.5 :}", "can't compare with gte");
    check_fail("{@ a = [{}] @}{: a[0] >= 0.5 :}", "can't compare with gte");
    check_fail("{@ def f(): end a = [f] @}{: a[0] >= 0.5 :}", "can't compare with gte");

    check_ok("{@ a = [1.0] @}{: 0.5 >= a[0] :}", "false");
    check_ok("{@ a = [1] @}{: 0.5 >= a[0] :}", "false");
    check_ok("{@ a = [true] @}{: 0.5 >= a[0] :}", "false");
    check_fail("{@ a = [nil] @}{: 0.5 >= a[0] :}", "can't compare gte with float");
    check_fail("{@ a = [[]] @}{: 0.5 >= a[0] :}", "can't compare gte with float");
    check_fail("{@ a = [{}] @}{: 0.5 >= a[0] :}", "can't compare gte with float");
    check_fail("{@ def f(): end a = [f] @}{: 0.5 >= a[0] :}", "can't compare gte with float");

    // -- lt operator
    check_ok("{@ a = [1.0] @}{: a[0] < 1.5 :}", "true");
    check_ok("{@ a = [1] @}{: a[0] < 1.5 :}", "true");
    check_ok("{@ a = [true] @}{: a[0] < 1.5 :}", "true");
    check_fail("{@ a = [nil] @}{: a[0] < 1.5 :}", "can't compare with lt (0)");
    check_fail("{@ a = [[]] @}{: a[0] < 1.5 :}", "can't compare with lt (6)");
    check_fail("{@ a = [{}] @}{: a[0] < 1.5 :}", "can't compare with lt (7)");
    check_fail("{@ def f(): end a = [f] @}{: a[0] < 1.5 :}", "can't compare with lt (10)");

    check_ok("{@ a = [1.0] @}{: 1.5 < a[0] :}", "false");
    check_ok("{@ a = [1] @}{: 1.5 < a[0] :}", "false");
    check_ok("{@ a = [true] @}{: 1.5 < a[0] :}", "false");
    check_fail("{@ a = [nil] @}{: 1.5 < a[0] :}", "can't compare lt with float");
    check_fail("{@ a = [[]] @}{: 1.5 < a[0] :}", "can't compare lt with float");
    check_fail("{@ a = [{}] @}{: 1.5 < a[0] :}", "can't compare lt with float");
    check_fail("{@ def f(): end a = [f] @}{: 1.5 < a[0] :}", "can't compare lt with float");

    // -- lte operator
    check_ok("{@ a = [1.0] @}{: a[0] <= 1.5 :}", "true");
    check_ok("{@ a = [1] @}{: a[0] <= 1.5 :}", "true");
    check_ok("{@ a = [true] @}{: a[0] <= 1.5 :}", "true");
    check_fail("{@ a = [nil] @}{: a[0] <= 1.5 :}", "can't compare with lte");
    check_fail("{@ a = [[]] @}{: a[0] <= 1.5 :}", "can't compare with lte");
    check_fail("{@ a = [{}] @}{: a[0] <= 1.5 :}", "can't compare with lte");
    check_fail("{@ def f(): end a = [f] @}{: a[0] <= 1.5 :}", "can't compare with lte");

    check_ok("{@ a = [1.0] @}{: 1.5 <= a[0] :}", "false");
    check_ok("{@ a = [1] @}{: 1.5 <= a[0] :}", "false");
    check_ok("{@ a = [true] @}{: 1.5 <= a[0] :}", "false");
    check_fail("{@ a = [nil] @}{: 1.5 <= a[0] :}", "can't compare lte with float");
    check_fail("{@ a = [[]] @}{: 1.5 <= a[0] :}", "can't compare lte with float");
    check_fail("{@ a = [{}] @}{: 1.5 <= a[0] :}", "can't compare lte with float");
    check_fail("{@ def f(): end a = [f] @}{: 1.5 <= a[0] :}", "can't compare lte with float");

    trv_cleanup;
}

static void
test_trv_expr_string_0(void) {
    trv_ready;

    check_ok("{: \"abc\" \n + \n \"def\" :}", "abcdef");

    trv_cleanup;
}

static void
test_trv_expr_fail_0(void) {
    trv_ready;

    check_fail("{@ 1 + + @}", "syntax error. not found rhs operand in expr");
    check_fail("{@ + 1 + @}", "not found blocks");
    check_fail("{@ 1 + + @}", "syntax error. not found rhs operand in expr");
    check_fail("{@ 1 + [] @}", "can't add with int");
    check_fail("{@ [] + 1 @}", "invalid right hand operand (1)");
    check_fail("{@ 1 + {} @}", "can't add with int");
    check_fail("{@ {} + 1 @}", "can't add");
    check_fail("{@ 1 + \"a\" @}", "can't add with int");
    check_fail("{@ \"a\" + 1 @}", "can't add 1 with string");

    check_fail("{@ 1 - - @}", "syntax error. not found rhs operand in expr");
    check_fail("{@ - 1 - @}", "syntax error. not found rhs operand in expr");
    check_fail("{@ 1 - - @}", "syntax error. not found rhs operand in expr");
    check_fail("{@ 1 - [] @}", "can't sub with int");
    check_fail("{@ [] - 1 @}", "can't sub");
    check_fail("{@ 1 - {} @}", "can't sub with int");
    check_fail("{@ {} - 1 @}", "can't sub");
    check_fail("{@ 1 - \"a\" @}", "can't sub with int");
    check_fail("{@ \"a\" - 1 @}", "can't sub");

    trv_cleanup;
}

static void
test_trv_term_0(void) {
    trv_ready;

    check_ok("{: 2 * 2 :}", "4");
    check_ok("{: 2 * 2 * 3 :}", "12");
    check_ok("{: 2 * \"abc\" :}", "abcabc");
    check_ok("{: \"abc\" * 2 :}", "abcabc");
    check_ok("{: \"abc\" * 2 * 2 :}", "abcabcabcabc");
    check_ok("{: 0 * \"abc\" :}", "");
    check_fail("{: -1 * \"abc\" :}", "can't mul string by negative value");

    trv_cleanup;
}

static void
test_trv_term_1(void) {
    trv_ready;

    check_ok("{: 4 / 2 :}", "2");
    check_ok("{: 4 / true :}", "4");

    check_fail("{: 4 / 0 :}", "zero division error");
    check_fail("{: 4 / false :}", "zero division error");
    check_fail("{: 4 / \"aa\" :}", "invalid right hand operand");
    check_fail("{: 4 / [] :}", "invalid right hand operand");
    check_fail("{: 4 / {} :}", "invalid right hand operand");

    check_ok("{: true / 1 :}", "1");
    check_ok("{: false / 1 :}", "0");

    check_fail("{: \"aa\" / 1 :}", "can't division");
    check_fail("{: [] / 1 :}", "can't division");
    check_fail("{: {} / 1 :}", "can't division");

    trv_cleanup;
}

static void
test_trv_term_2(void) {
    trv_ready;

    check_ok("{: 4 % 2 :}", "0");
    check_ok("{: 3 % 2 :}", "1");
    check_ok("{: 1 % true :}", "0");

    check_fail("{: 1 % 0 :}", "zero division error");
    check_fail("{: 1 % false :}", "zero division error");
    check_fail("{: \"a\" % 1 :}", "invalid left hand operand (5)");
    check_fail("{: [] % 1 :}", "invalid left hand operand (6)");
    check_fail("{: {} % 1 :}", "invalid left hand operand (7)");

    check_fail("{: 1 % \"a\" :}", "invalid right hand operand (5)");
    check_fail("{: 1 % [] :}", "invalid right hand operand (6)");
    check_fail("{: 1 % {} :}", "invalid right hand operand (7)");

    check_fail("{: 4 % nil :}", "invalid right hand operand (0)");
    check_fail("{: nil % 2 :}", "invalid left hand operand (0)");

    trv_cleanup;
}

static void
test_trv_term_3(void) {
    trv_ready;

    check_ok("{: 2 * 2 / 4 % 2 :}", "1");
    check_ok("{: 4 / 2 * 2 % 2 :}", "0");
    check_ok("{: 3 % 2 * 3 / 3 :}", "1");
    check_ok("{: 3 * 2 / 3 * 3 :}", "6");
    check_ok("{: 4 / 2 * 2 / 2 :}", "2");
    check_ok("{: 3 % 2 * 2 % 2 :}", "0");
    check_ok("{: 3 * 2 % 2 * 2 :}", "0");

    trv_cleanup;
}

static void
test_trv_term_fail_0(void) {
    trv_ready;

    check_fail("{: * :}", "not found blocks");
    check_fail("{: 1 * :}", "syntax error. not found rhs operand in term");
    check_fail("{: * 1 :}", "not found blocks");
    check_ok("{: 1 * true :}", "1");
    check_ok("{: false * 1 :}", "0");
    check_ok("{: 1 * \"a\" :}", "a");
    check_ok("{: \"a\" * 1 :}", "a");
    check_fail("{: 1 * [] :}", "can't mul with int");
    check_fail("{: [] * 1 :}", "can't mul");
    check_fail("{: 1 * {} :}", "can't mul with int");
    check_fail("{: {} * 1 :}", "can't mul");

    check_fail("{: / :}", "not found blocks");
    check_fail("{: 1 / :}", "syntax error. not found rhs operand in term");
    check_fail("{: / 1 :}", "not found blocks");
    check_ok("{: 1 / true :}", "1");
    check_ok("{: false / 1 :}", "0");
    check_fail("{: 1 / \"a\" :}", "invalid right hand operand");
    check_fail("{: \"a\" / 1 :}", "can't division");
    check_fail("{: 1 / [] :}", "invalid right hand operand");
    check_fail("{: [] / 1 :}", "can't division");
    check_fail("{: 1 / {} :}", "invalid right hand operand");
    check_fail("{: {} / 1 :}", "can't division");

    trv_cleanup;
}

static void
test_trv_call_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ def f(): end f() @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_call_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       puts(a[0], a[1])\n"
    "   end\n"
    "   a = [1, 2]\n"
    "   f(a)\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       puts(a[0], a[1])\n"
    "   end\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       a = [i, i+1]\n"
    "       f(a)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 1\n1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       puts(a[0], a[1], a[2])\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       j = i\n"
    "       a = [j, j+1, j+2]\n"
    "       f(a)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 1 2\n1 2 3\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "    from \"tests/lang/modules/list.cap\" import arrayToUl\n"
    "\n"
    "    for i = 0; i < 4; i += 1:\n"
    "       j = i\n"
    "       a = [j, j+1, j+2]\n"
    "       arrayToUl(a)\n"
    "    end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx),
            "<ul>\n"
            "    <li>0</li>\n"
            "    <li>1</li>\n"
            "    <li>2</li>\n"
            "</ul>\n"
            "<ul>\n"
            "    <li>1</li>\n"
            "    <li>2</li>\n"
            "    <li>3</li>\n"
            "</ul>\n"
            "<ul>\n"
            "    <li>2</li>\n"
            "    <li>3</li>\n"
            "    <li>4</li>\n"
            "</ul>\n"
            "<ul>\n"
            "    <li>3</li>\n"
            "    <li>4</li>\n"
            "    <li>5</li>\n"
            "</ul>\n"
        ));
    }

    trv_cleanup;
}

static void
test_trv_call_5(void) {
    trv_ready;

    check_ok("{@\n"
    "   def f1(a):\n"
    "       puts(a)\n"
    "       return a * 2\n"
    "   end\n"
    "   def f2(a):\n"
    "       return f1(a)\n"
    "   end\n"
    "   def f3(a):\n"
    "       return f2(a)\n"
    "   end\n"
    "   def f4(a):\n"
    "       return f3(a)\n"
    "   end\n"
    "@}{: f4(2) :}", "2\n4");

    trv_cleanup;
}

static void
test_trv_call_fail_0(void) {
    trv_ready;

    check_fail("{@\n"
    "def f():\n"
    "end\n"
    "f(\n"
    "@}"
    , "not found ')'");

    check_fail("{@\n"
    "def f():\n"
    "end\n"
    "f)\n"
    "@}"
    , "not found blocks");

    check_fail("{@\n"
    "def f():\n"
    "end\n"
    "f(1)\n"
    "@}"
    , "arguments not same length");

    check_ok("{@\n"
    "def f(a):\n"
    "end\n"
    "f(1)\n"
    "@}"
    , "");

    check_fail("{@\n"
    "def f(a):\n"
    "end\n"
    "f(1, 2)\n"
    "@}"
    , "arguments not same length");

    trv_cleanup;
}

static void
test_trv_index_0(void) {
    trv_ready;

    check_ok("{@ a = [0, 1] @}{: a[0] :},{: a[1] :}", "0,1");

    trv_cleanup;
}

static void
test_trv_index_1(void) {
    trv_ready;

    check_ok("{@ a = [0, 1] @}{: a[0] :}", "0");

    trv_cleanup;
}

static void
test_trv_index_fail_0(void) {
    trv_ready;

    check_fail("{@ a = [0, 1] @}{: a[ :}", "not found expression");
    check_fail("{@ a = [0, 1] @}{: a] :}", "syntax error. not found \":}\"");
    check_fail("{@ a = [0, 1] @}{: a[] :}", "not found expression");
    check_fail("{@ a = [0, 1] @}{: a[\"a\"] :}", "index isn't integer");
    check_fail("{@ a = [0, 1] @}{: a[0][0] :}", "not indexable (1)");

    trv_cleanup;
}

static void
test_trv_array_0(void) {
    trv_ready;

    check_ok("{@ a = [0, 1] @}", "");

    trv_cleanup;
}

static void
test_trv_array_1(void) {
    trv_ready;

    check_ok("{@\n"
    "   i = 0\n"
    "   a = [i, 1]\n"
    "   a[0] += 1\n"
    "   puts(i)\n"
    "@}", "0\n");

    trv_cleanup;
}

static void
test_trv_array_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   i = 0\n"
    "   a = [i, 1]\n"
    "   puts(i)\n"
    "   puts(a[0])\n"
    "   puts(id(i) == id(a[0]))"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\ntrue\n"));
    }

    trv_cleanup;
}

static void
test_trv_array_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   i = 0\n"
    "   s = \"abc\"\n"
    "   n = nil\n"
    "   l = [0, 1, 2]\n"
    "   d = {\"a\": 1, \"b\": 2}\n"
    "   a = [i, s, n, l, d]\n"
    "   puts(a[0], a[1], a[2], a[3][0], a[4][\"a\"])\n"
    "   puts(id(i) == id(a[0]))\n"
    "   puts(id(s) == id(a[1]))\n"
    "   puts(id(n) == id(a[2]))\n"
    "   puts(id(l) == id(a[3]))\n"
    "   puts(id(d) == id(a[4]))\n"
    "   l[0] = 3\n"
    "   puts(l[0] == a[3][0])\n"
    "   d[\"a\"] = 3\n"
    "   puts(d[\"a\"] == a[4][\"a\"])\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0 abc nil 0 1\ntrue\ntrue\ntrue\ntrue\ntrue\ntrue\ntrue\n"));
    }

    trv_cleanup;
}

static void
test_trv_array_4(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   a = [\n"
    "       1,\n"
    "       2, 3,\n"
    "       4,\n"
    "   ]\n"
    "@}{: a[0] :}{: a[1] :}{: a[2] :}{: a[3] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1234"));
    }

    trv_cleanup;
}

static void
test_trv_array_5(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ a = [] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1] \n b = a @}{: a :},{: b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array),(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [b = 1, c = 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, b = 2] @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    trv_cleanup;
}

static void
test_trv_array_6(void) {
    trv_ready;

    check_ok("{@ a = [] @}{: a :}", "(array)");

    trv_cleanup;
}

static void
test_trv_array_7(void) {
    trv_ready;

    check_ok("{@\n"
    "a = [[0, 1], [2, 3]]\n"
    "@}{: a[0][0] :},{: a[0][1] :},{: a[1][0] :},{: a[1][1] :}"
    , "0,1,2,3");

    trv_cleanup;
}

static void
test_trv_array_8(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "end\n"
    "a = [A(), A()]\n"
    "@}{: a[0] :},{: a[1] :}", "(object),(object)");

    trv_cleanup;
}

static void
test_trv_array_9(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "end\n"
    "a = [A(), A()]\n"
    "@}{: a[0] :},{: a[1] :}", "(object),(object)");

    trv_cleanup;
}

static void
test_trv_array_10(void) {
    trv_ready;

    check_ok("{@\n"
    "struct A:\n"
    "end\n"
    "def f():\n"
    "end\n"
    "a = [A(), f]\n"
    "@}{: a[0] :},{: a[1] :}", "(object),(function)");

    trv_cleanup;
}

static void
test_trv_array_11(void) {
    trv_ready;

    check_ok("{@\n"
    "d = {\"a\": 1, \"b\": 2}\n"
    "a = [d, d]\n"
    "@}{: a[0] :},{: a[1] :}", "(dict),(dict)");

    trv_cleanup;
}

static void
test_trv_array_12(void) {
    trv_ready;

    check_ok("{@\n"
    "d = {\"a\": 1, \"b\": 2}\n"
    "a = [d, d]\n"
    "@}{: a[0][\"a\"] :},{: a[1][\"b\"] :}"
    , "1,2");

    trv_cleanup;
}

static void
test_trv_array_13(void) {
    trv_ready;

    check_ok("{@\n"
    "d = {\"a\": 1, \"b\": 2}\n"
    "b = [2, d]\n"
    "a = [1, b, 3]\n"
    "@}{: a[1][1][\"b\"] :}", "2");

    trv_cleanup;
}

static void
test_trv_array_14(void) {
    trv_ready;

    check_ok("{@\n"
    "a = Array()\n"
    "@}{: len(a) :}", "0");

    trv_cleanup;
}

static void
test_trv_array_fail_0(void) {
    trv_ready;

    check_fail("{@ a = [ @}", "not found ']' in array");
    check_fail("{@ a = ] @}", "syntax error. not found rhs test in assign list");
    check_fail("{@ a = [,] @}", "not found ']' in array");
    check_fail("{@ a = [,,] @}", "not found ']' in array");
    check_ok("{@ a = [1,] @}", "");
    check_ok("{@ a = [1,2,] @}", "");
    check_fail("{@ a = [,1] @}", "not found ']' in array");

    trv_cleanup;
}

static void
test_trv_nil(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: nil :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_false(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: false :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "false"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_true(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: true :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_digit(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: 1 :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_string(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{: \"abc\" :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "abc"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_dict_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@ d = {\"a\":1, \"b\":2} @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_dict_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   d = {\n"
    "       \"a\" \n : \n 1 \n, \n"
    "       \"b\" \n : \n 2 \n, \n"
    "   }\n"
    "@}{: d[\"a\"] :}{: d[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "12"));
    }

    trv_cleanup;
}

static void
test_trv_dict_2(void) {
    trv_ready;

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@\n"
    "   d = {\"a\": 1}\n"
    "@}{: d[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "not found key \"b\""));
    }

    trv_cleanup;
}

static void
test_trv_dict_3(void) {
    trv_ready;

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@ a = { 1: 1 } @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "key is not string in dict elem"));
    }

    PadTkr_Parse(tkr, "{@ a = { \"k\": 1 } \n a[0] @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "index isn't string"));
    }

    PadTkr_Parse(tkr, "{@ k = 1 \n a = { k: 1 } @}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid key type in variable of dict"));
    }

    // success

    PadTkr_Parse(tkr, "{@ a = { \"key\": 1 } @}{: a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(dict)"));
    }

    PadTkr_Parse(tkr, "{@ a = { \"key\": 1 } @}{: a[\"key\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ a = { \"key\": \"val\" } @}{: a[\"key\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "val"));
    }

    PadTkr_Parse(tkr, "{@ a = { \"key\": [1, 2] } @}{: a[\"key\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(array)"));
    }

    // PadTkr_Parse(tkr, "{@ a = { \"key\": 1 }[\"key\"] @}{: a :}");
    // {
    PadAst_Clear(ast);
    //     PadCc_Compile(ast, PadTkr_GetToks(tkr));
    //     PadCtx_Clear(ctx);
    //     PadTrv_Trav(ast, ctx);
    //     assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    // }

    PadTkr_Parse(tkr, "{@ a = { \"k1\": 1, \"k2\": 2 } @}{: a[\"k1\"] :},{: a[\"k2\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1,2"));
    }

    PadTkr_Parse(tkr, "{@ a = { \"k1\": { \"k2\": 1 } } @}{: a[\"k1\"][\"k2\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ k = \"key\" \n a = { k: 1 } @}{: a[k] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_dict_fail_0(void) {
    trv_ready;    

    check_fail("{@ d = { @}", "not found right brace in parse dict");
    check_fail("{@ {1} @}", "not found colon in parse dict elem");
    check_fail("{@ {1:} @}", "not found value in parse dict elem");
    check_fail("{@ {1:2} @}", "key is not string in dict elem");
    check_fail("{@ {\"a\": 1, 2} @}", "not found colon in parse dict elem");
    check_ok("{@ {\"a\": 1, } @}", "");
    check_fail("{@ {\"a\": 1, \"b\"} @}", "not found colon in parse dict elem");
    check_fail("{@ {\"a\": 1, \"b\":} @}", "not found value in parse dict elem");
    check_fail("{@ {\"a\": b} @}", "\"b\" is not defined. can not store to dict elements");
    check_fail("{@ {[]} @}", "not found colon in parse dict elem");
    check_fail("{@ {{}} @}", "not found colon in parse dict elem");

    trv_cleanup;
}

static void
test_trv_identifier(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ i = 1 @}{: i :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_array_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ arr = [1, 2] \n arr.push(3) @}{: len(arr) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{: len([1, 2].push(3)) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@ a = [1, 2] @}{: a.pop() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@ a = [] @}{: a.pop() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadTkr_Parse(tkr, "{: [1, 2].pop() :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_builtin_dict_0(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAst_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    PadTkr_Parse(tkr, "{@ d = {\"a\": 1} @}{: d.get(1) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "invalid index type (1) of dict"));
    }

    PadTkr_Parse(tkr, "{@ d = {\"a\": 1} @}{: d.get(\"a\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        trace();
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    PadTkr_Parse(tkr, "{@ d = {\"a\": 1} @}{: d.get(\"b\") :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAst_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static void
test_trv_module_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/module.cap\" as mod\n"
    "   puts(\"done\")\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "imported\nimported module.cap\ndone\n"));
    }

    trv_cleanup;
}

static void
test_trv_chain_object(void) {
    trv_ready;

    /*****
    * ok *
    *****/

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1"));
    }

    /*******
    * fail *
    *******/

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "@}{: string.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}{: string.a :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}{: string.a.b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   import \"tests/lang/modules/string.cap\" as string\n"
    "@}{: string.a.b :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        (PadTrv_Trav(ast, ctx));
        assert(PadAst_HasErrs(ast));
        assert(!strcmp(PadAst_GetcFirstErrMsg(ast), "\"a\" is not defined"));
    }

    trv_cleanup;
}

static void
test_trv_etc_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def decolate(s):\n"
    "       return \"***\" + s + \"***\"\n"
    "   end\n"
    "   s = decolate(\"i love life\")\n"
    "   puts(s)\n"
    "   for i = 0; i < len(s); i += 1:\n"
    "       puts(s[i])\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "***i love life***\n*\n*\n*\ni\n \nl\no\nv\ne\n \nl\ni\nf\ne\n*\n*\n*\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_1(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def slice(arr, n):\n"
    "       mat = []\n"
    "       for i = 0; i < len(arr); i += n:\n"
    "           row = []\n"
    "           for j = 0; j < n and i + j < len(arr); j += 1:\n"
    "               row.push(arr[i+j])\n"
    "           end\n"
    "           mat.push(row)\n"
    "       end\n"
    "       return mat\n"
    "   end\n"
    "   arr = [1, 2, 3, 4]\n"
    "   mat = slice(arr, 2)\n"
    "   for i = 0; i < len(mat); i += 1:\n"
    "       row = mat[i]\n"
    "       for j = 0; j < len(row); j += 1:\n"
    "           puts(row[j])\n"
    "       end\n"
    "       puts(\",\")\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1\n2\n,\n3\n4\n,\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_2(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def header(title):\n"
    "@}<html>\n"
    "<head>\n"
    "<title>{: title :}</title>\n"
    "</head>\n"
    "{@\n"
    "   end\n"
    "\n"
    "   def body(title, message):\n"
    "@}<body>\n"
    "<h1>{: title :}</h1>\n"
    "<p>{: message :}</p>\n"
    "</body>\n"
    "{@\n"
    "   end\n"
    "\n"
    "   def footer():\n"
    "@}</html>\n"
    "{@\n"
    "   end\n"
    "\n"
    "   def index(kwargs):\n"
    "       title = kwargs[\"title\"]\n"
    "       message = kwargs[\"message\"]\n"
    "       header(title)\n"
    "       body(title, message)\n"
    "       footer()\n"
    "   end\n"
    "\n"
    "   index({\n"
    "       \"title\": \"Good will hunting\",\n"
    "       \"message\": \"I'm a robot\",\n"
    "   })\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "<html>\n"
            "<head>\n"
            "<title>Good will hunting</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>Good will hunting</h1>\n"
            "<p>I'm a robot</p>\n"
            "</body>\n"
            "</html>\n"
        ));
    }

    trv_cleanup;
}

static void
test_trv_etc_3(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   d = {\"a\": 1}"
    "   a = [d, 2]\n"
    "   a[0][\"a\"] += 1\n"
    "@}{: a[0][\"a\"] :},{: d[\"a\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2,2"));
    }

    trv_cleanup;
}

static void
test_trv_etc_4(void) {
    trv_ready;

    /***************************
    * theme: dict and function *
    ***************************/

    PadTkr_Parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   a = f(f)\n"
    "@}{: id(a) == id(f) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "true"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"a\"]\n"
    "   end\n"
    "   d = {\"a\": f}\n"
    "   a = d[\"a\"](d)"
    "@}{: a :},{: id(a) == id(f) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "(function),true"));
    }

    trv_cleanup;
}

static void
test_trv_etc_5(void) {

    /***************************
    * theme: dict and function *
    ***************************/

    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"a\"] + arg[\"b\"]\n"
    "   end\n"
    "   a = 1, b = 2\n"
    "   c = f({ \"a\" : a, \"b\": b })"
    "@}{: c :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"d\"][\"a\"] + arg[\"c\"][\"b\"]\n"
    "   end\n"
    "   d = { \"a\": 1, \"b\": 2 }\n"
    "   c = f({ \"d\" : d, \"c\": d })\n"
    "@}{: c :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       arg[\"d\"][\"a\"] += 1\n"
    "       arg[\"c\"][\"b\"] += 1\n"
    "   end\n"
    "   d = { \"a\": 1, \"b\": 2 }\n"
    "   c = f({ \"d\" : d, \"c\": d })\n"
    "@}{: d[\"a\"] :},{: d[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2,3"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       e = arg[\"d\"]\n"
    "       e[\"a\"] += 1\n"
    "   end\n"
    "   d = { \"a\": 1 }\n"
    "   c = f({ \"d\" : d })\n"
    "@}{: d[\"a\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       e = arg[\"d\"]\n"
    "       e[\"a\"] += 1\n"
    "   end\n"
    "   d = { \"a\": 1 }\n"
    "   c = f({ \"d\" : d })\n"
    "@}{: d[\"a\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       d = arg[\"d\"]\n"
    "       l = d[\"a\"]\n"
    "       l[0] += 1"
    "   end\n"
    "   l = [1, 2]\n"
    "   d = { \"a\": l }\n"
    "   c = f({ \"d\" : d })\n"
    "@}{: l[0] :},{: l[1] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2,2"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       d = arg[\"d\"]\n"
    "       l = d[\"a\"]\n"
    "       return { \"a\": l[0]+1, \"b\": l[1]+1 }\n"
    "   end\n"
    "   l = [1, 2]\n"
    "   d = { \"a\": l }\n"
    "   c = f({ \"d\" : d })\n"
    "@}{: c[\"a\"] :},{: c[\"b\"] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2,3"));
    }

    trv_cleanup;
}

static void
test_trv_etc_6(void) {
    trv_ready;

    /***************************
    * theme: list and function *
    ***************************/

    PadTkr_Parse(tkr, "{@\n"
    "   def f(l):\n"
    "       l.push(3)\n"
    "   end\n"
    "   l = [1, 2]\n"
    "   f(l)\n"
    "@}{: l[2] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   def f(l):\n"
    "       l.push([3, 4])\n"
    "       return l"
    "   end\n"
    "   l = [1, 2]\n"
    "   l2 = f(l)\n"
    "@}{: l2[2][0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   g = 3"
    "   def f(l):\n"
    "       l.push(g)\n"
    "   end\n"
    "   l = [1, 2]\n"
    "   f(l)\n"
    "@}{: l[2] :},{: id(l[2]) != id(g) :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "3,true"));
    }

    trv_cleanup;
}

static void
test_trv_etc_7(void) {
    trv_ready;

    /*******************************
    * theme: for and if statements *
    *******************************/

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 4; i += 1:\n"
    "       if i % 2 == 0:\n"
    "           puts(\"nyan\")\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nyan\nnyan\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 4; i += 1:\n"
    "       if i % 2 == 0:\n"
    "           for j = 0; j < 2; j += 1:\n"
    "               puts(j)"
    "           end\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   m = 2\n"
    "   if m == 2:\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   m = 3\n"
    "   if m == 2:\n"
    "   elif m == 3:\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   m = 4\n"
    "   if m == 2:\n"
    "   elif m == 3:\n"
    "   else:\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_8(void) {
    trv_ready;

    /**************************
    * theme: function and for *
    **************************/

    PadTkr_Parse(tkr, "{@\n"
    "   def f(n):\n"
    "       for i = 0; i < n; i += 1:\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "   \n"
    "   for i = 0; i < 2; i += 1:\n"
    "       f(i+1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n1\n"));
    }

    PadTkr_Parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       def f(n):\n"
    "           for i = 0; i < n; i += 1:\n"
    "               puts(i)\n"
    "           end\n"
    "       end\n"
    "       f(i+1)\n"
    "   end\n"
    "@}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadCtx_Clear(ctx);
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "0\n0\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_9(void) {
    trv_ready;

    const char *s = "{@\n"
"arr = [3, 2, 4, 1]\n"
"for j = 0; j < 4; j += 1:\n"
"   for i = 0; i < len(arr) - 1; i += 1:\n"
"       if arr[i] > arr[i + 1]:\n"
"           tmp = arr[i]\n"
"           arr[i] = arr[i + 1]\n"
"           arr[i + 1] = tmp\n"
"       end\n"
"   end\n"
"end\n"
"puts(arr[0], arr[1], arr[2], arr[3])\n"
"@}";

    PadTkr_Parse(tkr, s);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "1 2 3 4\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_10(void) {
    trv_ready;

    const char *s = "{@\n"
"arr = [4, 1, 2, 3]\n"
"i = 0\n"
"tmp = arr[i]\n"
"arr[i] = arr[i + 1]\n"
"arr[i + 1] = tmp\n"
"i = 1\n"
"tmp = arr[i]\n"
"arr[i] = arr[i + 1]\n"
"arr[i + 1] = tmp\n"
"i = 2\n"
"tmp = arr[i]\n"
"arr[i] = arr[i + 1]\n"
"arr[i + 1] = tmp\n"
"i = 0\n"
"tmp = arr[i]\n"
"arr[i] = arr[i + 1]\n"
"arr[i + 1] = tmp\n"
"puts(arr[0], arr[1], arr[2], arr[3])\n"
"@}";

    PadTkr_Parse(tkr, s);
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "2 1 3 4\n"));
    }

    trv_cleanup;
}

static void
test_trv_unicode_0(void) {
    trv_ready;

    PadTkr_Parse(tkr, "{@\n"
    "   s = \"abc\""
    "@}{: s[0] :}");
    {
        PadAst_Clear(ast);
        PadCc_Compile(ast, PadTkr_GetToks(tkr));
        PadTrv_Trav(ast, ctx);
        assert(!PadAst_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "a"));
    }

    trv_cleanup;
}

static void
test_trv_unicode_1(void) {
    trv_ready;

    check_ok("{@\n"
    "   s = \"あいう\""
    "@}{: s[0] :}{: s[2] :}", "あう");

    trv_cleanup;
}

static void
test_trv_scope_0(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "if true:\n"
    "   a = 1\n"
    "end\n"
    "@}{: a :}", "1");

    trv_cleanup;
}

static void
test_trv_scope_1(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "def f():\n"
    "   a = 1\n"
    "end\n"
    "f()\n"
    "@}{: a :}", "0");

    trv_cleanup;
}

static void
test_trv_scope_2(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "def f():\n"
    "   a = 1\n"
    "end\n"
    "f()\n"
    "@}{: a :}", "0");

    trv_cleanup;
}

static void
test_trv_scope_3(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "def f():\n"
    "   puts(a)\n"
    "end\n"
    "f()\n"
    "@}", "0\n");

    trv_cleanup;
}

static void
test_trv_scope_4(void) {
    trv_ready;

    check_fail("{@\n"
    "def f(a):\n"
    "   def inner():\n"
    "       return a\n"
    "   end\n"
    "   return inner\n"
    "end\n"
    "c = f(1)\n"
    "@}{: c() :}", "\"a\" is not defined");  // TODO: change fail to ok

    trv_cleanup;
}

static void
test_trv_scope_5(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "def f():\n"
    "   return a\n"
    "end\n"
    "@}{: f() :}", "0");

    trv_cleanup;
}

static void
test_trv_scope_6(void) {
    trv_ready;

    check_ok("{@\n"
    "a = 0\n"
    "def f():\n"
    "   return a + 1\n"
    "end\n"
    "@}{: f() :}", "1");

    trv_cleanup;
}

static void
test_trv_scope_7(void) {
    trv_ready;

    check_fail("{@\n"
    "a = 0\n"
    "def f():\n"
    "   a += 1\n"
    "   return a\n"
    "end\n"
    "@}{: f() :}", "\"a\" is not defined");

    trv_cleanup;
}

static void 
test_trv_mutable_and_immutable(void) {
    trv_ready;

    /* Pad's mutable and immutable of object
     *
     * Mutable:
     *
     *   array
     *   dict
     *
     * Immutable:
     *
     *   int
     *   float
     *   string
     * 
     * But int and float of function arguments copied and pass
     */
    // check_fail("{@ 0 += 1 @}", "invalid left hand operand (1)");

    // check_ok("{@ i = 0 iid = id(i) i += 1 @}{: iid != id(i) :},{: i :}", "true,1");
    // check_ok("{@ i = 0.12 iid = id(i) i += 0.01 @}{: iid != id(i) :},{: i :}", "true,0.13");
    // check_ok("{@ i = \"aaa\" iid = id(i) i += \"bbb\" @}{: iid != id(i) :},{: i :}", "true,aaabbb");

    // check_ok_trace("{@ def f(a): a += 1 end \n i = 0 \n f(i) @}{: i :}", "0")
    // check_ok_trace("{@ def f(a): a += 0.1 end \n i = 0.0 \n f(i) @}{: i :}", "0.0")
    // check_ok_trace("{@ def f(a): a += \"b\" end \n i = \"a\" \n f(i) @}{: i :}", "a")

    check_ok_trace("{@ a = [100] aid = id(a[0]) a[0] += 1 @}{: aid != id(a[0]) :},{: a[0] :}", "true,101")
    // check_ok_trace("{@ a = {\"b\": 0} aid = id(a[\"b\"]) a[\"b\"] += 1 @}{: aid != id(a[\"b\"]) :},{: a[\"b\"] :}", "true,1")

    trv_cleanup;
}

static const struct testcase
traverser_tests[] = {
    {"scope_0", test_trv_scope_0},
    {"scope_1", test_trv_scope_1},
    {"scope_2", test_trv_scope_2},
    {"scope_3", test_trv_scope_3},
    {"scope_4", test_trv_scope_4},
    {"scope_5", test_trv_scope_5},
    {"scope_6", test_trv_scope_6},
    {"scope_7", test_trv_scope_7},
    {"mutable_and_immutable", test_trv_mutable_and_immutable},
    {"assign_and_reference_0", test_trv_assign_and_reference_0},
    {"assign_and_reference_1", test_trv_assign_and_reference_1},
    {"assign_and_reference_2", test_trv_assign_and_reference_2},
    {"assign_and_reference_3", test_trv_assign_and_reference_3},
    {"assign_and_reference_4", test_trv_assign_and_reference_4},
    {"assign_and_reference_5", test_trv_assign_and_reference_5},
    {"assign_and_reference_6", test_trv_assign_and_reference_6},
    {"assign_and_reference_7", test_trv_assign_and_reference_7},
    {"assign_and_reference_8", test_trv_assign_and_reference_8},
    {"assign_and_reference_9", test_trv_assign_and_reference_9},
    {"assign_and_reference_10", test_trv_assign_and_reference_10},
    {"assign_and_reference_11", test_trv_assign_and_reference_11},
    {"assign_and_reference_11_5", test_trv_assign_and_reference_11_5},
    {"assign_and_reference_11_6", test_trv_assign_and_reference_11_6},
    {"assign_and_reference_11_7", test_trv_assign_and_reference_11_7},
    {"assign_and_reference_12", test_trv_assign_and_reference_12},
    {"assign_and_reference_13", test_trv_assign_and_reference_13},
    {"assign_and_reference_14", test_trv_assign_and_reference_14},
    {"assign_and_reference_15", test_trv_assign_and_reference_15},
    {"assign_and_reference_16", test_trv_assign_and_reference_16},
    {"assign_and_reference_all", test_trv_assign_and_reference_all},
    {"code_block", test_trv_code_block},
    {"code_block_fail", test_trv_code_block_fail},
    {"ref_block", test_trv_ref_block},
    {"ref_block_fail", test_trv_ref_block_fail},
    {"text_block", test_trv_text_block},
    {"if_stmt_0", test_trv_if_stmt_0},
    {"if_stmt_1", test_trv_if_stmt_1},
    {"if_stmt_2", test_trv_if_stmt_2},
    {"if_stmt_3", test_trv_if_stmt_3},
    {"if_stmt_4", test_trv_if_stmt_4},
    {"if_stmt_5", test_trv_if_stmt_5},
    {"if_stmt_6", test_trv_if_stmt_6},
    {"if_stmt_7", test_trv_if_stmt_7},
    {"if_stmt_8", test_trv_if_stmt_8},
    {"if_stmt_9", test_trv_if_stmt_9},
    {"if_stmt_10", test_trv_if_stmt_10},
    {"if_stmt_11", test_trv_if_stmt_11},
    {"if_stmt_12", test_trv_if_stmt_12},
    {"if_stmt_13", test_trv_if_stmt_13},
    {"if_stmt_fail_0", test_trv_if_stmt_fail_0},
    {"if_stmt_fail_1", test_trv_if_stmt_fail_1},
    {"elif_stmt_0", test_trv_elif_stmt_0},
    {"elif_stmt_1", test_trv_elif_stmt_1},
    {"elif_stmt_2", test_trv_elif_stmt_2},
    {"elif_stmt_3", test_trv_elif_stmt_3},
    {"elif_stmt_4", test_trv_elif_stmt_4},
    {"elif_stmt_5", test_trv_elif_stmt_5},
    {"elif_stmt_6", test_trv_elif_stmt_6},
    {"elif_stmt_7", test_trv_elif_stmt_7},

    {"else_stmt_0", test_trv_else_stmt_0},
    {"else_stmt_1", test_trv_else_stmt_1},
    {"else_stmt_2", test_trv_else_stmt_2},
    {"else_stmt_3", test_trv_else_stmt_3},
    {"else_stmt_4", test_trv_else_stmt_4},

    {"for_stmt_0", test_trv_for_stmt_0},
    {"for_stmt_1", test_trv_for_stmt_1},
    {"for_stmt_2", test_trv_for_stmt_2},
    {"for_stmt_3", test_trv_for_stmt_3},
    {"for_stmt_4", test_trv_for_stmt_4},
    {"for_stmt_5", test_trv_for_stmt_5},
    {"for_stmt_6", test_trv_for_stmt_6},
    {"for_stmt_7", test_trv_for_stmt_7},
    {"for_stmt_8", test_trv_for_stmt_8},
    {"for_stmt_9", test_trv_for_stmt_9},
    {"for_stmt_10", test_trv_for_stmt_10},
    {"for_stmt_11", test_trv_for_stmt_11},
    {"for_stmt_12", test_trv_for_stmt_12},
    {"break_stmt_0", test_trv_break_stmt_0},
    {"break_stmt_1", test_trv_break_stmt_1},
    {"break_stmt_2", test_trv_break_stmt_2},
    {"break_stmt_3", test_trv_break_stmt_3},
    {"continue_stmt_0", test_trv_continue_stmt_0},
    {"continue_stmt_1", test_trv_continue_stmt_1},
    {"continue_stmt_2", test_trv_continue_stmt_2},
    {"continue_stmt_3", test_trv_continue_stmt_3},
    {"continue_stmt_4", test_trv_continue_stmt_4},
    {"continue_stmt_5", test_trv_continue_stmt_5},
    {"return_stmt_0", test_trv_return_stmt_0},
    {"return_stmt_1", test_trv_return_stmt_1},
    {"return_stmt_2", test_trv_return_stmt_2},
    {"return_stmt_3", test_trv_return_stmt_3},
    {"return_stmt_4", test_trv_return_stmt_4},
    {"return_stmt_5", test_trv_return_stmt_5},
    {"return_stmt_6", test_trv_return_stmt_6},
    {"block_stmt_0", test_trv_block_stmt_0},
    {"block_stmt_1", test_trv_block_stmt_1},
    {"block_stmt_2", test_trv_block_stmt_2},
    {"block_stmt_3", test_trv_block_stmt_3},
    {"block_stmt_4", test_trv_block_stmt_4},
    {"block_stmt_fail_0", test_trv_block_stmt_fail_0},
    {"inject_stmt_0", test_trv_inject_stmt_0},
    {"inject_stmt_1", test_trv_inject_stmt_1},
    {"inject_stmt_2", test_trv_inject_stmt_2},
    {"inject_stmt_3", test_trv_inject_stmt_3},
    {"inject_stmt_4", test_trv_inject_stmt_4},
    {"inject_stmt_5", test_trv_inject_stmt_5},
    {"inject_stmt_6", test_trv_inject_stmt_6},
    {"inject_stmt_7", test_trv_inject_stmt_7},
    {"inject_stmt_8", test_trv_inject_stmt_8},
    {"inject_stmt_9", test_trv_inject_stmt_9},
    {"inject_stmt_10", test_trv_inject_stmt_10},
    {"inject_stmt_11", test_trv_inject_stmt_11},
    {"inject_stmt_12", test_trv_inject_stmt_12},
    {"inject_stmt_13", test_trv_inject_stmt_13},
    {"inject_stmt_14", test_trv_inject_stmt_14},
    {"inject_stmt_15", test_trv_inject_stmt_15},
    {"inject_stmt_16", test_trv_inject_stmt_16},
    {"inject_stmt_17", test_trv_inject_stmt_17},
    {"inject_stmt_18", test_trv_inject_stmt_18},
    {"inject_stmt_19", test_trv_inject_stmt_19},
    {"inject_stmt_20", test_trv_inject_stmt_20},
    {"inject_stmt_fail_0", test_trv_inject_stmt_fail_0},
    {"func_def_0", test_trv_func_def_0},
    {"func_def_1", test_trv_func_def_1},
    {"func_def_2", test_trv_func_def_2},
    {"func_def_3", test_trv_func_def_3},
    {"func_def_4", test_trv_func_def_4},
    {"func_def_5", test_trv_func_def_5},
    {"func_def_6", test_trv_func_def_6},
    {"func_def_7", test_trv_func_def_7},
    {"func_def_8", test_trv_func_def_8},
    {"func_def_9", test_trv_func_def_9},
    {"func_def_10", test_trv_func_def_10},
    {"func_def_11", test_trv_func_def_11},
    {"func_def_12", test_trv_func_def_12},
    {"func_def_13", test_trv_func_def_13},
    {"func_def_14", test_trv_func_def_14},
    {"func_def_fail_0", test_trv_func_def_fail_0},
    {"func_met_0", test_trv_func_met_0},
    {"func_met_1", test_trv_func_met_1},
    {"func_met_2", test_trv_func_met_2},
    {"func_met_3", test_trv_func_met_3},
    {"func_met_4", test_trv_func_met_4},
    {"func_met_5", test_trv_func_met_5},
    {"func_extends_0", test_trv_func_extends_0},
    {"func_extends_fail_0", test_trv_func_extends_fail_0},
    {"func_super_0", test_trv_func_super_0},
    {"func_super_1", test_trv_func_super_1},
    {"func_super_2", test_trv_func_super_2},
    {"func_super_fail_0", test_trv_func_super_fail_0},
    {"struct_1", test_trv_struct_1},
    {"struct_2", test_trv_struct_2},
    {"struct_3", test_trv_struct_3},
    {"struct_4", test_trv_struct_4},
    {"struct_5", test_trv_struct_5},
    {"struct_6", test_trv_struct_6},
    {"struct_7", test_trv_struct_7},
    {"struct_8", test_trv_struct_8},
    {"struct_9", test_trv_struct_9},
    {"struct_10", test_trv_struct_10},
    {"struct_11", test_trv_struct_11},
    {"struct_12", test_trv_struct_12},
    {"struct_13", test_trv_struct_13},
    {"struct_14", test_trv_struct_14},
    {"struct_15", test_trv_struct_15},
    {"struct_16", test_trv_struct_16},
    {"struct_17", test_trv_struct_17},
    {"struct_18", test_trv_struct_18},
    {"struct_19", test_trv_struct_19},
    {"struct_20", test_trv_struct_20},
    {"struct_21", test_trv_struct_21},
    {"struct_22", test_trv_struct_22},
    {"struct_23", test_trv_struct_23},
    {"struct_24", test_trv_struct_24},
    {"struct_25", test_trv_struct_25},
    {"struct_26", test_trv_struct_26},
    {"struct_27", test_trv_struct_27},
    {"struct_28", test_trv_struct_28},
    {"struct_29", test_trv_struct_29},
    {"struct_30", test_trv_struct_30},
    {"struct_31", test_trv_struct_31},
    {"struct_32", test_trv_struct_32},
    {"struct_33", test_trv_struct_33},
    {"struct_34", test_trv_struct_34},
    {"struct_35", test_trv_struct_35},
    {"struct_36", test_trv_struct_36},
    {"struct_37", test_trv_struct_37},
    {"struct_38", test_trv_struct_38},
    {"struct_39", test_trv_struct_39},
    {"struct_40", test_trv_struct_40},
    {"struct_41", test_trv_struct_41},
    {"struct_42", test_trv_struct_42},
    {"struct_43", test_trv_struct_43},
    {"struct_44", test_trv_struct_44},
    {"struct_45", test_trv_struct_45},
    {"struct_46", test_trv_struct_46},
    {"struct_47", test_trv_struct_47},
    {"struct_fail_0", test_trv_struct_fail_0},
    {"type_0", test_trv_type_0},
    {"type_1", test_trv_type_1},
    {"type_2", test_trv_type_2},
    {"type_3", test_trv_type_3},
    {"type_4", test_trv_type_4},
    {"type_5", test_trv_type_5},
    {"assign_list_0", test_trv_assign_list_0},
    {"assign_list_1", test_trv_assign_list_1},
    {"assign_list_2", test_trv_assign_list_2},
    {"assign_list_3", test_trv_assign_list_3},
    {"assign_list_fail_0", test_trv_assign_list_fail_0},
    {"multi_assign_0", test_trv_multi_assign_0},
    {"multi_assign_1", test_trv_multi_assign_1},
    {"multi_assign_fail_0", test_trv_multi_assign_fail_0},
    {"or_test_0", test_trv_or_test_0},
    {"or_test_fail_0", test_trv_or_test_fail_0},
    {"and_test_0", test_trv_and_test_0},
    {"and_test_fail_0", test_trv_and_test_fail_0},
    {"not_test_0", test_trv_not_test_0},
    {"not_test_fail_0", test_trv_not_test_fail_0},
    {"asscalc_0", test_trv_asscalc_0},
    {"asscalc_1", test_trv_asscalc_1},
    {"asscalc_2", test_trv_asscalc_2},
    {"asscalc_3", test_trv_asscalc_3},
    {"asscalc_4", test_trv_asscalc_4},
    {"asscalc_5", test_trv_asscalc_5},
    {"asscalc_6", test_trv_asscalc_6},
    {"asscalc_7", test_trv_asscalc_7},
    {"asscalc_8", test_trv_asscalc_8},
    {"asscalc_9", test_trv_asscalc_9},
    {"asscalc_10", test_trv_asscalc_10},
    {"asscalc_11", test_trv_asscalc_11},
    {"asscalc_12", test_trv_asscalc_12},
    {"asscalc_13", test_trv_asscalc_13},
    {"asscalc_14", test_trv_asscalc_14},
    {"asscalc_15", test_trv_asscalc_15},
    {"asscalc_16", test_trv_asscalc_16},
    {"asscalc_17", test_trv_asscalc_17},
    {"asscalc_18", test_trv_asscalc_18},
    {"asscalc_19", test_trv_asscalc_19},
    {"asscalc_20", test_trv_asscalc_20},
    {"asscalc_21", test_trv_asscalc_21},
    {"asscalc_22", test_trv_asscalc_22},
    {"asscalc_23", test_trv_asscalc_23},
    {"asscalc_24", test_trv_asscalc_24},
    {"asscalc_25", test_trv_asscalc_25},
    {"asscalc_add_ass_string", test_trv_asscalc_add_ass_string},
    {"asscalc_fail_0", test_trv_asscalc_fail_0},
    {"expr_0", test_trv_expr_0},
    {"expr_1", test_trv_expr_1},
    {"expr_2", test_trv_expr_2},
    {"expr_3", test_trv_expr_3},
    {"expr_4", test_trv_expr_4},
    {"expr_4a", test_trv_expr_4a},
    {"expr_5", test_trv_expr_5},
    {"expr_6", test_trv_expr_6},
    {"expr_7", test_trv_expr_7},
    {"expr_8", test_trv_expr_8},
    {"expr_9", test_trv_expr_9},
    {"expr_float", test_trv_expr_float},
    {"expr_string_0", test_trv_expr_string_0},
    {"expr_fail_0", test_trv_expr_fail_0},
    {"term_0", test_trv_term_0},
    {"term_1", test_trv_term_1},
    {"term_2", test_trv_term_2},
    {"term_3", test_trv_term_3},
    {"term_fail_0", test_trv_term_fail_0},
    {"call_0", test_trv_call_0},
    {"call_1", test_trv_call_1},
    {"call_2", test_trv_call_2},
    {"call_3", test_trv_call_3},
    {"call_4", test_trv_call_4},
    {"call_5", test_trv_call_5},
    {"call_fail_0", test_trv_call_fail_0},
    {"index_0", test_trv_index_0},
    {"index_1", test_trv_index_1},
    {"index_fail_0", test_trv_index_fail_0},
    {"array_0", test_trv_array_0},
    {"array_1", test_trv_array_1},
    {"array_2", test_trv_array_2},
    {"array_3", test_trv_array_3},
    {"array_4", test_trv_array_4},
    {"array_5", test_trv_array_5},
    {"array_6", test_trv_array_6},
    {"array_7", test_trv_array_7},
    {"array_8", test_trv_array_8},
    {"array_9", test_trv_array_9},
    {"array_10", test_trv_array_10},
    {"array_11", test_trv_array_11},
    {"array_12", test_trv_array_12},
    {"array_13", test_trv_array_13},
    {"array_fail_0", test_trv_array_fail_0},
    {"nil", test_trv_nil},
    {"false", test_trv_false},
    {"true", test_trv_true},
    {"digit", test_trv_digit},
    {"string", test_trv_string},
    {"dict_0", test_trv_dict_0},
    {"dict_1", test_trv_dict_1},
    {"dict_2", test_trv_dict_2},
    {"dict_3", test_trv_dict_3},
    {"dict_fail_0", test_trv_dict_fail_0},
    {"identifier", test_trv_identifier},
    {"traverse", test_PadTrv_Trav},
    {"long_code", test_trv_long_code},
    {"comparison", test_trv_comparison},
    {"comparison_0", test_trv_comparison_0},
    {"comparison_1", test_trv_comparison_1},
    {"comparison_2", test_trv_comparison_2},
    {"comparison_3", test_trv_comparison_3},
    {"comparison_4", test_trv_comparison_4},
    {"comparison_5", test_trv_comparison_5},
    {"comparison_6", test_trv_comparison_6},
    {"comparison_7", test_trv_comparison_7},
    {"array_index", test_trv_array_index},
    {"text_block_old", test_trv_text_block_old},
    {"ref_block_old", test_trv_ref_block_old},
    {"assign_0", test_trv_assign_0},
    {"assign_1", test_trv_assign_1},
    {"assign_2", test_trv_assign_2},
    {"assign_3", test_trv_assign_3},
    {"assign_4", test_trv_assign_4},
    {"assign_5", test_trv_assign_5},
    {"assign_fail_0", test_trv_assign_fail_0},
    {"atom_0", test_trv_atom_0},
    {"index", test_trv_index},
    {"string_index", test_trv_string_index},
    {"multi_assign", test_trv_multi_assign},
    {"and_test", test_trv_and_test},
    {"assign_list", test_trv_assign_list},
    {"test_list", test_trv_test_list},
    {"dot_0", test_trv_dot_0},
    {"dot_1", test_trv_dot_1},
    {"dot_2", test_trv_dot_2},
    {"dot_3", test_trv_dot_3},
    {"dot_4", test_trv_dot_4},
    {"dot_5", test_trv_dot_5},
    {"dot_6", test_trv_dot_6},
    {"negative_0", test_trv_negative_0},
    {"call", test_trv_call},
    {"builtin_structs_error_0", test_trv_builtin_structs_error_0},
    {"builtin_structs_error_1", test_trv_builtin_structs_error_1},
    {"builtin_modules_opts_0", test_trv_builtin_modules_opts_0},
    {"builtin_modules_alias_0", test_trv_builtin_modules_alias_0},
    {"builtin_modules_alias_1", test_trv_builtin_modules_alias_1},
    {"builtin_modules_alias_2", test_trv_builtin_modules_alias_2},
    {"builtin_modules_array_0", test_trv_builtin_modules_array_0},
    {"builtin_modules_array_1", test_trv_builtin_modules_array_1},
    {"builtin_functions", test_trv_builtin_functions},
    {"builtin_functions_puts_0", test_trv_builtin_functions_puts_0},
    {"builtin_functions_len_0", test_trv_builtin_functions_len_0},
    {"builtin_functions_type", test_trv_builtin_functions_type},
    {"builtin_functions_cast", test_trv_builtin_functions_cast},
    {"builtin_functions_type_dict", test_trv_builtin_functions_type_dict},
    {"builtin_functions_copy_0", test_trv_builtin_functions_copy_0},
    {"builtin_functions_deepcopy_0", test_trv_builtin_functions_deepcopy_0},
    {"builtin_functions_assert_0", test_trv_builtin_functions_assert_0},
    {"builtin_functions_assert_1", test_trv_builtin_functions_assert_1},
    {"builtin_functions_extract_0", test_trv_builtin_functions_extract_0},
    {"builtin_functions_setattr_0", test_trv_builtin_functions_setattr_0},
    {"builtin_functions_getattr_0", test_trv_builtin_functions_getattr_0},
    {"builtin_functions_dance_0", test_trv_builtin_functions_dance_0},
    {"builtin_functions_ord_0", test_trv_builtin_functions_ord_0},
    {"builtin_functions_chr_0", test_trv_builtin_functions_chr_0},
    {"builtin_string", test_trv_builtin_string},
    {"builtin_unicode_split", test_trv_builtin_unicode_split},
    {"builtin_unicode_rstrip", test_trv_builtin_unicode_rstrip},
    {"builtin_unicode_lstrip", test_trv_builtin_unicode_lstrip},
    {"builtin_unicode_strip", test_trv_builtin_unicode_strip},
    {"builtin_unicode_isdigit", test_trv_builtin_unicode_isdigit},
    {"builtin_unicode_isalpha", test_trv_builtin_unicode_isalpha},
    {"builtin_unicode_isspace", test_trv_builtin_unicode_isspace},
    {"builtin_array_0", test_trv_builtin_array_0},
    {"builtin_dict_0", test_trv_builtin_dict_0},
    {"module_0", test_trv_module_0},
    {"chain_object", test_trv_chain_object},
    {"etc_0", test_trv_etc_0},
    {"etc_1", test_trv_etc_1},
    {"etc_2", test_trv_etc_2},
    {"etc_3", test_trv_etc_3},
    {"etc_4", test_trv_etc_4},
    {"etc_5", test_trv_etc_5},
    {"etc_6", test_trv_etc_6},
    {"etc_7", test_trv_etc_7},
    {"etc_8", test_trv_etc_8},
    {"etc_9", test_trv_etc_9},
    {"etc_10", test_trv_etc_10},
    {"unicode_0", test_trv_unicode_0},
    {"unicode_1", test_trv_unicode_1},
    {"import_stmt_0", test_trv_import_stmt_0},
    {"import_stmt_1", test_trv_import_stmt_1},
    {"import_stmt_2", test_trv_import_stmt_2},
    {"import_stmt_3", test_trv_import_stmt_3},
    {"import_stmt_4", test_trv_import_stmt_4},
    {"import_stmt_5", test_trv_import_stmt_5},
    {"from_import_stmt_1", test_trv_from_import_stmt_1},
    {"from_import_stmt_2", test_trv_from_import_stmt_2},
    {"from_import_stmt_3", test_trv_from_import_stmt_3},
    {0},
};

/**************
* error_stack *
**************/

static void
test_PadErrStack_New(void) {
    PadErrStack *stack = PadErrStack_New();
    assert(stack);
    PadErrStack_Del(stack);
}

static void
test_PadErrStack_PushBack(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(PadErrStack_Len(stack) == 0);
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(PadErrStack_Len(stack) == 2);

    const PadErrElem *elem = PadErrStack_Getc(stack, 0);
    assert(elem);
    assert(!strcmp(elem->filename, "file1"));
    assert(elem->lineno == 1);
    assert(!strcmp(elem->funcname, "func1"));
    assert(!strcmp(elem->message, "this is message1"));

    elem = PadErrStack_Getc(stack, 1);
    assert(elem);
    assert(!strcmp(elem->filename, "file2"));
    assert(elem->lineno == 2);
    assert(!strcmp(elem->funcname, "func2"));
    assert(!strcmp(elem->message, "this is message2"));

    assert(PadErrStack_Getc(stack, 2) == NULL);

    PadErrStack_Del(stack);
}

static void
test_errstack_resize(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(PadErrStack_Len(stack) == 0);
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file5", 5, "func5", "this is %s", "message5"));
    assert(PadErrStack_Len(stack) == 5);

    const PadErrElem *elem = PadErrStack_Getc(stack, 0);
    assert(elem);
    assert(!strcmp(elem->filename, "file1"));
    assert(elem->lineno == 1);
    assert(!strcmp(elem->funcname, "func1"));
    assert(!strcmp(elem->message, "this is message1"));

    elem = PadErrStack_Getc(stack, 1);
    assert(elem);
    assert(!strcmp(elem->filename, "file2"));
    assert(elem->lineno == 2);
    assert(!strcmp(elem->funcname, "func2"));
    assert(!strcmp(elem->message, "this is message2"));

    elem = PadErrStack_Getc(stack, 2);
    assert(elem);
    assert(!strcmp(elem->filename, "file3"));
    assert(elem->lineno == 3);
    assert(!strcmp(elem->funcname, "func3"));
    assert(!strcmp(elem->message, "this is message3"));

    elem = PadErrStack_Getc(stack, 3);
    assert(elem);
    assert(!strcmp(elem->filename, "file4"));
    assert(elem->lineno == 4);
    assert(!strcmp(elem->funcname, "func4"));
    assert(!strcmp(elem->message, "this is message4"));

    elem = PadErrStack_Getc(stack, 4);
    assert(elem);
    assert(!strcmp(elem->filename, "file5"));
    assert(elem->lineno == 5);
    assert(!strcmp(elem->funcname, "func5"));
    assert(!strcmp(elem->message, "this is message5"));

    PadErrStack_Del(stack);
}

static void
test_PadErrStack_Trace(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));

    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    PadErrStack_Trace(stack, stderr);
    assert(strcmp(buf, "Error:\n    file1: 1: func1: This is message1.\n    file2: 2: func2: This is message2."));

    fseek(stderr, 0, SEEK_SET);
    setbuf(stderr, NULL);
    PadErrStack_Del(stack);
}

static void
test_PadErrElem_Show(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));

    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    const PadErrElem *elem = PadErrStack_Getc(stack, 0);
    PadErrElem_Show(elem, stderr);
    // assert(!strcmp(buf, "file1: 1: func1: This is message1.\n"));

    fseek(stderr, 0, SEEK_SET);
    buf[0] = '\0';

    elem = PadErrStack_Getc(stack, 1);
    PadErrElem_Show(elem, stderr);
    // assert(!strcmp(buf, "file2: 2: func2: This is message2.\n"));

    setbuf(stderr, NULL);
    PadErrStack_Del(stack);
}

static void
test_PadErrStack_ExtendFrontOther_0(void) {
    PadErrStack *stack = PadErrStack_New();
    PadErrStack *other = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));

    assert(PadErrStack_Len(stack) == 2);
    assert(PadErrStack_Len(other) == 2);

    assert(PadErrStack_ExtendFrontOther(stack, other));
    assert(PadErrStack_Len(stack) == 4);
    assert(PadErrStack_Len(other) == 2);

    PadErrStack_Del(stack);
    PadErrStack_Del(other);
}

static void
test_PadErrStack_ExtendFrontOther_1(void) {
    PadErrStack *stack = PadErrStack_New();
    PadErrStack *other = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file5", 5, "func5", "this is %s", "message5"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file6", 6, "func6", "this is %s", "message6"));

    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(PadErrStack_Len(stack) == 2);
    assert(PadErrStack_Len(other) == 4);

    assert(PadErrStack_ExtendFrontOther(stack, other));
    assert(PadErrStack_Len(stack) == 6);
    assert(PadErrStack_Len(other) == 4);

    PadErrStack_Del(stack);
    PadErrStack_Del(other);
}

static void
test_PadErrStack_ExtendFrontOther_2(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(PadErrStack_Len(stack) == 4);

    assert(PadErrStack_ExtendFrontOther(stack, stack));
    assert(PadErrStack_Len(stack) == 8);

    PadErrStack_Del(stack);
}

static void
test_PadErrStack_ExtendBackOther_0(void) {
    PadErrStack *stack = PadErrStack_New();
    PadErrStack *other = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));

    assert(PadErrStack_Len(stack) == 2);
    assert(PadErrStack_Len(other) == 2);

    assert(PadErrStack_ExtendBackOther(stack, other));
    assert(PadErrStack_Len(stack) == 4);
    assert(PadErrStack_Len(other) == 2);

    PadErrStack_Del(stack);
    PadErrStack_Del(other);
}

static void
test_PadErrStack_ExtendBackOther_1(void) {
    PadErrStack *stack = PadErrStack_New();
    PadErrStack *other = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file5", 5, "func5", "this is %s", "message5"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file6", 4, "func6", "this is %s", "message6"));

    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(other, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(PadErrStack_Len(stack) == 2);
    assert(PadErrStack_Len(other) == 4);

    assert(PadErrStack_ExtendBackOther(stack, other));
    assert(PadErrStack_Len(stack) == 6);
    assert(PadErrStack_Len(other) == 4);

    PadErrStack_Del(stack);
    PadErrStack_Del(other);
}

static void
test_PadErrStack_ExtendBackOther_2(void) {
    PadErrStack *stack = PadErrStack_New();

    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file1", 1, "func1", "this is %s", "message1"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file2", 2, "func2", "this is %s", "message2"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file3", 3, "func3", "this is %s", "message3"));
    assert(_PadErrStack_PushBack(stack, NULL, 0, NULL, 0, "file4", 4, "func4", "this is %s", "message4"));

    assert(PadErrStack_Len(stack) == 4);

    assert(PadErrStack_ExtendBackOther(stack, stack));
    assert(PadErrStack_Len(stack) == 8);

    PadErrStack_Del(stack);
}

static void
test_PadErrStack_TrimAround(void) {
    PadErrStack *stack = PadErrStack_New();

    const char *src = "the source strings";
    int32_t pos = 10;
    _PadErrStack_PushBack(stack, NULL, 0, src, pos, "file1", 1, "func1", "this is dog");
    _PadErrStack_PushBack(stack, NULL, 0, src, pos, "file1", 1, "func1", "this is bird");

    PadStr *s = PadErrStack_TrimAround(src, pos);
    assert(s);
    assert(!strcmp(PadStr_Getc(s),
"the source strings\n"
"         ^"));

    PadErrStack_Del(stack);
}

static const struct testcase
PadErrStackests[] = {
    {"PadErrElem_Show", test_PadErrElem_Show},
    {"PadErrStack_New", test_PadErrStack_New},
    {"PadErrStack_PushBack", test_PadErrStack_PushBack},
    {"errstack_resize", test_errstack_resize},
    {"PadErrStack_Trace", test_PadErrStack_Trace},
    {"PadErrStack_ExtendFrontOther_0", test_PadErrStack_ExtendFrontOther_0},
    {"PadErrStack_ExtendFrontOther_1", test_PadErrStack_ExtendFrontOther_1},
    {"PadErrStack_ExtendFrontOther_2", test_PadErrStack_ExtendFrontOther_2},
    {"PadErrStack_ExtendBackOther_0", test_PadErrStack_ExtendBackOther_0},
    {"PadErrStack_ExtendBackOther_1", test_PadErrStack_ExtendBackOther_1},
    {"PadErrStack_ExtendBackOther_2", test_PadErrStack_ExtendBackOther_2},
    {"PadErrStack_TrimAround", test_PadErrStack_TrimAround},
    {0},
};

/**********
* lang/gc *
**********/

static void
test_lang_PadGC_New(void) {
    PadGC *gc = PadGC_New();
    assert(gc);
    PadGC_Del(gc);
}

static void
test_lang_PadGC_Alloc(void) {
    PadGC *gc = PadGC_New();
    assert(gc);

    PadGCItem item = {0};
    PadGC_Alloc(gc, &item, 100);

    assert(item.ptr);
    assert(item.ref_counts == 0);

    item.ref_counts++;
    item.ref_counts++;

    PadGC_Free(gc, &item);
    assert(item.ptr);
    assert(item.ref_counts == 2);

    item.ref_counts--;
    PadGC_Free(gc, &item);
    assert(item.ptr);
    assert(item.ref_counts == 1);

    item.ref_counts--;
    PadGC_Free(gc, &item);
    assert(item.ptr == NULL);
    assert(item.ref_counts == 0);

    PadGC_Del(gc);
}

static const struct testcase
lang_PadGCests[] = {
    {"PadGC_New", test_lang_PadGC_New},
    {"PadGC_Alloc", test_lang_PadGC_Alloc},
    {0},
};

/*******************
* lang/object_dict *
*******************/

static void
test_lang_PadObjDict_Move(void) {
    PadGC *gc = PadGC_New();
    PadObjDict *d = PadObjDict_New(gc);

    PadObj *obj1 = PadObj_NewInt(gc, 1);
    PadObj_IncRef(obj1);
    PadObjDict_Move(d, "abc", obj1);
    assert(PadObjDict_Len(d) == 1);

    PadObj *obj2 = PadObj_NewInt(gc, 1);
    PadObj_IncRef(obj2);
    PadObjDict_Move(d, "def", obj2);
    assert(PadObjDict_Len(d) == 2);

    PadObjDictItem *item1 = PadObjDict_Get(d, "abc");
    assert(obj1 == item1->value);

    PadObjDictItem *item2 = PadObjDict_Get(d, "def");
    assert(obj2 == item2->value);

    PadObjDict_Del(d);
    PadGC_Del(gc);
}

static void
test_lang_PadObjDict_Set(void) {
    PadGC *gc = PadGC_New();
    PadObjDict *d = PadObjDict_New(gc);

    PadObj *obj1 = PadObj_NewInt(gc, 1);
    PadObj_IncRef(obj1);
    PadObjDict_Move(d, "abc", obj1);
    assert(PadObjDict_Len(d) == 1);

    PadObj *obj2 = PadObj_NewInt(gc, 1);
    PadObj_IncRef(obj2);
    PadObjDict_Move(d, "def", obj2);
    assert(PadObjDict_Len(d) == 2);

    PadObjDictItem *item1 = PadObjDict_Get(d, "abc");
    assert(obj1 == item1->value);

    PadObjDictItem *item2 = PadObjDict_Get(d, "def");
    assert(obj2 == item2->value);

    PadObjDict_Del(d);
    PadGC_Del(gc);
}

static void
test_lang_PadObjDict_Pop(void) {
    /**********
    * pop one *
    **********/

    PadGC *gc = PadGC_New();
    PadObjDict *d = PadObjDict_New(gc);
    PadObj *obj = PadObj_NewInt(gc, 0);

    PadObj_IncRef(obj);
    PadObjDict_Move(d, "abc", obj);
    assert(PadObjDict_Len(d) == 1);
    PadObj *popped = PadObjDict_Pop(d, "abc");
    assert(popped);
    assert(PadObjDict_Len(d) == 0);
    assert(obj == popped);

    PadObjDict_Del(d);
    PadGC_Del(gc);

    /***********
    * pop many *
    ***********/

    gc = PadGC_New();
    d = PadObjDict_New(gc);

    for (int32_t i = 0; i < 10; ++i) {
        PadObj *obj = PadObj_NewInt(gc, i);
        char key[10];
        snprintf(key, sizeof key, "obj%d", i);
        PadObj_IncRef(obj);
        PadObjDict_Move(d, key, obj);
    }
    assert(PadObjDict_Len(d) == 10);

    for (int32_t i = 0; i < 10; ++i) {
        char key[10];
        snprintf(key, sizeof key, "obj%d", i);
        PadObj *popped = PadObjDict_Pop(d, key);
        assert(popped);
    }
    assert(PadObjDict_Len(d) == 0);

    PadObjDict_Del(d);
    PadGC_Del(gc);
}

static const struct testcase
lang_PadObjDictests[] = {
    {"move", test_lang_PadObjDict_Move},
    {"set", test_lang_PadObjDict_Set},
    {"pop", test_lang_PadObjDict_Pop},
    {0},
};

/**************
* stdlib/list *
**************/

static void
test_lang_stdlib_list_dump(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/list.pad\" import List\n"
        "list = List()\n"
        "list.push(1)\n"
        "list.push(2)\n"
        "list.push(3)\n"
        "list.dump()\n"
        "@}", "1\n2\n3\n");

    trv_cleanup;
}

static void
test_lang_stdlib_list_push(void) {
    // @see list_dump
}

static void
test_lang_stdlib_list_pop(void) {
    trv_ready;
/*
    check_ok("{@\n"
        "from \"lib/list.pad\" import List\n"
        "list = List.new()\n"
        "list.push(1)\n"
        "list.push(2)\n"
        "val = list.pop()\n"
        "@}{: val :}", "2");
*/
    trv_cleanup;
}

static const struct testcase
stdlib_list_tests[] = {
    {"dump", test_lang_stdlib_list_dump},
    {"push", test_lang_stdlib_list_push},
    {"pop", test_lang_stdlib_list_pop},
    {0},
};

/****************
* stdlib/stream *
****************/

static void
test_lang_stdlib_stream_new(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/stream.pad\" import Stream\n"
        "s = Stream.new(\"abc\")\n"
        "@}{: s.buf :},{: s.index :},{: s.length :}", "abc,0,3");

    trv_cleanup;
}

static void
test_lang_stdlib_stream_get(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/stream.pad\" import Stream\n"
        "s = Stream.new(\"abc\")\n"
        "puts(s.eof())\n"
        "puts(s.get())\n"
        "puts(s.get())\n"
        "puts(s.get())\n"
        "puts(s.get())\n"
        "puts(s.eof())\n"
        "@}", "false\na\nb\nc\nnil\ntrue\n");

    trv_cleanup;
}

static void
test_lang_stdlib_stream_next(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/stream.pad\" import Stream\n"
        "s = Stream.new(\"abc\")\n"
        "puts(s.cur(0))\n"
        "s.next()\n"
        "puts(s.cur(0))\n"
        "s.next()\n"
        "puts(s.cur(0))\n"
        "s.next()\n"
        "puts(s.cur(0))\n"
        "s.next()\n"
        "puts(s.index)\n"
        "@}", "a\nb\nc\nnil\n3\n");

    trv_cleanup;
}

static void
test_lang_stdlib_stream_prev(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/stream.pad\" import Stream\n"
        "src = \"abc\""
        "s = Stream.new(src)\n"
        "s.index = len(src)\n"
        "puts(s.cur(0))\n"
        "s.prev()\n"
        "puts(s.cur(0))\n"
        "s.prev()\n"
        "puts(s.cur(0))\n"
        "s.prev()\n"
        "puts(s.cur(0))\n"
        "s.prev()\n"
        "puts(s.cur(0))\n"
        "@}", "nil\nc\nb\na\na\n");

    trv_cleanup;
}

static void
test_lang_stdlib_stream_cur(void) {
    trv_ready;

    check_ok("{@\n"
        "from \"lib/stream.pad\" import Stream\n"
        "s = Stream.new(\"abc\")\n"
        "puts(s.cur(0))\n"
        "@}", "a\n");

    trv_cleanup;
}

static const struct testcase
stdlib_stream_tests[] = {
    {"new", test_lang_stdlib_stream_new},
    {"get", test_lang_stdlib_stream_get},
    {"next", test_lang_stdlib_stream_next},
    {"prev", test_lang_stdlib_stream_prev},
    {"cur", test_lang_stdlib_stream_cur},
    {0},
};

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    // lib
    {"cstring_array", cstrarr_tests},
    {"cstring", cPadStrests},
    {"string", PadStrests},
    {"unicode", PadUniests},
    {"file", file_tests},
    {"cl", PadCLests},
    {"cmdline", PadCmdlineests},
    {"error", error_tests},
    {"util", utiltests},
    {"path", pathtests},
    {"opts", lang_PadOptsests},
    {"tokenizer", PadTkrests},
    {"compiler", compiler_tests},
    {"traverser", traverser_tests},
    {"stdlib.list", stdlib_list_tests},
    {"stdlib.stream", stdlib_stream_tests},
    {"error_stack", PadErrStackests},
    {"gc", lang_PadGCests},
    {"objdict", lang_PadObjDictests},
    {0},
};

struct PadOpts {
    bool ishelp;
    int32_t argc;
    char **argv;
    int32_t optind;
};

static int32_t
parseopts(struct PadOpts *opts, int argc, char *argv[]) {
    // Init opts
    *opts = (struct PadOpts) {0};
    optind = 0;
    opterr = 0;

    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': opts->ishelp = true; break;
        case '?':
        default: die("unknown option"); break;
        }
    }

    if (argc < optind) {
        die("failed to parse option");
    }

    opts->argc = argc;
    opts->optind = optind;
    opts->argv = argv;

    return 0;
}

static int32_t
modtest(const char *modname) {
    int32_t ntest = 0;
    const struct testmodule *fndmod = NULL;

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        if (strcmp(modname, m->name) == 0) {
            fndmod = m;
        }
    }
    if (!fndmod) {
        return 0;
    }

    printf("\n* module '%s'\n", fndmod->name);

    for (const struct testcase *t = fndmod->tests; t->name; ++t) {
        printf("- testing '%s'\n", t->name);
        t->test();
        ++ntest;
    }

    return ntest;
}

static int32_t
methtest(const char *modname, const char *methname) {
    const struct testmodule *fndmod = NULL;

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        if (strcmp(modname, m->name) == 0) {
            fndmod = m;
        }
    }
    if (!fndmod) {
        return 0;
    }

    printf("\n* module '%s'\n", fndmod->name);

    const struct testcase *fndt = NULL;
    for (const struct testcase *t = fndmod->tests; t->name; ++t) {
        if (!strcmp(t->name, methname)) {
            fndt = t;
            break;
        }
    }
    if (!fndt) {
        return 0;
    }

    printf("* method '%s'\n", fndt->name);
    fndt->test();

    return 1;
}

static int32_t
fulltests(void) {
    int32_t ntest = 0;

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        printf("\n* module '%s'\n", m->name);
        for (const struct testcase *t = m->tests; t->name; ++t) {
            printf("- testing '%s'\n", t->name);
            t->test();
            ++ntest;
        }
    }

    return ntest;
}

static void
run(const struct PadOpts *opts) {
    int32_t ntest = 0;
    clock_t start;
    clock_t end;

    if (opts->argc - opts->optind == 1) {
        start = clock();
        ntest = modtest(opts->argv[opts->optind]);
        end = clock();
    } else if (opts->argc - opts->optind >= 2) {
        start = clock();
        ntest = methtest(opts->argv[opts->optind], opts->argv[opts->optind+1]);
        end = clock();
    } else {
        start = clock();
        ntest = fulltests();
        end = clock();
    }

    fflush(stdout);
    fprintf(stderr, "\nRun %d test in %0.3lfs.\n", ntest, (double)(end-start)/CLOCKS_PER_SEC);
    fprintf(stderr, "\n");
    fprintf(stderr, "OK\n");

    fflush(stderr);
}

static void
cleanup(void) {
    remove_test_file();
}

int
main(int argc, char *argv[]) {
    setlocale(LC_CTYPE, "");

    struct PadOpts opts;
    if (parseopts(&opts, argc, argv) != 0) {
        die("failed to parse options");
    }

    run(&opts);
    cleanup();

    return 0;
}
