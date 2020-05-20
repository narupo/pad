/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include <tests.h>

/*********
* macros *
*********/

#define showbuf() printf("stdout[%s]\n", ctx_getc_stdout_buf(ctx))
#define showerr() printf("stderr[%s]\n", ctx_getc_stderr_buf(ctx))

#define showdetail() printf("detail[%s]\n", ast_getc_first_error_message(ast))
#define ERR errstack_trace(ast->error_stack, stderr)

#define ast_debug(stmt) { \
    ast_set_debug(ast, true); \
    stmt; \
    ast_set_debug(ast, false); \
} \

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
    char tmp[FILE_NPATH] = {0};
    assert(file_solve(tmp, sizeof tmp, path));
    snprintf(dst, dstsz, "%s", tmp);
    return dst;
}

#define trv_ready \
    config_t *config = config_new(); \
    tokenizer_option_t *opt = tkropt_new(); \
    tokenizer_t *tkr = tkr_new(mem_move(opt)); \
    ast_t *ast = ast_new(config); \
    gc_t *gc = gc_new(); \
    context_t *ctx = ctx_new(gc); \

#define trv_cleanup \
    ctx_del(ctx); \
    gc_del(gc); \
    ast_del(ast); \
    tkr_del(tkr); \
    config_del(config); \

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
test_cstrarr_new(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);
    cstrarr_del(arr);
}

void
test_cstrarr_escdel(void) {
    // test
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_escdel(NULL) == NULL);

    char **escarr = cstrarr_escdel(arr);
    assert(escarr != NULL);

    int i;
    for (i = 0; escarr[i]; ++i) {
    }
    assert(i == 0);
    _freeescarr(escarr);

    // test
    arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_push(arr, "0") != NULL);
    assert(cstrarr_push(arr, "1") != NULL);
    assert(cstrarr_push(arr, "2") != NULL);

    escarr = cstrarr_escdel(arr);
    assert(_countescarr(escarr) == 3);
    assert(strcmp(escarr[0], "0") == 0);
    assert(strcmp(escarr[1], "1") == 0);
    assert(strcmp(escarr[2], "2") == 0);
    _freeescarr(escarr);
}

void
test_cstrarr_push(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_push(NULL, "1") == NULL);
    assert(cstrarr_push(arr, NULL) == NULL);
    assert(cstrarr_push(arr, "") != NULL);
    assert(cstrarr_push(arr, "1") != NULL);

    assert(cstrarr_len(arr) == 2);

    cstrarr_del(arr);
}

void
test_cstrarr_move(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_move(arr, NULL) != NULL);
    assert(cstrarr_getc(arr, 0) == NULL);

    char *ptr = cstr_edup("string");
    assert(ptr != NULL);

    assert(cstrarr_move(arr, ptr) != NULL);
    assert(strcmp(cstrarr_getc(arr, 1), "string") == 0);

    cstrarr_del(arr);
}

void
test_cstrarr_sort(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_sort(NULL) == NULL);

    assert(cstrarr_push(arr, "1") != NULL);
    assert(cstrarr_push(arr, "2") != NULL);
    assert(cstrarr_push(arr, "0") != NULL);

    assert(cstrarr_sort(arr) != NULL);
    assert(strcmp(cstrarr_getc(arr, 0), "0") == 0);
    assert(strcmp(cstrarr_getc(arr, 1), "1") == 0);
    assert(strcmp(cstrarr_getc(arr, 2), "2") == 0);

    cstrarr_del(arr);
}

void
test_cstrarr_getc(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_getc(NULL, 0) == NULL);
    assert(cstrarr_getc(arr, 0) == NULL);
    assert(cstrarr_getc(arr, -1) == NULL);

    assert(cstrarr_push(arr, "0") != NULL);
    assert(cstrarr_push(arr, "1") != NULL);
    assert(cstrarr_push(arr, "2") != NULL);

    assert(strcmp(cstrarr_getc(arr, 0), "0") == 0);
    assert(strcmp(cstrarr_getc(arr, 1), "1") == 0);
    assert(strcmp(cstrarr_getc(arr, 2), "2") == 0);
    assert(cstrarr_getc(arr, 3) == NULL);

    cstrarr_del(arr);
}

void
test_cstrarr_len(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_len(NULL) == 0);
    assert(cstrarr_len(arr) == 0);

    assert(cstrarr_push(arr, "0") != NULL);
    assert(cstrarr_push(arr, "1") != NULL);
    assert(cstrarr_push(arr, "2") != NULL);
    assert(cstrarr_len(arr) == 3);

    cstrarr_del(arr);
}

void
test_cstrarr_show(void) {
    cstring_array_t *arr = cstrarr_new();
    assert(arr != NULL);

    assert(cstrarr_show(NULL, stdout) == NULL);
    assert(cstrarr_show(arr, NULL) == NULL);
    assert(cstrarr_show(arr, stdout) != NULL);

    cstrarr_del(arr);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
cstrarr_tests[] = {
    {"cstrarr_new", test_cstrarr_new},
    {"cstrarr_escdel", test_cstrarr_escdel},
    {"cstrarr_push", test_cstrarr_push},
    {"cstrarr_move", test_cstrarr_move},
    {"cstrarr_sort", test_cstrarr_sort},
    {"cstrarr_getc", test_cstrarr_getc},
    {"cstrarr_len", test_cstrarr_len},
    {"cstrarr_show", test_cstrarr_show},
    {0},
};

/**********
* cmdline *
**********/

void
test_cmdline_new(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);
    cmdline_del(cmdline);
}

void
test_cmdline_del(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);
    cmdline_del(cmdline);
}

void
test_cmdline_parse_0(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    assert(cmdline_parse(cmdline, "abc"));

    cmdline_del(cmdline);
}

void
test_cmdline_parse_1(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    assert(cmdline_parse(cmdline, "abc && def"));

    cmdline_del(cmdline);
}

void
test_cmdline_parse_2(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    assert(cmdline_parse(cmdline, "abc | def"));

    cmdline_del(cmdline);
}

void
test_cmdline_parse_3(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    assert(cmdline_parse(cmdline, "abc > def"));

    cmdline_del(cmdline);
}

void
test_cmdline_parse(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    const cmdline_object_t *obj = NULL;

    assert(cmdline_parse(cmdline, "abc && def | ghi > jkl"));
    assert(cmdline_len(cmdline) == 7);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_AND);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "def"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 3);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_PIPE);
    obj = cmdline_getc(cmdline, 4);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "ghi"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 5);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_REDIRECT);
    obj = cmdline_getc(cmdline, 6);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "jkl"));
    assert(cl_len(obj->cl) == 1);

    cmdline_del(cmdline);
}

void
test_cmdline_parse_pipe(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    const cmdline_object_t *obj = NULL;

    assert(cmdline_parse(cmdline, "abc"));
    assert(cmdline_len(cmdline) == 1);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc"));
    assert(cl_len(obj->cl) == 1);

    assert(cmdline_parse(cmdline, "abc | def"));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_PIPE);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "def"));
    assert(cl_len(obj->cl) == 1);

    assert(cmdline_parse(cmdline, "abc -d efg | hij -d \"klm\""));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc -d efg"));
    assert(cl_len(obj->cl) == 3);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_PIPE);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "hij -d \"klm\""));
    assert(cl_len(obj->cl) == 3);

    assert(cmdline_parse(cmdline, "a | b | c | d | e"));
    assert(cmdline_len(cmdline) == 9);

    cmdline_del(cmdline);
}

void
test_cmdline_parse_and(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    const cmdline_object_t *obj = NULL;

    assert(cmdline_parse(cmdline, "abc && def"));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_AND);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "def"));
    assert(cl_len(obj->cl) == 1);

    assert(cmdline_parse(cmdline, "abc -d efg && hij -d \"klm\""));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc -d efg"));
    assert(cl_len(obj->cl) == 3);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_AND);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "hij -d \"klm\""));
    assert(cl_len(obj->cl) == 3);

    assert(cmdline_parse(cmdline, "a && b && c && d && e"));
    assert(cmdline_len(cmdline) == 9);

    cmdline_del(cmdline);
}

void
test_cmdline_parse_redirect(void) {
    cmdline_t *cmdline = cmdline_new();
    assert(cmdline);

    const cmdline_object_t *obj = NULL;

    assert(cmdline_parse(cmdline, "abc > def"));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc"));
    assert(cl_len(obj->cl) == 1);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_REDIRECT);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "def"));
    assert(cl_len(obj->cl) == 1);

    assert(cmdline_parse(cmdline, "abc -d efg > hij -d \"klm\""));
    assert(cmdline_len(cmdline) == 3);
    obj = cmdline_getc(cmdline, 0);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "abc -d efg"));
    assert(cl_len(obj->cl) == 3);
    obj = cmdline_getc(cmdline, 1);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_REDIRECT);
    obj = cmdline_getc(cmdline, 2);
    assert(obj);
    assert(obj->type == CMDLINE_OBJECT_TYPE_CMD);
    assert(!strcmp(str_getc(obj->command), "hij -d \"klm\""));
    assert(cl_len(obj->cl) == 3);

    assert(cmdline_parse(cmdline, "a > b > c > d > e"));
    assert(cmdline_len(cmdline) == 9);

    cmdline_del(cmdline);
}

static const struct testcase
cmdline_tests[] = {
    {"cmdline_new", test_cmdline_new},
    {"cmdline_del", test_cmdline_del},
    {"cmdline_parse", test_cmdline_parse},
    {"cmdline_parse_0", test_cmdline_parse_0},
    {"cmdline_parse_1", test_cmdline_parse_1},
    {"cmdline_parse_2", test_cmdline_parse_2},
    {"cmdline_parse_3", test_cmdline_parse_3},
    {"cmdline_parse_pipe", test_cmdline_parse_pipe},
    {"cmdline_parse_and", test_cmdline_parse_and},
    {"cmdline_parse_redirect", test_cmdline_parse_redirect},
    {0},
};

/*********
* string *
*********/

static void
test_cstring_cstr_app(void) {
    char dst[100] = {0};

    assert(cstr_app(dst, sizeof dst, NULL) == NULL);
    assert(cstr_app(NULL, sizeof dst, "source") == NULL);
    assert(cstr_app(dst, 0, "source") == NULL);

    assert(cstr_app(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(cstr_app(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(cstr_app(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(cstr_app(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
}

static void
test_cstring_cstr_app_fmt(void) {
    char dst[100] = {0};

    assert(cstr_app_fmt(dst, sizeof dst, NULL) == NULL);
    assert(cstr_app_fmt(NULL, sizeof dst, "source") == NULL);
    assert(cstr_app_fmt(dst, 0, "source") == NULL);

    assert(cstr_app_fmt(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(cstr_app_fmt(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(cstr_app_fmt(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(cstr_app_fmt(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);

    *dst = '\0';
    assert(cstr_app_fmt(dst, sizeof dst, "n %d is %c", 10, 'i') != NULL);
    assert(strcmp(dst, "n 10 is i") == 0);
}

static void
test_cstring_cstr_cpywithout(void) {
    char dst[100];

    assert(cstr_cpywithout(NULL, sizeof dst, "abc123def456", "") == NULL);
    assert(cstr_cpywithout(dst, 0, "abc123def456", "") == NULL);
    assert(cstr_cpywithout(dst, sizeof dst, NULL, "") == NULL);
    assert(cstr_cpywithout(dst, sizeof dst, "abc123def456", NULL) == NULL);

    assert(cstr_cpywithout(dst, sizeof dst, "abc123def456", "") != NULL);
    assert(strcmp(dst, "abc123def456") == 0);
    assert(cstr_cpywithout(dst, sizeof dst, "abc123def456", "123456") != NULL);
    assert(strcmp(dst, "abcdef") == 0);
    assert(cstr_cpywithout(dst, sizeof dst, "abc123def456", "abcdef") != NULL);
    assert(strcmp(dst, "123456") == 0);
}

static void
test_cstring_cstr_edup(void) {
    char *p = cstr_edup("string");
    assert(strcmp(p, "string") == 0);
    free(p);
}

static void
test_str_del(void) {
    string_t *s = str_new();
    assert(s != NULL);
    str_del(NULL);
    str_del(s);
}

static void
test_str_esc_del(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_esc_del(NULL) == NULL);
    char *ptr = str_esc_del(s);
    assert(ptr != NULL);
    free(ptr);
}

static void
test_str_new(void) {
    string_t *s = str_new();
    assert(s != NULL);
    str_del(s);
}

static void
test_str_new_other(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    assert(str_new_other(NULL) == NULL);
    string_t *o = str_new_other(s);
    assert(o != NULL);
    assert(strcmp(str_getc(o), "1234") == 0);
    str_del(o);
    str_del(s);
}

static void
test_str_len(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_len(NULL) == -1);
    assert(str_len(s) == 0);
    assert(str_app(s, "abc") != NULL);
    assert(str_len(s) == 3);
    str_del(s);
}

static void
test_str_capa(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_capa(NULL) == -1);
    assert(str_capa(s) == 4);
    assert(str_app(s, "1234") != NULL);
    assert(str_capa(s) == 8);
    str_del(s);
}

static void
test_str_getc(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_getc(NULL) == NULL);
    assert(strcmp(str_getc(s), "") == 0);
    assert(str_app(s, "1234") != NULL);
    assert(strcmp(str_getc(s), "1234") == 0);
    str_del(s);
}

static void
test_str_empty(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_empty(NULL) == 0);
    assert(str_empty(s) == 1);
    assert(str_app(s, "1234") != NULL);
    assert(str_empty(s) == 0);
    str_del(s);
}

static void
test_str_clear(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_app(NULL, "1234") == NULL);
    assert(str_app(s, NULL) == NULL);
    assert(str_app(s, "1234") != NULL);
    assert(str_len(s) == 4);
    str_clear(s);
    assert(str_len(s) == 0);
    str_del(s);
}

static void
test_str_set(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(NULL, "1234") == NULL);
    assert(str_set(s, NULL) == NULL);
    assert(str_set(s, "1234") != NULL);
    assert(strcmp(str_getc(s), "1234") == 0);
    assert(str_set(s, "12") != NULL);
    assert(strcmp(str_getc(s), "12") == 0);
    str_del(s);
}

static void
test_str_resize(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_capa(NULL) == -1);
    assert(str_capa(s) == 4);
    assert(str_resize(s, 4*2) != NULL);
    assert(str_capa(s) == 8);
    str_del(s);
}

static void
test_str_pushb(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_pushb(NULL, '1') == NULL);
    assert(str_pushb(s, 0) == NULL);
    assert(str_pushb(s, '\0') == NULL);
    assert(str_pushb(s, '1') != NULL);
    assert(str_pushb(s, '2') != NULL);
    assert(strcmp(str_getc(s), "12") == 0);
    str_del(s);
}

static void
test_str_popb(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_popb(NULL) == '\0');
    assert(str_set(s, "1234") != NULL);
    assert(strcmp(str_getc(s), "1234") == 0);
    assert(str_popb(s) == '4');
    assert(str_popb(s) == '3');
    assert(strcmp(str_getc(s), "12") == 0);
    str_del(s);
}

static void
test_str_pushf(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_pushf(NULL, '1') == NULL);
    assert(str_pushf(s, 0) == NULL);
    assert(str_pushf(s, '\0') == NULL);
    assert(str_pushf(s, '1') != NULL);
    assert(str_pushf(s, '2') != NULL);
    assert(strcmp(str_getc(s), "21") == 0);
    str_del(s);
}

static void
test_str_popf(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_popf(NULL) == '\0');
    assert(str_set(s, "1234") != NULL);
    assert(str_popf(s) == '1');
    assert(str_popf(s) == '2');
    assert(strcmp(str_getc(s), "34") == 0);
    str_del(s);
}

static void
test_str_app(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_app(NULL, "1234") == NULL);
    assert(str_app(s, NULL) == NULL);
    assert(str_app(s, "1234") != NULL);
    assert(strcmp(str_getc(s), "1234") == 0);
    str_del(s);
}

static void
test_str_app_stream(void) {
    string_t *s = str_new();
    assert(s != NULL);

    char curdir[1024];
    char path[1024];
    assert(file_realpath(curdir, sizeof curdir, ".") != NULL);
    assert(file_solvefmt(path, sizeof path, "%s/src/tests.c", curdir) != NULL);

    FILE *fin = fopen(path, "r");
    assert(fin != NULL);
    assert(str_app_stream(NULL, fin) == NULL);
    assert(str_app_stream(s, NULL) == NULL);
    assert(str_app_stream(s, fin) != NULL);
    assert(fclose(fin) == 0);

    str_del(s);
}

static void
test_str_app_other(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    string_t *o = str_new();
    assert(o != NULL);
    assert(str_set(o, "1234") != NULL);
    assert(str_app_other(NULL, o) == NULL);
    assert(str_app_other(s, NULL) == NULL);
    assert(str_app_other(s, o) != NULL);
    assert(strcmp(str_getc(s), "12341234") == 0);
    str_del(o);
    str_del(s);

    s = str_new();
    assert(str_set(s, "1234") != NULL);
    assert(str_app_other(s, s) != NULL);
    assert(strcmp(str_getc(s), "12341234") == 0);
    str_del(s);
}

static void
test_str_app_fmt(void) {
    string_t *s = str_new();
    assert(s != NULL);
    char buf[1024];
    assert(str_app_fmt(NULL, buf, sizeof buf, "%s", "test") == NULL);
    assert(str_app_fmt(s, NULL, sizeof buf, "%s", "test") == NULL);
    assert(str_app_fmt(s, buf, 0, "%s", "test") == NULL);
    assert(str_app_fmt(s, buf, sizeof buf, NULL, "test") == NULL);
    assert(str_app_fmt(s, buf, sizeof buf, "%s %d %c", "1234", 1, '2') != NULL);
    assert(strcmp(str_getc(s), "1234 1 2") == 0);
    str_del(s);
}

static void
test_str_rstrip(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    assert(str_rstrip(NULL, "34") == NULL);
    assert(str_rstrip(s, NULL) == NULL);
    assert(str_rstrip(s, "34") != NULL);
    assert(strcmp(str_getc(s), "12") == 0);
    str_del(s);
}

static void
test_str_lstrip(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    assert(str_lstrip(NULL, "12") == NULL);
    assert(str_lstrip(s, NULL) == NULL);
    assert(str_lstrip(s, "12") != NULL);
    assert(strcmp(str_getc(s), "34") == 0);
    str_del(s);
}

static void
test_str_strip(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "--1234--") != NULL);
    assert(str_strip(NULL, "-") == NULL);
    assert(str_strip(s, NULL) == NULL);
    assert(str_strip(s, "-") != NULL);
    assert(strcmp(str_getc(s), "1234") == 0);
    str_del(s);
}

static void
test_str_findc(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    assert(str_findc(NULL, "") == NULL);
    assert(str_findc(s, NULL) == NULL);
    const char *fnd = str_findc(s, "23");
    assert(fnd != NULL);
    assert(strcmp(fnd, "234") == 0);
    str_del(s);
}

static void
test_str_lower(void) {
    assert(str_lower(NULL) == NULL);
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "ABC") != NULL);
    string_t *cp = str_lower(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc"));
    str_del(cp);
    str_del(s);
}

static void
test_str_upper(void) {
    assert(str_upper(NULL) == NULL);
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "abc") != NULL);
    string_t *cp = str_upper(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "ABC"));
    str_del(cp);
    str_del(s);
}

static void
test_str_capitalize(void) {
    assert(str_capitalize(NULL) == NULL);
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "abc") != NULL);
    string_t *cp = str_capitalize(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "Abc"));
    str_del(cp);
    str_del(s);
}

static void
test_str_snake(void) {
    assert(str_snake(NULL) == NULL);
    string_t *s = str_new();
    assert(s != NULL);

    assert(str_set(s, "abc") != NULL);
    string_t *cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "AbcDefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "abc-def-ghi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "_abcDefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "-abcDefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "_-abcDefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi_abc-DefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi_abc_def_ghi"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = str_snake(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc_def_ghi_abc_def_ghi"));
    str_del(cp);

    str_del(s);
}

static void
test_str_camel(void) {
#undef showcp
#define showcp() printf("cp[%s]\n", str_getc(cp))

    assert(str_camel(NULL) == NULL);
    string_t *s = str_new();
    assert(s != NULL);

    assert(str_set(s, "abc") != NULL);
    string_t *cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abc"));
    str_del(cp);

    assert(str_set(s, "ABC") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "aBC"));
    str_del(cp);

    assert(str_set(s, "AFormatB") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "aFormatB"));
    str_del(cp);

    assert(str_set(s, "ABFormat") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "aBFormat"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "AbcDefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "abc-def-ghi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "_abcDefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "-abcDefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "_-abcDefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhi"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi_abc-DefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhiAbcDefGhi"));
    str_del(cp);

    assert(str_set(s, "abcDefGhi__abc--DefGhi") != NULL);
    cp = str_camel(s);
    assert(cp);
    assert(!strcmp(str_getc(cp), "abcDefGhiAbcDefGhi"));
    str_del(cp);

    str_del(s);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
string_tests[] = {
    {"cstr_app", test_cstring_cstr_app},
    {"cstr_app_fmt", test_cstring_cstr_app_fmt},
    {"cstr_cpywithout", test_cstring_cstr_cpywithout},
    {"cstr_edup", test_cstring_cstr_edup},
    {"str_del", test_str_del},
    {"str_esc_del", test_str_esc_del},
    {"str_new", test_str_new},
    {"str_new_other", test_str_new_other},
    {"str_new_other", test_str_new_other},
    {"str_len", test_str_len},
    {"str_capa", test_str_capa},
    {"str_getc", test_str_getc},
    {"str_empty", test_str_empty},
    {"str_clear", test_str_clear},
    {"str_set", test_str_set},
    {"str_resize", test_str_resize},
    {"str_pushb", test_str_pushb},
    {"str_popb", test_str_popb},
    {"str_pushf", test_str_pushf},
    {"str_popf", test_str_popf},
    {"str_app", test_str_app},
    {"str_app_stream", test_str_app_stream},
    {"str_app_other", test_str_app_other},
    {"str_app_fmt", test_str_app_fmt},
    {"str_rstrip", test_str_rstrip},
    {"str_lstrip", test_str_lstrip},
    {"str_strip", test_str_strip},
    {"str_findc", test_str_findc},
    {"str_lower", test_str_lower},
    {"str_upper", test_str_upper},
    {"str_capitalize", test_str_capitalize},
    {"str_snake", test_str_snake},
    {"str_camel", test_str_camel},
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
    static char path[FILE_NPATH];

#ifdef _TESTS_WINDOWS
    char tmp[FILE_NPATH];
    assert(file_get_user_home(tmp, sizeof tmp) != NULL);
    assert(file_solvefmt(path, sizeof path, "%s/cap.test.file", tmp) != NULL);
#else
    assert(file_solve(path, sizeof path, "/tmp/cap.test.file") != NULL);
#endif

    if (!file_exists(path)) {
        FILE *f = file_open(path, "wb");
        assert(f != NULL);
        fprintf(f, "%s", get_test_fcontent());
        assert(file_close(f) == 0);
    }
    return path;
}

static void
remove_test_file(void) {
    const char *path = get_test_finpath();
    if (file_exists(path)) {
        assert(file_remove(path) == 0);
    }
}

static FILE *
get_test_fin(void) {
    FILE *fin = file_open(get_test_finpath(), "rb");
    assert(fin != NULL);
    return fin;
}

static int
get_test_finsize(void) {
    return strlen(get_test_fcontent());
}

static const char *
get_test_dirpath(void) {
    static char path[FILE_NPATH];
#ifdef _TESTS_WINDOWS
    assert(file_get_user_home(path, sizeof path) != NULL);
#else
    assert(file_solve(path, sizeof path, "/tmp") != NULL);
#endif
    return path;
}

static void
test_file_close(void) {
    FILE* f = file_open(get_test_finpath(), "rb");
    assert(f != NULL);
    assert(file_close(NULL) != 0);
    assert(file_close(f) == 0);
}

static void
test_file_open(void) {
    test_file_close();
}

static void
test_file_copy(void) {
    FILE *f = file_open(get_test_finpath(), "rb");
    assert(f != NULL);
    // TODO
    assert(file_close(f) == 0);
}

static void
test_file_closedir(void) {
    DIR *f = file_opendir(get_test_dirpath());
    assert(f != NULL);
    assert(file_closedir(NULL) == -1);
    assert(file_closedir(f) == 0);
}

static void
test_file_opendir(void) {
    test_file_closedir();
}

static void
test_file_realpath(void) {
    char path[FILE_NPATH];

    assert(file_realpath(NULL, sizeof path, "/tmp/../tmp") == NULL);
    assert(file_realpath(path, 0, "/tmp/../tmp") == NULL);
    assert(file_realpath(path, sizeof path, NULL) == NULL);

    char userhome[FILE_NPATH];
    assert(file_get_user_home(userhome, sizeof userhome));

    char src[FILE_NPATH + 5] = {0};
    snprintf(src, sizeof src, "%s%c..", userhome, FILE_SEP);
    assert(file_realpath(path, sizeof path, src) != NULL);
}

static void
test_file_exists(void) {
    assert(file_exists(NULL) == false);
    assert(file_exists(get_test_dirpath()));
    assert(!file_exists("/nothing/directory"));
}

static void
test_file_mkdirmode(void) {
    // TODO
}

static void
test_file_mkdirq(void) {
    assert(file_mkdirq(NULL) != 0);
}

static void
test_file_trunc(void) {
    char path[FILE_NPATH];
    char userhome[FILE_NPATH];
    assert(file_get_user_home(userhome, sizeof userhome) != NULL);
    assert(file_solvefmt(path, sizeof path, "%s/cap.ftrunc", userhome) != NULL);

    assert(!file_exists(path));
    assert(!file_trunc(NULL));
    assert(file_trunc(path));
    assert(file_exists(path));
    assert(file_remove(path) == 0);
}

static void
test_file_solve(void) {
    char path[FILE_NPATH];
    assert(file_solve(NULL, sizeof path, "/tmp/../tmp") == NULL);
    assert(file_solve(path, 0, "/tmp/../tmp") == NULL);
    assert(file_solve(path, sizeof path, NULL) == NULL);
    assert(file_solve(path, sizeof path, get_test_dirpath()) != NULL);
}

static void
test_file_solvecp(void) {
    assert(!file_solvecp(NULL));
    char *path = file_solvecp(get_test_dirpath());
    assert(path != NULL);
    assert(strcmp(path, get_test_dirpath()) == 0);
    free(path);
}

static void
test_file_solvefmt(void) {
    char path[1024];
    assert(file_solvefmt(NULL, sizeof path, "/%s/../%s", "tmp", "tmp") == NULL);
    assert(file_solvefmt(path, 0, "/%s/../%s", "tmp", "tmp") == NULL);
    assert(file_solvefmt(path, sizeof path, NULL, "tmp", "tmp") == NULL);
    assert(file_solvefmt(path, sizeof path, "%s", get_test_dirpath()) != NULL);
}

static void
test_file_isdir(void) {
    assert(!file_isdir(NULL));
    assert(file_isdir(get_test_dirpath()));
    assert(!file_isdir("/not/found/directory"));
}

static void
test_file_readcp(void) {
    FILE *fin = file_open(get_test_finpath(), "rb");
    assert(fin != NULL);
    assert(!file_readcp(NULL));
    char *p = file_readcp(fin);
    file_close(fin);
    assert(p != NULL);
    free(p);
}

static void
test_file_size(void) {
    FILE *fin = file_open(get_test_finpath(), "rb");
    assert(fin != NULL);
    assert(file_size(NULL) == -1);
    assert(file_size(fin) == get_test_finsize());
    assert(file_close(fin) == 0);
}

static void
test_file_suffix(void) {
    assert(file_suffix(NULL) == NULL);
    const char *suf = file_suffix("/this/is/text/file.txt");
    assert(suf != NULL);
    assert(strcmp(suf, "txt") == 0);
}

static void
test_file_dirname(void) {
    char name[FILE_NPATH];
    char userhome[FILE_NPATH];
    char path[FILE_NPATH];
    assert(file_get_user_home(userhome, sizeof userhome));
    assert(file_solvefmt(path, sizeof path, "%s/file", userhome));

    assert(file_dirname(NULL, sizeof name, path) == NULL);
    assert(file_dirname(name, 0, path) == NULL);
    assert(file_dirname(name, sizeof name, NULL) == NULL);
    assert(file_dirname(name, sizeof name, path) != NULL);
    assert(strcmp(name, userhome) == 0);
}

static void
test_file_basename(void) {
    char name[FILE_NPATH];
    char userhome[FILE_NPATH];
    char path[FILE_NPATH];
    assert(file_get_user_home(userhome, sizeof userhome));
    assert(file_solvefmt(path, sizeof path, "%s/file.txt", userhome));

    assert(file_basename(NULL, sizeof name, path) == NULL);
    assert(file_basename(name, 0, path) == NULL);
    assert(file_basename(name, sizeof name, NULL) == NULL);
    assert(file_basename(name, sizeof name, path) != NULL);
    assert(strcmp(name, "file.txt") == 0);
}

static void
test_file_getline(void) {
    FILE *fin = get_test_fin();
    assert(fin != NULL);
    char line[1024];
    assert(file_getline(NULL, sizeof line, fin) == EOF);
    assert(file_getline(line, 0, fin) == EOF);
    assert(file_getline(line, sizeof line, NULL) == EOF);
    assert(file_getline(line, sizeof line, fin) != EOF);
    assert(strcmp(get_test_fcontent_nonewline(), line) == 0);
    assert(file_close(fin) == 0);
}

static void
test_file_readline(void) {
    char line[1024];
    assert(file_readline(NULL, sizeof line, get_test_finpath()) == NULL);
    assert(file_readline(line, 0, get_test_finpath()) == NULL);
    assert(file_readline(line, sizeof line, NULL) == NULL);
    assert(file_readline(line, sizeof line, get_test_finpath()) != NULL);
    assert(strcmp(line, get_test_fcontent_nonewline()) == 0);
}

static void
test_file_writeline(void) {
    assert(file_writeline(NULL, get_test_finpath()) == NULL);
    assert(file_writeline(get_test_fcontent_nonewline(), NULL) == NULL);
    assert(file_writeline(get_test_fcontent_nonewline(), get_test_finpath()));
    test_file_readline();
}

static void
test_file_dirnodedel(void) {
    file_dirclose(NULL);
    assert(file_diropen(NULL) == NULL);
    assert(file_dirread(NULL) == NULL);
    file_dirnodedel(NULL);

    struct file_dir *dir = file_diropen(get_test_dirpath());
    assert(dir != NULL);

    for (struct file_dirnode *node; (node = file_dirread(dir)); ) {
        const char *dname = file_dirnodename(node);
        assert(dname != NULL);
        file_dirnodedel(node);
    }

    assert(file_dirclose(dir) == 0);
}

static void
test_file_dirnodename(void) {
    // test_file_dirclose
}

static void
test_file_dirclose(void) {
    // test_file_dirclose
}

static void
test_file_diropen(void) {
    // test_file_dirclose
}

static void
test_file_dirread(void) {
    // test_file_dirclose
}

static void
test_file_conv_line_encoding(void) {
    char *encoded;

    encoded = file_conv_line_encoding(NULL, "abc");
    assert(!encoded);

    encoded = file_conv_line_encoding("nothing", "abc");
    assert(!encoded);

    encoded = file_conv_line_encoding("crlf", NULL);
    assert(!encoded);

    encoded = file_conv_line_encoding("crlf", "abc");
    assert(encoded);
    assert(!strcmp(encoded, "abc"));
    free(encoded);

    // to crlf
    encoded = file_conv_line_encoding("crlf", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    encoded = file_conv_line_encoding("crlf", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    encoded = file_conv_line_encoding("crlf", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\r\ndef\r\n"));
    free(encoded);

    // to cr
    encoded = file_conv_line_encoding("cr", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    encoded = file_conv_line_encoding("cr", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    encoded = file_conv_line_encoding("cr", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\rdef\r"));
    free(encoded);

    // to lf
    encoded = file_conv_line_encoding("lf", "abc\r\ndef\r\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);

    encoded = file_conv_line_encoding("lf", "abc\rdef\r");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);

    encoded = file_conv_line_encoding("lf", "abc\ndef\n");
    assert(encoded);
    assert(!strcmp(encoded, "abc\ndef\n"));
    free(encoded);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
file_tests[] = {
    {"file_close", test_file_close},
    {"file_open", test_file_open},
    {"file_copy", test_file_copy},
    {"file_closedir", test_file_closedir},
    {"file_opendir", test_file_opendir},
    {"file_realpath", test_file_realpath},
    {"file_exists", test_file_exists},
    {"file_mkdirmode", test_file_mkdirmode},
    {"file_mkdirq", test_file_mkdirq},
    {"file_trunc", test_file_trunc},
    {"file_solve", test_file_solve},
    {"file_solvecp", test_file_solvecp},
    {"file_solvefmt", test_file_solvefmt},
    {"file_isdir", test_file_isdir},
    {"file_readcp", test_file_readcp},
    {"file_size", test_file_size},
    {"file_suffix", test_file_suffix},
    {"file_dirname", test_file_dirname},
    {"file_basename", test_file_basename},
    {"file_getline", test_file_getline},
    {"file_readline", test_file_readline},
    {"file_writeline", test_file_writeline},
    {"file_dirnodedel", test_file_dirnodedel},
    {"file_dirnodename", test_file_dirnodename},
    {"file_dirclose", test_file_dirclose},
    {"file_diropen", test_file_diropen},
    {"file_dirread", test_file_dirread},
    {"file_conv_line_encoding", test_file_conv_line_encoding},
    {0},
};

/*****
* cl *
*****/


static void
test_cl_cldel(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    cl_del(cl);
}

static void
test_cl_clescdel(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    size_t parrlen = cl_len(cl);
    char **parr = cl_escdel(cl);
    assert(parr != NULL);
    freeargv(parrlen, parr);
}

static void
test_cl_clnew(void) {
    // test_cl_cldel
}

static void
test_cl_clresize(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    assert(cl_capa(cl) == 4);
    assert(cl_resize(cl, 8));
    assert(cl_capa(cl) == 8);
    cl_del(cl);
}

static void
test_cl_clpush(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    assert(cl_len(cl) == 0);
    assert(cl_push(cl, "123"));
    assert(cl_push(cl, "223"));
    assert(cl_push(cl, "323"));
    assert(strcmp(cl_getc(cl, 1), "223") == 0);
    assert(cl_len(cl) == 3);
    cl_del(cl);
}

static void
test_cl_clgetc(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    assert(cl_push(cl, "123"));
    assert(strcmp(cl_getc(cl, 0), "123") == 0);
    cl_del(cl);
}

static void
test_cl_clclear(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);
    assert(cl_push(cl, "123"));
    assert(cl_push(cl, "223"));
    assert(cl_len(cl) == 2);
    cl_clear(cl);
    assert(cl_len(cl) == 0);
    cl_del(cl);
}

static void
test_cl_clparsestropts(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    assert(cl_parse_str_opts(cl, "cmd -h -ab 123 --help 223", CL_WRAP));
    assert(strcmp(cl_getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cl_getc(cl, 1), "'-h'") == 0);
    assert(strcmp(cl_getc(cl, 2), "'-ab'") == 0);
    assert(strcmp(cl_getc(cl, 3), "'123'") == 0);
    assert(strcmp(cl_getc(cl, 4), "'--help'") == 0);
    assert(strcmp(cl_getc(cl, 5), "'223'") == 0);

    assert(cl_parse_str_opts(cl, "cmd -a 123", CL_WRAP));
    assert(strcmp(cl_getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cl_getc(cl, 1), "'-a'") == 0);
    assert(strcmp(cl_getc(cl, 2), "'123'") == 0);

    assert(cl_parse_str_opts(cl, "\"cmd\" \"-a\" \"123\"", CL_WRAP));
    assert(strcmp(cl_getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cl_getc(cl, 1), "'-a'") == 0);
    assert(strcmp(cl_getc(cl, 2), "'123'") == 0);

    assert(cl_parse_str_opts(cl, "\"cmd\" \"-a\" \"123\"", CL_WRAP));
    assert(strcmp(cl_getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cl_getc(cl, 1), "'-a'") == 0);
    assert(strcmp(cl_getc(cl, 2), "'123'") == 0);

    assert(cl_parse_str_opts(cl, "cmd -a 123", CL_ESCAPE));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "123") == 0);

    assert(cl_parse_str_opts(cl, "cmd -a \"1'23\"", CL_ESCAPE));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "1\\'23") == 0);

    cl_del(cl);
}

static void
test_cl_clparsestr(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    assert(cl_parse_str(cl, "cmd -h -ab 123 --help 223"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-h") == 0);
    assert(strcmp(cl_getc(cl, 2), "-ab") == 0);
    assert(strcmp(cl_getc(cl, 3), "123") == 0);
    assert(strcmp(cl_getc(cl, 4), "--help") == 0);
    assert(strcmp(cl_getc(cl, 5), "223") == 0);

    assert(cl_parse_str(cl, "cmd -a \"abc\""));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd -a 'a\"bc'"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "a\"bc") == 0);

    assert(cl_parse_str(cl, "cmd -a=abc"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd -a=\"abc\""));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd -a='abc'"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd \"-a\"=\"abc\""));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd '-a'='abc'"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "-a") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd --abc=abc"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "--abc") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd --abc=\"abc\""));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "--abc") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd --abc='abc'"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "--abc") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd \"--abc\"=\"abc\""));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "--abc") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    assert(cl_parse_str(cl, "cmd '--abc'='abc'"));
    assert(strcmp(cl_getc(cl, 0), "cmd") == 0);
    assert(strcmp(cl_getc(cl, 1), "--abc") == 0);
    assert(strcmp(cl_getc(cl, 2), "abc") == 0);

    cl_del(cl);
}

static void
test_cl_clparseargvopts(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    cl_del(cl);
}

static void
test_cl_clparseargv(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    cl_del(cl);
}

static void
test_cl_clshow(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    cl_del(cl);
}

static void
test_cl_cllen(void) {
    cl_t *cl = cl_new();
    assert(cl != NULL);

    cl_del(cl);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
cl_tests[] = {
    {"cl_del", test_cl_cldel},
    {"cl_escdel", test_cl_clescdel},
    {"cl_new", test_cl_clnew},
    {"cl_resize", test_cl_clresize},
    {"cl_getc", test_cl_clgetc},
    {"cl_push", test_cl_clpush},
    {"cl_clear", test_cl_clclear},
    {"cl_parse_str_opts", test_cl_clparsestropts},
    {"cl_parse_str", test_cl_clparsestr},
    {"cl_parseargvopts", test_cl_clparseargvopts},
    {"cl_parseargv", test_cl_clparseargv},
    {"cl_show", test_cl_clshow},
    {"cl_len", test_cl_cllen},
    {0},
};

/********
* error *
********/

static void
test_error_fix_text(void) {
    char buf[BUFSIZ] = {0};

    err_fix_text(buf, sizeof buf, "text", false);
    assert(strcmp(buf, "Text.") == 0);
    buf[0] = '\0';

    err_fix_text(buf, sizeof buf, "text.text", false);
    assert(strcmp(buf, "Text. Text.") == 0);
    buf[0] = '\0';

    err_fix_text(buf, sizeof buf, "text. text", true);
    assert(strcmp(buf, "Text. Text.") == 0);
    buf[0] = '\0';

    err_fix_text(buf, sizeof buf, "text.     text", false);
    assert(strcmp(buf, "Text. Text.") == 0);
    buf[0] = '\0';

    err_fix_text(buf, sizeof buf, "Failed to open directory \"/path/to/dir\".failed to remove recursive.", false);
    assert(strcmp(buf, "Failed to open directory \"/path/to/dir\". Failed to remove recursive.") == 0);
    buf[0] = '\0';
}

static void
test_error__log(void) {
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    _log_unsafe("file", 100, "func", "warn", "msg");
    // assert(strcmp(buf, "")); // TODO

    setbuf(stderr, NULL);
}

static void
test_error_die(void) {
    // nothing todo
}

static void
test_error_error(void) {
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    err_error("this is error");
    // assert(strcmp(buf, "Error: This is error. No such file or directory.\n") == 0);

    setbuf(stderr, NULL);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
error_tests[] = {
    {"fix_text", test_error_fix_text},
    {"_log", test_error__log},
    {"die", test_error_die},
    {"error", test_error_error},
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
        argv[i] = cstr_edup("abc");
    }

    return argv;
}

static void
test_util_freeargv(void) {
    int argc = 2;
    char **argv = __create_testargv(argc);
    assert(argv != NULL);
    freeargv(argc, argv);
}

static void
test_util_showargv(void) {
    // TODO
    // this test was failed
    return;

    char buf[1024] = {0};

    int argc = 2;
    char **argv = __create_testargv(argc);
    assert(argv != NULL);

    setbuf(stdout, buf);
    showargv(argc, argv);
    setbuf(stdout, NULL);

    assert(!strcmp(buf, "abc\nabc\n"));

    freeargv(argc, argv);
}

static void
test_util_isoutofhome(void) {
    char userhome[FILE_NPATH];
    assert(file_get_user_home(userhome, sizeof userhome) != NULL);

    char varhome[FILE_NPATH];
    assert(file_solvefmt(varhome, sizeof varhome, "%s/.cap/var/home", userhome) != NULL);

    char caphome[FILE_NPATH];
    assert(file_readline(caphome, sizeof caphome, varhome) != NULL);

    assert(isoutofhome(varhome, "/not/found/dir"));
    assert(!isoutofhome(varhome, caphome));
}

static void
test_util_randrange(void) {
    int min = 0;
    int max = 10;
    int n = randrange(min, max);
    for (int i = min; i < max; ++i) {
        if (n == i) {
            return;
        }
    }

    assert(0 && "invalid value range");
}

static void
test_util_safesystem(void) {
    char cmd[1024];
#ifdef _TESTS_WINDOWS
    assert(file_solvefmt(cmd, sizeof cmd, "dir") != NULL);
#else
    const char *path = "/tmp/f";
    if (file_exists(path)) {
        assert(remove(path) == 0);
    }
    assert(file_solvefmt(cmd, sizeof cmd, "/bin/sh -c \"touch %s\"", path) != NULL);
    assert(safesystem(cmd, SAFESYSTEM_DEFAULT) == 0);
    assert(file_exists(path));
#endif
}

static void
test_util_argsbyoptind(void) {
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

    cstring_array_t *args = argsbyoptind(argc, argv, optind);
    // cstrarr_show(args, stdout);
    assert(strcmp(cstrarr_getc(args, 0), "program") == 0);
    assert(strcmp(cstrarr_getc(args, 1), "arg1") == 0);
    assert(strcmp(cstrarr_getc(args, 2), "arg2") == 0);
    cstrarr_del(args);
}

static void
test_util_solve_cmdline_arg_path(void) {
    config_t *config = config_new();
    config->scope = CAP_SCOPE_LOCAL;

    char fname[FILE_NPATH];

#ifdef _CAP_WINDOWS
    snprintf(config->home_path, sizeof config->home_path, "C:\\path\\to\\home");
    snprintf(config->cd_path, sizeof config->cd_path, "C:\\path\\to\\cd");

    assert(solve_cmdline_arg_path(config, fname, sizeof fname, "path/to/file"));
    assert(!strcmp(fname, "C:\\path\\to\\cd\\path\\to\\file"));

    assert(solve_cmdline_arg_path(config, fname, sizeof fname, "/path/to/file"));
    assert(!strcmp(fname, "C:\\path\\to\\home\\path\\to\\file"));
#else
    snprintf(config->home_path, sizeof config->home_path, "/path/to/home");
    snprintf(config->cd_path, sizeof config->cd_path, "/path/to/cd");

    assert(solve_cmdline_arg_path(config, fname, sizeof fname, "path/to/file"));
    assert(!strcmp(fname, "/path/to/cd/path/to/file"));

    assert(solve_cmdline_arg_path(config, fname, sizeof fname, "/path/to/file"));
    assert(!strcmp(fname, "/path/to/home/path/to/file"));
#endif
    config_del(config);
}

static void
test_util_escape(void) {
    char dst[1024];
    assert(escape(dst, sizeof dst, "abca", "a"));
    assert(!strcmp(dst, "\\abc\\a"));
}

static void
test_util_compile_argv(void) {
    config_t *config = config_new();
    int argc = 4;
    char *argv[] = {
        "make",
        "file",
        "-a",
        "bbb",
        NULL,
    };
    const char *src = "{: opts.get(\"a\") :}";

    char *compiled = compile_argv(config, argc-1, argv+1, src);

    printf("compiled[%s]\n", compiled);
    assert(!strcmp(compiled, "bbb"));

    free(compiled);
    config_del(config);
}

static void
test_util_pop_tail_slash(void) {
    char s[100];
#ifdef _TESTS_WINDOWS
    strcpy(s, "C:\\path\\to\\dir\\");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\path\\to\\dir");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "C:\\"));
#else
    strcpy(s, "/path/to/dir/");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/path/to/dir");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/");
    assert(pop_tail_slash(s));
    assert(!strcmp(s, "/"));
#endif
}

static const struct testcase
utiltests[] = {
    {"freeargv", test_util_freeargv},
    {"showargv", test_util_showargv},
    {"isoutofhome", test_util_isoutofhome},
    {"randrange", test_util_randrange},
    {"safesystem", test_util_safesystem},
    {"argsbyoptind", test_util_argsbyoptind},
    {"solve_cmdline_arg_path", test_util_solve_cmdline_arg_path},
    {"escape", test_util_escape},
    {"compile_argv", test_util_compile_argv},
    {"pop_tail_slash", test_util_pop_tail_slash},
    {0},
};

/*******
* path *
*******/

static void
test_path_pop_back_of(void) {
    char s[100];

    assert(path_pop_back_of(NULL, '?') == NULL);

    strcpy(s, "abc");
    assert(path_pop_back_of(s, 'c'));
    assert(!strcmp(s, "ab"));

    assert(path_pop_back_of(s, '?'));
    assert(!strcmp(s, "ab"));
}

void
test_path_pop_tail_slash(void) {
    char s[100];

    assert(path_pop_tail_slash(NULL) == NULL);

#ifdef _TESTS_WINDOWS
    strcpy(s, "C:\\path\\to\\dir\\");
    assert(path_pop_tail_slash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));

    strcpy(s, "C:\\path\\to\\dir");
    assert(path_pop_tail_slash(s));
    assert(!strcmp(s, "C:\\path\\to\\dir"));
#else
    strcpy(s, "/path/to/dir/");
    assert(path_pop_tail_slash(s));
    assert(!strcmp(s, "/path/to/dir"));

    strcpy(s, "/path/to/dir");
    assert(path_pop_tail_slash(s));
    assert(!strcmp(s, "/path/to/dir"));
#endif
}

static const struct testcase
pathtests[] = {
    {"path_pop_back_of", test_path_pop_back_of},
    {"path_pop_tail_slash", test_path_pop_tail_slash},
    {0},
};

/************
* lang/opts *
************/

static void
test_lang_opts_new(void) {
    opts_t *opts = opts_new();
    assert(opts);
    opts_del(opts);
}

static void
test_lang_opts_parse(void) {
    opts_t *opts = opts_new();
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
    assert(opts_parse(opts, argc, argv));

    assert(opts_args_len(opts) == 3);
    assert(opts_getc_args(opts, -1) == NULL);
    assert(opts_getc_args(opts, 0));
    assert(opts_getc_args(opts, 1));
    assert(opts_getc_args(opts, 2));
    assert(opts_getc_args(opts, 3) == NULL);
    assert(!strcmp(opts_getc_args(opts, 0), "make"));
    assert(!strcmp(opts_getc_args(opts, 1), "arg1"));
    assert(!strcmp(opts_getc_args(opts, 2), "arg2"));
    assert(opts_getc(opts, "a"));
    assert(!strcmp(opts_getc(opts, "a"), "aaa"));
    assert(opts_getc(opts, "bbb"));
    assert(!strcmp(opts_getc(opts, "bbb"), "bbb"));
    assert(opts_has(opts, "a"));
    assert(opts_has(opts, "bbb"));
    opts_del(opts);
}

static void
test_lang_opts_parse_0(void) {
    opts_t *opts = opts_new();
    assert(opts);

    int argc = 1;
    char *argv[] = {
        "make",
        NULL,
    };
    assert(opts_parse(opts, argc, argv));
    opts_del(opts);
}

static void
test_lang_opts_getc_args_0(void) {
    opts_t *opts = opts_new();
    assert(opts);

    int argc = 3;
    char *argv[] = {
        "cmd",
        "arg1",
        "arg2",
        NULL,
    };

    assert(opts_parse(opts, argc, argv));
    assert(!strcmp(opts_getc_args(opts, 0), "cmd"));
    assert(!strcmp(opts_getc_args(opts, 1), "arg1"));
    assert(!strcmp(opts_getc_args(opts, 2), "arg2"));
    opts_del(opts);
}

static void
test_lang_opts_getc_args_1(void) {
    opts_t *opts = opts_new();
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

    assert(opts_parse(opts, argc, argv));
    assert(!strcmp(opts_getc(opts, "a"), "optarg1"));
    assert(!strcmp(opts_getc(opts, "b"), "optarg2"));
    assert(opts_getc_args(opts, 0));
    assert(!strcmp(opts_getc_args(opts, 0), "cmd"));
    assert(opts_getc_args(opts, 1));
    assert(!strcmp(opts_getc_args(opts, 1), "arg1"));
    assert(opts_getc_args(opts, 2));
    assert(!strcmp(opts_getc_args(opts, 2), "arg2"));
    opts_del(opts);
}

static const struct testcase
lang_opts_tests[] = {
    {"opts_new", test_lang_opts_new},
    {"opts_parse", test_lang_opts_parse},
    {"opts_parse_0", test_lang_opts_parse_0},
    {"opts_getc_args_0", test_lang_opts_getc_args_0},
    {"opts_getc_args_1", test_lang_opts_getc_args_1},
    {0},
};

/************
* tokenizer *
************/

static void
test_tkr_new(void) {
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(opt);
    tkr_del(tkr);
}

static void
test_tkr_parse(void) {
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(opt);
    const token_t *token;

    tkr_parse(tkr, "abc");
    {
        assert(tkr_tokens_len(tkr) == 1);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_TEXT_BLOCK);
        assert(strcmp(token->text, "abc") == 0);
    }

    tkr_parse(tkr, "abc{@@}bbc");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_TEXT_BLOCK);
        assert(strcmp(token->text, "abc") == 0);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_TEXT_BLOCK);
        assert(strcmp(token->text, "bbc") == 0);
    }

    // test of realloc of tokens
    tkr_parse(tkr, "{@......@}");
    {
        assert(tkr_tokens_len(tkr) == 8);
    }

    tkr_parse(tkr, "");
    {
        assert(tkr_has_error_stack(tkr) == false);
        assert(tkr_tokens_len(tkr) == 0);
    }

    tkr_parse(tkr, "{@");
    {
        assert(tkr_tokens_len(tkr) == 1);
        assert(tkr_has_error_stack(tkr) == true);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
    }

    tkr_parse(tkr, "{@@");
    {
        assert(tkr_has_error_stack(tkr) == true);
        assert(strcmp(tkr_getc_first_error_message(tkr), "invalid syntax. single '@' is not supported") == 0);
    }

    tkr_parse(tkr, "{@@}");
    {
        assert(tkr_tokens_len(tkr) == 2);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@.@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_DOT_OPE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@..@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_DOT_OPE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_DOT_OPE);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@,@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_COMMA);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@,,@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_COMMA);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_COMMA);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@:@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_COLON);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@;@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_SEMICOLON);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@(@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_LPAREN);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@)@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RPAREN);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@[@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_LBRACKET);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@]@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RBRACKET);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@{@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_LBRACE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@}@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RBRACE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@()@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_LPAREN);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RPAREN);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@a@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "a") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@abc@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "abc") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@abc123@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "abc123") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@abc_123@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "abc_123") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@123@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_INTEGER);
        assert(token->lvalue == 123);
        assert(strcmp(token->text, "123") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@-123@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_SUB);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_INTEGER);
        assert(token->lvalue == 123);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /**********
    * as from *
    **********/

    tkr_parse(tkr, "{@as@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_AS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@from@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_FROM);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /*************
    * statements *
    *************/

    tkr_parse(tkr, "{@ end @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_END);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ if @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_IF);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ elif @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_ELIF);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ else @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_ELSE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ for @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_FOR);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /************
    * operators *
    ************/

    tkr_parse(tkr, "{@ + @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ADD);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ - @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_SUB);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ * @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MUL);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ / @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_DIV);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ % @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MOD);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ = @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ += @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ADD_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ -= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_SUB_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ *= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MUL_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ /= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_DIV_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ %= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MOD_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /***********************
    * comparison operators *
    ***********************/

    tkr_parse(tkr, "{@ == @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_EQ);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ != @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_NOT_EQ);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ <= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_LTE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ >= @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_GTE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ < @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_LT);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ > @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_GT);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ or @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_OR);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ and @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_AND);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@ not @}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_NOT);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /*******
    * expr *
    *******/

    tkr_parse(tkr, "{@ 1 * 2 @}");
    {
        assert(tkr_tokens_len(tkr) == 5);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_INTEGER);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_OP_MUL);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_INTEGER);
        token = tkr_tokens_getc(tkr, 4);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /*********
    * others *
    *********/

    tkr_parse(tkr, "{@\"\"@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\"abc\"@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "abc") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\"abc\"\"bbc\"@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "abc") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "bbc") == 0);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr,
        "{@ import alias\n"
        "alias.set(\"dtl\", \"run bin/date-line\") @}");
    {
        assert(tkr_tokens_len(tkr) == 13);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_STMT_IMPORT);
        assert(strcmp(token->text, "import") == 0);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "alias") == 0);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 4);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "alias") == 0);
        token = tkr_tokens_getc(tkr, 5);
        assert(token->type == TOKEN_TYPE_DOT_OPE);
        token = tkr_tokens_getc(tkr, 6);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        assert(strcmp(token->text, "set") == 0);
        token = tkr_tokens_getc(tkr, 7);
        assert(token->type == TOKEN_TYPE_LPAREN);
        token = tkr_tokens_getc(tkr, 8);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "dtl") == 0);
        token = tkr_tokens_getc(tkr, 9);
        assert(token->type == TOKEN_TYPE_COMMA);
        token = tkr_tokens_getc(tkr, 10);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        assert(strcmp(token->text, "run bin/date-line") == 0);
        token = tkr_tokens_getc(tkr, 11);
        assert(token->type == TOKEN_TYPE_RPAREN);
        token = tkr_tokens_getc(tkr, 12);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /******************
    * reference block *
    ******************/

    tkr_parse(tkr, "{:");
    {
        assert(tkr_tokens_len(tkr) == 1);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        assert(tkr_has_error_stack(tkr) == true);
        assert(strcmp(tkr_getc_first_error_message(tkr), "not closed by block") == 0);
    }

    tkr_parse(tkr, "{::}");
    {
        assert(tkr_tokens_len(tkr) == 2);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{:\n:}");
    {
        assert(tkr_tokens_len(tkr) == 1);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        assert(tkr_has_error_stack(tkr) == true);
        assert(strcmp(tkr_getc_first_error_message(tkr), "syntax error. unsupported character \"\n\"") == 0);
    }

    tkr_parse(tkr, "{:abc:}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{:abc123:}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{:abc_123:}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: 123 :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_INTEGER);
        assert(token->lvalue == 123);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: alias.run(\"dtl\") :}");
    {
        assert(tkr_tokens_len(tkr) == 8);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_DOT_OPE);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_IDENTIFIER);
        token = tkr_tokens_getc(tkr, 4);
        assert(token->type == TOKEN_TYPE_LPAREN);
        token = tkr_tokens_getc(tkr, 5);
        assert(token->type == TOKEN_TYPE_DQ_STRING);
        token = tkr_tokens_getc(tkr, 6);
        assert(token->type == TOKEN_TYPE_RPAREN);
        token = tkr_tokens_getc(tkr, 7);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    /*****************************
    * reference block: operators *
    *****************************/

    tkr_parse(tkr, "{: + :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ADD);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: - :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_SUB);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: * :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MUL);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: / :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_DIV);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: = :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: += :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_ADD_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: -= :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_SUB_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: *= :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_MUL_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: /= :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_DIV_ASS);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    /****************************************
    * reference block: comparison operators *
    ****************************************/

    tkr_parse(tkr, "{: == :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_EQ);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    tkr_parse(tkr, "{: != :}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LDOUBLE_BRACE);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_OP_NOT_EQ);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RDOUBLE_BRACE);
    }

    /**********
    * comment *
    **********/

    tkr_parse(tkr, "{@\n"
    "// comment\n"
    "@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\n"
    "// comment\n"
    "// comment\n"
    "@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\n"
    "// comment\n");
    {
        assert(tkr_has_error_stack(tkr));
        assert(!strcmp(tkr_getc_first_error_message(tkr), "not closed by block"));
    }

    tkr_parse(tkr, "{@ // comment");
    {
        assert(tkr_has_error_stack(tkr));
        assert(!strcmp(tkr_getc_first_error_message(tkr), "not closed by block"));
    }

    tkr_parse(tkr, "{@\n"
    "/* comment */"
    "@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\n"
    "/* comment \n"
    "   comment \n"
    "   comment */"
    "@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    /***********
    * newlines *
    ***********/

    tkr_parse(tkr, "{@\n@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\n\n@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\r@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\r\r@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\r\n@}");
    {
        assert(tkr_tokens_len(tkr) == 3);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_parse(tkr, "{@\r\n\r\n@}");
    {
        assert(tkr_tokens_len(tkr) == 4);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 2);
        assert(token->type == TOKEN_TYPE_NEWLINE);
        token = tkr_tokens_getc(tkr, 3);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

    tkr_del(tkr);
}

/**
 * 0 memory leaks
 * 2020/02/25
 */
static const struct testcase
tokenizer_tests[] = {
    {"tkr_new", test_tkr_new},
    {"tkr_parse", test_tkr_parse},
    {0},
};

/***********
* compiler *
***********/

static void
test_ast_show_error(const ast_t *ast) {
    if (ast_has_errors(ast)) {
        printf("error detail[%s]\n", ast_getc_first_error_message(ast));
    }
}

static void
test_cc_basic_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;

    tkr_parse(tkr, "");
    ast_clear(ast);
    ast_clear(ast);
    cc_compile(ast, tkr_get_tokens(tkr));
    root = ast_getc_root(ast);
    assert(root == NULL);

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_basic_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_assign_list_t *assign_list;
    node_assign_t *assign;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_asscalc_t *asscalc;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_factor_t *factor;
    node_atom_t *atom;
    node_identifier_t *identifier;

    tkr_parse(tkr, "{@ i = 0 @}"); {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
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
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "i"));
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_code_block(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;

    tkr_parse(tkr, "{@@}");
    ast_clear(ast);
    cc_compile(ast, tkr_get_tokens(tkr));
    root = ast_getc_root(ast);
    assert(root);
    program = root->real;
    blocks = program->blocks->real;
    code_block = blocks->code_block->real;
    assert(code_block);

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_code_block_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{@@}");
    ast_clear(ast);
    cc_compile(ast, tkr_get_tokens(tkr));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_ref_block(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_ref_block_t *ref_block;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_array_t_ *array;
    node_array_elems_t *array_elems;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_nil_t *nil;
    node_identifier_t *identifier;

    tkr_parse(tkr, "{: nil :}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        nil = atom->nil->real;
        assert(nil != NULL);
    }

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{: var :}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{: [1, 2] :}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        ref_block = blocks->ref_block->real;
        formula = ref_block->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        assert(nodearr_len(array_elems->nodearr) == 2);
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_ref_block_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{: nil :}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_ref_block_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{: 1 :}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_ref_block_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{: var :}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_ref_block_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{: [1, 2] :}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_formula(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_identifier_t *identifier;
    node_formula_t *formula;
    node_assign_t *assign;
    node_assign_list_t *assign_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_digit_t *digit;
    node_string_t *string;

    tkr_parse(tkr, "{@ a = 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ a = b = 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        assert(nodearr_len(assign->nodearr) == 3);
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        assert(or_test);
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        assert(and_test);
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        assert(not_test);
        comparison = not_test->comparison->real;
        assert(comparison);
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "b"));
        test = nodearr_get(assign->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ a = 1, b = 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 1);
        assign = nodearr_get(assign_list->nodearr, 1)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "b"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_dict(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_factor_t *factor;
    node_atom_t *atom;
    node_dict_t *dict;
    node_dict_elems_t *dict_elems;
    node_dict_elem_t *dict_elem;
    node_simple_assign_t *simple_assign;
    node_asscalc_t *asscalc;

    tkr_parse(tkr, "{@ { \"key\" : \"value\" } @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        assert(nodearr_len(dict_elems->nodearr) == 1);
        dict_elem = nodearr_get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    tkr_parse(tkr, "{@ { \"key\" : \"value\", } @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        assert(nodearr_len(dict_elems->nodearr) == 1);
        dict_elem = nodearr_get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    tkr_parse(tkr, "{@ { \"key1\" : \"value1\", \"key2\" : \"value2\" } @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        program = root->real;
        assert(program != NULL);
        assert(program->blocks != NULL);
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        assert(nodearr_len(dict_elems->nodearr) == 2);
        dict_elem = nodearr_get(dict_elems->nodearr, 0)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
        dict_elem = nodearr_get(dict_elems->nodearr, 1)->real;
        simple_assign = dict_elem->key_simple_assign->real;
        assert(simple_assign);
        simple_assign = dict_elem->value_simple_assign->real;
        assert(simple_assign);
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_dict_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{@ {} @}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_dict_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{@ { \"key\" : \"value\", } @}");
    ast_clear(ast);
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_dict_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{@ { \"key1\" : \"value1\", \"key2\" : \"value2\" } @}");
    ast_clear(ast);
    (cc_compile(ast, tkr_get_tokens(tkr)));

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_expr(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_digit_t *digit;
    node_comp_op_t *comp_op;
    node_add_sub_op_t *add_sub_op;
    node_mul_div_op_t *mul_div_op;

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 == 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_EQ);
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        asscalc = nodearr_get(comparison->nodearr, 2)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 == 2 == 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_EQ);
        asscalc = nodearr_get(comparison->nodearr, 2)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = nodearr_get(comparison->nodearr, 3)->real;
        assert(comp_op->op == OP_EQ);
        asscalc = nodearr_get(comparison->nodearr, 4)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 != 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_NOT_EQ);
        asscalc = nodearr_get(comparison->nodearr, 2)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 != 2 != 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_NOT_EQ);
        asscalc = nodearr_get(comparison->nodearr, 2)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = nodearr_get(comparison->nodearr, 3)->real;
        assert(comp_op->op == OP_NOT_EQ);
        asscalc = nodearr_get(comparison->nodearr, 4)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 + 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        add_sub_op = nodearr_get(expr->nodearr, 1)->real;
        assert(add_sub_op->op == OP_ADD);
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 + 2 + 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        term = nodearr_get(expr->nodearr, 4)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 - 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        add_sub_op = nodearr_get(expr->nodearr, 1)->real;
        assert(add_sub_op->op == OP_SUB);
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 - 2 - 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        term = nodearr_get(expr->nodearr, 4)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 * 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        mul_div_op = nodearr_get(term->nodearr, 1)->real;
        assert(mul_div_op->op == OP_MUL);
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = nodearr_get(term->nodearr, 2)->real;
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

    tkr_parse(tkr, "{@ 1 * 2 * 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = nodearr_get(term->nodearr, 2)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        negative = nodearr_get(term->nodearr, 4)->real;
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

    tkr_parse(tkr, "{@ 1 / 2 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        mul_div_op = nodearr_get(term->nodearr, 1)->real;
        assert(mul_div_op->op == OP_DIV);
    }

    tkr_parse(tkr, "{@ 1 / 2 / 3 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        negative = nodearr_get(term->nodearr, 2)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        negative = nodearr_get(term->nodearr, 4)->real;
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

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_index(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_identifier_t *identifier;

    tkr_parse(tkr, "{@ a[0] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    tkr_parse(tkr, "{@ a[0][0] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_dot(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;

    tkr_parse(tkr, "{@ a.b @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ a.b() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ a.b[0] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_call(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_identifier_t *identifier;
    node_chain_t *chain;

    tkr_parse(tkr, "{@ f() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "f"));
    }

    tkr_parse(tkr, "{@ f(1) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ f(1, \"abc\") @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ a.b() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;

        negative = nodearr_get(term->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ f()() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "f"));
   }

    tkr_parse(tkr, "{@ a[0]() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        chain = negative->chain->real;
        factor = chain->factor->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(!strcmp(identifier->identifier, "a"));
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_array(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_array_elems_t *array_elems;
    node_simple_assign_t *simple_assign;
    node_array_t_ *array;

    tkr_parse(tkr, "{@ [] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        assert(nodearr_len(array_elems->nodearr) == 0);
    }

    tkr_parse(tkr, "{@ [1, 2] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(nodearr_len(array_elems->nodearr) == 2);
        simple_assign = nodearr_get(array_elems->nodearr, 0)->real;
        assert(nodearr_len(simple_assign->nodearr) == 1);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ [1] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(nodearr_len(array_elems->nodearr) == 1);
        simple_assign = nodearr_get(array_elems->nodearr, 0)->real;
        assert(nodearr_len(simple_assign->nodearr) == 1);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
    }

    tkr_parse(tkr, "{@ [a = 1] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(nodearr_len(array_elems->nodearr) == 1);
        simple_assign = nodearr_get(array_elems->nodearr, 0)->real;
        assert(nodearr_len(simple_assign->nodearr) == 2);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
        assert(test);
    }

    tkr_parse(tkr, "{@ [a = 1, b = 2] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(nodearr_len(array_elems->nodearr) == 2);
        simple_assign = nodearr_get(array_elems->nodearr, 0)->real;
        assert(nodearr_len(simple_assign->nodearr) == 2);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
        assert(test);
        simple_assign = nodearr_get(array_elems->nodearr, 1)->real;
        assert(nodearr_len(simple_assign->nodearr) == 2);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
        assert(test);
    }

    tkr_parse(tkr, "{@ [1, a = 2] @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        array = atom->array->real;
        array_elems = array->array_elems->real;
        assert(nodearr_len(array_elems->nodearr) == 2);
        simple_assign = nodearr_get(array_elems->nodearr, 0)->real;
        assert(nodearr_len(simple_assign->nodearr) == 1);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
        assert(test);
        simple_assign = nodearr_get(array_elems->nodearr, 1)->real;
        assert(nodearr_len(simple_assign->nodearr) == 2);
        test = nodearr_get(simple_assign->nodearr, 0)->real;
    }


    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_asscalc(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_identifier_t *identifier;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_digit_t *digit;
    node_string_t *string;
    node_augassign_t *augassign;
    node_t *node;

    tkr_parse(tkr, "{@ a += 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_ADD_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ a += \"b\" @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_ADD_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ a -= 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_SUB_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ a *= 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;

        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    /* tkr_parse(tkr, "{@ a /= 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_DIV_ASS);
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        factor = nodearr_get(asscalc->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    } */
/*
    tkr_parse(tkr, "{@ func() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }
*/
    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_atom(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_identifier_t *identifier;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_digit_t *digit;
    node_string_t *string;
    node_nil_t *nil;
    node_false_t *false_;
    node_true_t *true_;

    tkr_parse(tkr, "{@ nil @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->nil->type == NODE_TYPE_NIL);
        nil = atom->nil->real;
        assert(nil);
    }

    tkr_parse(tkr, "{@ false @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->false_->type == NODE_TYPE_FALSE);
        false_ = atom->false_->real;
        assert(false_);
    }

    tkr_parse(tkr, "{@ true @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->true_->type == NODE_TYPE_TRUE);
        true_ = atom->true_->real;
        assert(true_);
    }

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->digit->type == NODE_TYPE_DIGIT);
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 1);
    }

    /* tkr_parse(tkr, "{@ -1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        index = nodearr_get(term->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        assert(atom->digit->type == NODE_TYPE_DIGIT);
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == -1);
    } */

    tkr_parse(tkr, "{@ \"abc\" @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->string->type == NODE_TYPE_STRING);
        string = atom->string->real;
        assert(string);
        assert(!strcmp(string->string, "abc"));
    }

    tkr_parse(tkr, "{@ var @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        assert(atom->identifier->type == NODE_TYPE_IDENTIFIER);
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "var"));
    }
/*
    tkr_parse(tkr, "{@ f() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        assert(atom->caller->type == NODE_TYPE_CALLER);
        caller = atom->caller->real;
        assert(caller);
    }
*/

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_compile(void) {
    // head
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    const node_t *root;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_text_block_t *text_block;
    node_elems_t *elems;
    node_stmt_t *stmt;
    node_if_stmt_t *if_stmt;
    node_for_stmt_t *for_stmt;
    node_elif_stmt_t *elif_stmt;
    node_else_stmt_t *else_stmt;
    node_identifier_t *identifier;
    node_formula_t *formula;
    node_multi_assign_t *multi_assign;
    node_assign_t *assign;
    node_assign_list_t *assign_list;
    node_test_list_t *test_list;
    node_test_t *test;
    node_or_test_t *or_test;
    node_and_test_t *and_test;
    node_not_test_t *not_test;
    node_comparison_t *comparison;
    node_expr_t *expr;
    node_term_t *term;
    node_negative_t *negative;
    node_chain_t *chain;
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_digit_t *digit;
    node_string_t *string;
    node_break_stmt_t *break_stmt;
    node_continue_stmt_t *continue_stmt;
    node_def_t *def;
    node_func_def_t *func_def;
    node_func_def_params_t *func_def_params;
    node_func_def_args_t *func_def_args;

    /***********
    * func_def *
    ***********/

    tkr_parse(tkr, "{@ def func(a, b): end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
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
        assert(nodearr_len(func_def_args->identifiers) == 2);
        identifier = nodearr_get(func_def_args->identifiers, 0)->real;
        assert(!strcmp(identifier->identifier, "a"));
        identifier = nodearr_get(func_def_args->identifiers, 1)->real;
        assert(!strcmp(identifier->identifier, "b"));
    }

    tkr_parse(tkr, "{@ def func(): end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == NODE_TYPE_DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == NODE_TYPE_FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == NODE_TYPE_IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == NODE_TYPE_FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == NODE_TYPE_FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(nodearr_len(func_def_args->identifiers) == 0);
    }

    tkr_parse(tkr, "{@ def func(): a = 1 end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == NODE_TYPE_DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == NODE_TYPE_FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == NODE_TYPE_IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == NODE_TYPE_FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == NODE_TYPE_FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(nodearr_len(func_def_args->identifiers) == 0);

        assert(func_def->contents);
        elems = nodearr_get(func_def->contents, 0)->real;
        assert(elems);
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        assert(elems);
        assert(elems->def);
        assert(elems->def->type == NODE_TYPE_DEF);
        def = elems->def->real;
        assert(def);
        assert(def->func_def);
        assert(def->func_def->type == NODE_TYPE_FUNC_DEF);
        func_def = def->func_def->real;
        assert(func_def->identifier);
        assert(func_def->identifier->type == NODE_TYPE_IDENTIFIER);
        identifier = func_def->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        assert(func_def->func_def_params->type == NODE_TYPE_FUNC_DEF_PARAMS);
        func_def_params = func_def->func_def_params->real;
        assert(func_def_params->func_def_args->type == NODE_TYPE_FUNC_DEF_ARGS);
        func_def_args = func_def_params->func_def_args->real;
        assert(nodearr_len(func_def_args->identifiers) == 0);

        elems = nodearr_get(func_def->contents, 0)->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
    tkr_parse(tkr, "{@ func() + 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        term = nodearr_get(expr->nodearr, 2)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }
*/
/*
    tkr_parse(tkr, "{@ my.func() @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }

    tkr_parse(tkr, "{@ my.func(1) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    tkr_parse(tkr, "{@ my.func(1, 2) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    tkr_parse(tkr, "{@ my.func(\"abc\") @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(nodearr_get(test_list->nodearr, 0));
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
    }

    tkr_parse(tkr, "{@ my.func(\"abc\", \"def\") @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(nodearr_get(test_list->nodearr, 0));
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
    }

    tkr_parse(tkr, "{@ my.func(\"abc\", \"def\", \"ghi\") @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(nodearr_get(test_list->nodearr, 0));
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "ghi"));
    }

    tkr_parse(tkr, "{@ my.func(\"\", \"\") @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier = caller->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        test_list = caller->test_list->real;
        assert(nodearr_get(test_list->nodearr, 0));
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        dot = nodearr_get(term->nodearr, 0)->real;
        index = nodearr_get(dot->nodearr, 0)->real;
        factor = index->factor->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
    }
*/
    /************
    * test_list *
    ************/

    tkr_parse(tkr, "{@ 1, 2 @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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
        test = nodearr_get(test_list->nodearr, 1)->real;
        assert(test);
        assert(test->or_test);
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1, 2, 3 @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ \"abc\", \"def\" @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ \"abc\", \"def\", \"ghi\" @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1, \"def\" @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1, var @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1, var, \"abc\" @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1, var, \"abc\", func() @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 3)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->formula != NULL);
        assert(elems->formula->type == NODE_TYPE_FORMULA);
        assert(elems->formula->real != NULL);
        formula = elems->formula->real;
        assert(formula->multi_assign != NULL);
        assert(formula->multi_assign->type == NODE_TYPE_MULTI_ASSIGN);
        assert(formula->multi_assign->real != NULL);
        multi_assign = formula->multi_assign->real;
        assert(nodearr_get(multi_assign->nodearr, 0) != NULL);
        assert(nodearr_get(multi_assign->nodearr, 0)->type == NODE_TYPE_TEST_LIST);
        assert(nodearr_get(multi_assign->nodearr, 0)->real != NULL);
        assert(nodearr_len(multi_assign->nodearr) == 1);
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        assert(nodearr_get(test_list->nodearr, 0) != NULL);
        assert(nodearr_get(test_list->nodearr, 0)->type == NODE_TYPE_TEST);
        assert(nodearr_get(test_list->nodearr, 0)->real != NULL);
        assert(nodearr_get(test_list->nodearr, 1) == NULL);
        test = nodearr_get(test_list->nodearr, 0)->real;
        assert(test->or_test != NULL);
        assert(test->or_test->type == NODE_TYPE_OR_TEST);
        assert(test->or_test->real != NULL);
        or_test = test->or_test->real;
        assert(nodearr_get(or_test->nodearr, 0) != NULL);
        assert(nodearr_get(or_test->nodearr, 0)->type == NODE_TYPE_AND_TEST);
        assert(nodearr_get(or_test->nodearr, 0)->real != NULL);
        assert(nodearr_len(or_test->nodearr) == 1);
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        assert(and_test->nodearr != NULL);
        assert(nodearr_len(and_test->nodearr) == 1);
        assert(nodearr_get(and_test->nodearr, 0)->real != NULL);
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        assert(not_test->not_test == NULL);
        assert(not_test->comparison != NULL);
        assert(not_test->comparison->type == NODE_TYPE_COMPARISON);
        assert(not_test->comparison->real != NULL);
        comparison = not_test->comparison->real;
        // TODO
    }

    tkr_parse(tkr, "{@ 1 or 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        assert(and_test);
        assert(and_test->nodearr);
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        assert(not_test);
        assert(not_test->comparison);
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ 1 or 1 or 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 2)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ 1 and 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = nodearr_get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ 1 and 1 and 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = nodearr_get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = nodearr_get(and_test->nodearr, 2)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ not 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ not not 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ 1 or 1 and 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ 1 and 1 or 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = nodearr_get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ not 1 or 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        and_test = nodearr_get(or_test->nodearr, 1)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    tkr_parse(tkr, "{@ not 1 and 1 @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        multi_assign = formula->multi_assign->real;
        test_list = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(test_list->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        not_test = not_test->not_test->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
        not_test = nodearr_get(and_test->nodearr, 1)->real;
        comparison = not_test->comparison->real;
        assert(comparison != NULL);
    }

    /*********
    * blocks *
    *********/

    tkr_parse(tkr, "{@@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    tkr_parse(tkr, "abc{@@}def");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
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

    tkr_parse(tkr, "{@@}{@@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        blocks = blocks->blocks->real;
        code_block = blocks->code_block->real;
        assert(code_block != NULL);
        assert(code_block->elems == NULL);
    }

    tkr_parse(tkr, "{@@}abc{@@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
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

    tkr_parse(tkr, "{@\n@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    tkr_parse(tkr, "{@\n\n@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems == NULL);
    }

    /***************
    * if statement *
    ***************/

    tkr_parse(tkr, "{@ if 1: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1 + 2: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        stmt = elems->stmt->real;
        if_stmt = stmt->if_stmt->real;
        test = if_stmt->test->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        asscalc = nodearr_get(comparison->nodearr, 0)->real;
        expr = nodearr_get(asscalc->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
        assert(negative);
        chain = negative->chain->real;
        assert(chain);
        factor = chain->factor->real;
        assert(factor);
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        negative = nodearr_get(term->nodearr, 0)->real;
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

    tkr_parse(tkr, "abc{@ if 1: end @}def");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    tkr_parse(tkr, "{@\n\nif 1: end\n\n@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@\n\nif 1:\n\nend\n\n@}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: else: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
        assert(else_stmt);
    }

    tkr_parse(tkr, "{@ if 1:\n\nelse:\n\nend @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
    }

    tkr_parse(tkr, "{@ if 1: elif 2: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nelif 2:\n\nend @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: elif 2: else: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    tkr_parse(tkr, "{@ if 1:\n\nelif 2:\n\nelse:\n\nend @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    tkr_parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
    }

    tkr_parse(tkr, "{@ if 1: if 2: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = nodearr_get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nif 2:\n\nend\n\nend @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = nodearr_get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
    }

    tkr_parse(tkr, "{@ if 1: if 2: end if 3: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = nodearr_get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems != NULL);
        assert(elems->elems->type == NODE_TYPE_ELEMS);
        assert(elems->elems->real != NULL);
        stmt = elems->stmt->real;
        elems = elems->elems->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = nodearr_get(if_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ else: @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        stmt = elems->stmt->real;
        if_stmt = stmt->if_stmt->real;
        blocks = nodearr_get(if_stmt->contents, 0)->real;
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 2: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = nodearr_get(if_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ if 2: end @}def{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = nodearr_get(if_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks == NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    /****************
    * for statement *
    ****************/

    tkr_parse(tkr, "{@ for 1; 1; 1: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: if 1: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        elems = nodearr_get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: for 1; 1; 1: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        elems = nodearr_get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        assert(stmt->if_stmt == NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for 1: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for 1: if 1: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        elems = nodearr_get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ for 1: @}{@ if 1: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ for 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for: end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for: @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    tkr_parse(tkr, "{@ for: @}{@ if 1: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula == NULL);
        assert(for_stmt->comp_formula == NULL);
        assert(for_stmt->update_formula == NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: for 1; 1; 1: end end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(nodearr_len(if_stmt->contents) == 1);
        elems = nodearr_get(if_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 0);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: @}{@ if 1: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: @}abc{@ if 1: end @}def{@ end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        blocks = nodearr_get(for_stmt->contents, 0)->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
        blocks = blocks->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        assert(blocks->blocks != NULL);
        assert(blocks->blocks->type == NODE_TYPE_BLOCKS);
        assert(blocks->blocks->real != NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        assert(stmt->if_stmt != NULL);
        assert(stmt->if_stmt->type == NODE_TYPE_IF_STMT);
        assert(stmt->if_stmt->real != NULL);
        if_stmt = stmt->if_stmt->real;
        assert(if_stmt->test != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = blocks->blocks->real;
        assert(blocks->code_block == NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block != NULL);
        assert(blocks->text_block->type == NODE_TYPE_TEXT_BLOCK);
        assert(blocks->text_block->real != NULL);
        assert(blocks->blocks == NULL);
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "def"));
    }

    /*******
    * jump *
    *******/

    tkr_parse(tkr, "{@ for 1; 1; 1: break end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        elems = nodearr_get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        break_stmt = stmt->break_stmt->real;
        assert(break_stmt);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: continue end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        assert(root != NULL);
        assert(root->type == NODE_TYPE_PROGRAM);
        assert(root->real != NULL);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt == NULL);
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt != NULL);
        assert(stmt->for_stmt->type == NODE_TYPE_FOR_STMT);
        assert(stmt->for_stmt->real != NULL);
        for_stmt = stmt->for_stmt->real;
        assert(for_stmt->init_formula != NULL);
        assert(for_stmt->init_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->init_formula->real != NULL);
        assert(for_stmt->comp_formula != NULL);
        assert(for_stmt->comp_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->comp_formula->real != NULL);
        assert(for_stmt->update_formula != NULL);
        assert(for_stmt->update_formula->type == NODE_TYPE_FORMULA);
        assert(for_stmt->update_formula->real != NULL);
        assert(nodearr_len(for_stmt->contents) == 1);
        elems = nodearr_get(for_stmt->contents, 0)->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        continue_stmt = stmt->continue_stmt->real;
        assert(continue_stmt);
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
    // tail
}

static void
test_cc_import_stmt(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    const node_t *root;
    node_t *node;
    node_program_t *program;
    node_blocks_t *blocks;
    node_code_block_t *code_block;
    node_elems_t *elems;
    node_stmt_t *stmt;
    node_import_stmt_t *import_stmt;
    node_import_as_stmt_t *import_as_stmt;
    node_from_import_stmt_t *from_import_stmt;
    node_import_vars_t *import_vars;
    node_import_var_t *import_var;
    node_string_t *path;
    node_identifier_t *identifier;

    /**********************
    * import as statement *
    **********************/

    tkr_parse(tkr, "{@ import \"path/to/module\" as mod @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt);
        assert(import_stmt->import_as_stmt->type == NODE_TYPE_IMPORT_AS_STMT);
        assert(import_stmt->import_as_stmt->real);
        assert(import_stmt->from_import_stmt == NULL);

        import_as_stmt = import_stmt->import_as_stmt->real;
        assert(import_as_stmt);
        assert(import_as_stmt->path);
        assert(import_as_stmt->path->type == NODE_TYPE_STRING);
        assert(import_as_stmt->path->real);
        assert(import_as_stmt->alias);
        assert(import_as_stmt->alias->type == NODE_TYPE_IDENTIFIER);
        assert(import_as_stmt->alias->real);

        path = import_as_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        identifier = import_as_stmt->alias->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "mod"));
    }

    tkr_parse(tkr, "{@ import \"path/to/module\" as mod \n @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ import @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found path in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \"path/to/module\" @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found keyword 'as' in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \"path/to/module\" as @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found alias in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \n\"path/to/module\" as mod @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found path in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \"path/to/module\" \n as mod @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found keyword 'as' in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \"path/to/module\" as \n mod @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found alias in compile import as statement"));
    }

    /************************
    * from import statement *
    ************************/

    tkr_parse(tkr, "{@ from \"path/to/module\" import func @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == NODE_TYPE_FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == NODE_TYPE_STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == NODE_TYPE_IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(nodearr_len(import_vars->nodearr) == 1);

        node = nodearr_get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);

        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "func"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import func as f @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == NODE_TYPE_FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == NODE_TYPE_STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == NODE_TYPE_IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(nodearr_len(import_vars->nodearr) == 1);

        node = nodearr_get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias);
        assert(import_var->alias->type == NODE_TYPE_IDENTIFIER);
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

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == NODE_TYPE_FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == NODE_TYPE_STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == NODE_TYPE_IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(nodearr_len(import_vars->nodearr) == 1);

        node = nodearr_get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);

        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);

        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa, bbb ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == NODE_TYPE_FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == NODE_TYPE_STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == NODE_TYPE_IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(nodearr_len(import_vars->nodearr) == 2);

        node = nodearr_get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));

        node = nodearr_get(import_vars->nodearr, 1);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "bbb"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa, bbb, ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import (\naaa,\nbbb,\n) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, bbb ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        root = ast_getc_root(ast);
        assert(root->type == NODE_TYPE_PROGRAM);
        program = root->real;
        assert(program->blocks != NULL);
        assert(program->blocks->type == NODE_TYPE_BLOCKS);
        assert(program->blocks->real != NULL);
        blocks = program->blocks->real;
        assert(blocks->code_block != NULL);
        assert(blocks->code_block->type == NODE_TYPE_CODE_BLOCK);
        assert(blocks->code_block->real != NULL);
        assert(blocks->ref_block == NULL);
        assert(blocks->text_block == NULL);
        code_block = blocks->code_block->real;
        assert(code_block->elems != NULL);
        assert(code_block->elems->type == NODE_TYPE_ELEMS);
        assert(code_block->elems->real != NULL);
        elems = code_block->elems->real;
        assert(elems->stmt != NULL);
        assert(elems->stmt->type == NODE_TYPE_STMT);
        assert(elems->stmt->real != NULL);
        assert(elems->formula == NULL);
        assert(elems->elems == NULL);
        stmt = elems->stmt->real;
        assert(stmt->import_stmt != NULL);
        assert(stmt->import_stmt->type == NODE_TYPE_IMPORT_STMT);
        assert(stmt->import_stmt->real != NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt);
        assert(import_stmt->import_as_stmt == NULL);
        assert(import_stmt->from_import_stmt);
        assert(import_stmt->from_import_stmt->type == NODE_TYPE_FROM_IMPORT_STMT);
        assert(import_stmt->from_import_stmt->real);

        from_import_stmt = import_stmt->from_import_stmt->real;
        assert(from_import_stmt);
        assert(from_import_stmt->path);
        assert(from_import_stmt->path->type == NODE_TYPE_STRING);
        assert(from_import_stmt->path->real);
        assert(from_import_stmt->import_vars);
        assert(from_import_stmt->import_vars->type == NODE_TYPE_IMPORT_VARS);
        assert(from_import_stmt->import_vars->real);

        path = from_import_stmt->path->real;
        assert(path);
        assert(path->string);
        assert(!strcmp(path->string, "path/to/module"));

        import_vars = from_import_stmt->import_vars->real;
        assert(import_vars);
        assert(import_vars->nodearr);
        assert(nodearr_len(import_vars->nodearr) == 2);

        node = nodearr_get(import_vars->nodearr, 0);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias);
        assert(import_var->alias->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->alias->real);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "aaa"));
        identifier = import_var->alias->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "a"));

        node = nodearr_get(import_vars->nodearr, 1);
        assert(node);
        assert(node->type == NODE_TYPE_IMPORT_VAR);
        assert(node->real);
        import_var = node->real;
        assert(import_var);
        assert(import_var->identifier);
        assert(import_var->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(import_var->identifier->real);
        assert(import_var->alias == NULL);
        identifier = import_var->identifier->real;
        assert(identifier);
        assert(identifier->identifier);
        assert(!strcmp(identifier->identifier, "bbb"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a, bbb ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a, \nbbb ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import (\n aaa as a,\n bbb \n) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ from @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found path in compile from import statement"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import in compile from import statement"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import variables in compile from import statement"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import \naaa @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import variables in compile from import statement"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import aaa as @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found second identifier in compile import variable"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid token 5 in compile import variables"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa as a @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid token 5 in compile import variables"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import variable in compile import variables"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa as a, bbb @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid token 5 in compile import variables"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa \n as a ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid token 44 in compile import variables"));
    }

    tkr_parse(tkr, "{@ from \"path/to/module\" import ( aaa as \n a ) @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found second identifier in compile import variable"));
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

static void
test_cc_func_def(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);

    tkr_parse(tkr, "{@ def func():\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def func():\n"
    "@}123{@\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def func():\n"
    "@}123{@\n"
    "@}223{@\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "@}123{@\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def func():\n"
    "   i = 0\n"
    "@}123{@\n"
    "   j = 1\n"
    "end @}");
    {
        ast_clear(ast);
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_errors(ast));
    }

    tkr_del(tkr);
    ast_del(ast);
    config_del(config);
}

/**
 * 0 memory leaks
 * 2020/02/27
 */
static const struct testcase
compiler_tests[] = {
    {"cc_compile", test_cc_compile},
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
test_trv_comparison(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = 1 == 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 == \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare equal with int"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 1 == f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare equal with int"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" == 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare equal with string"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = \"abc\" == f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare equal with string"));
    }

    tkr_parse(tkr, "{@ a = 1 == 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 == 1 == 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 == 1 == 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 != \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare not equal with int"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" != 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare not equal with string"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" != \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f != 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare not equal with func"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 1 != f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare not equal with int"));
    }

    tkr_parse(tkr, "{@ a = 0 != 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 != 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 != 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    /**
     * well-formed on Python
     * ill-formed on Ruby
     */
    tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" == \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't compare equal with bool"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    /*********************************
    * boolean can convert to integer *
    *********************************/

    tkr_parse(tkr, "{@ a = true == 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = false == 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    /******
    * lte *
    ******/

    tkr_parse(tkr, "{@ a = 1 <= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 2 <= 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true <= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true <= 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 0 <= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 2 <= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    /******
    * gte *
    ******/

    tkr_parse(tkr, "{@ a = 1 >= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 2 >= 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true >= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true >= 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 0 >= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 2 >= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    /*****
    * lt *
    *****/

    tkr_parse(tkr, "{@ a = 1 < 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 2 < 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true < 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true < 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 0 < true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 < true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    /*****
    * gt *
    *****/

    tkr_parse(tkr, "{@ a = 1 > 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 2 > 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true > 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true > 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 0 > true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 2 > true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_array_index(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    // tkr_parse(tkr, "{@ a[0] @}");
    // {
    ast_clear(ast);
    //     (cc_compile(ast, tkr_get_tokens(tkr)));
    //     ctx_clear(ctx);
    //     (trv_traverse(ast, ctx));
    //     assert(ast_has_errors(ast));
    //     assert(!strcmp(ast_getc_first_error_message(ast), "can't index access. \"a\" is not defined"));
    // }

    tkr_parse(tkr, "{@ a = [1, 2] \n @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] \n @}{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] \n @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] \n @}{: a[2] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "index out of range"));
    }

    /* tkr_parse(tkr, "{@ a = [1, 2] \n @}{: a[-1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "index out of range of array"));
    } */

    // tkr_parse(tkr, "{@ a = (b, c = 1, 2)[0] \n @}{: a :}");
    // {
    //     ast_clear(ast);
    //     cc_compile(ast, tkr_get_tokens(tkr));
    //     ctx_clear(ctx);
    //     (trv_traverse(ast, ctx));
    //     assert(!ast_has_errors(ast));
    //     assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    // }

    // tkr_parse(tkr, "{@ a = (b, c = 1, 2)[1] \n @}{: a :}");
    // {
    ast_clear(ast);
    //     cc_compile(ast, tkr_get_tokens(tkr));
    //     ctx_clear(ctx);
    //     (trv_traverse(ast, ctx));
    //     assert(!ast_has_errors(ast));
    //     assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    // }

    tkr_parse(tkr, "{@ a = [[1, 2]] \n @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [[1, 2]] \n @}{: a[0][0] :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_text_block_old(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "abc");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_ref_block_old(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: nil :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{: false :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{: true :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 123 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    tkr_parse(tkr, "{: \"abc\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined in ref block"));
    }

    /* tkr_parse(tkr, "{: alias(\"dtl\", \"run bin/date-line.py\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    } */

    tkr_parse(tkr, "{: 1 + 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: 1 + 1 + 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{: [1, 2] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_1(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "@}{: string :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(module)"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(module)"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "   string.b = string.a\n"
    "@}{: string.b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_2(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_atom_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ nil @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ false @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ true @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ \"abc\" @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ var @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    trv_cleanup;
}

static void
test_trv_array(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [1] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [1] \n b = a @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array),(array)"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [b = 1, c = 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [1, b = 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_index(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a[0] :},{: a[1] :},{: a[2] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a,b,c"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = {\"a\": 1, \"b\": 2} @}{: a[\"a\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = {\"a\": 1, \"b\": 2} @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = a[0] or a[1] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = a[0] and a[1] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "b"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = not a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = a[0] or a[1] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = a[0] and a[1] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = not a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] or a[\"b\"] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] and a[\"b\"] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = not a[\"a\"] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = a[0] == \"a\" @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = \"a\" == a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = a[0] != \"a\" @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n b = \"a\" != a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = a[0] == 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = 1 == a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = a[0] != 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n b = 1 != a[0] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] == 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = 1 == a[\"a\"] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = a[\"a\"] != 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2} \n b = 1 != a[\"a\"] @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n if a[0] == \"a\": puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n a[0] = 3 @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,2"));
    }

    tkr_parse(tkr, "{@ a = [1,2] \n a[0] = 3 \n a[1] = 4 @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,4"));
    }

    tkr_parse(tkr, "{@ a = [\"a\",\"b\"] \n a[0] = \"c\" @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "c,b"));
    }

    tkr_parse(tkr, "{@ a = [\"a\",\"b\"] \n a[0] = \"c\" \n a[1] = \"d\" @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "c,d"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2 } \n a[\"a\"] = 3 @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,2"));
    }

    tkr_parse(tkr, "{@ a = {\"a\":1, \"b\":2 } \n a[\"a\"] = 3 \n a[\"b\"] = 4 @}{: a[\"a\"] :},{: a[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,4"));
    }

    tkr_parse(tkr, "{@ a = [] a.push(1) @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [\"abc_def\"] @}{: a[0].camel() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcDef"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_string_index(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = \"ab\" \n @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a"));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n @}{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "b"));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n @}{: a[2] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "index out of range"));
    }

    tkr_parse(tkr, "{@ a = (\"a\" + \"b\")[0] \n @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a"));
    }

    tkr_parse(tkr, "{@ a = (\"a\" + \"b\")[1] \n @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "b"));
    }

    tkr_parse(tkr, "{@ a = \"ab\"[0][0] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_multi_assign(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    // error

    tkr_parse(tkr, "{@ a, b = 1, 2, 3 @}{: a :} {: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't assign array to array. not same length"));
    }

    tkr_parse(tkr, "{@ a, b = 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right operand (1)"));
    }

    // success

    tkr_parse(tkr, "{@ a, b = 1, 2 @}{: a :} {: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2"));
    }

    tkr_parse(tkr, "{@ a = 1, 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_and_test(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    // nil and objects

    tkr_parse(tkr, "{@ a = nil and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = nil and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = nil and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    // digit and objects

    tkr_parse(tkr, "{@ a = 1 and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1 and 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 1 and 2 and 3 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = 1 and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = 0 and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = 0 and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = 1 and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 0 and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = 0 and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = 0 and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = 1 and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = 1 and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = 0 and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = 0 and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = 1 and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 1 and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 0 and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    // bool and objects

    tkr_parse(tkr, "{@ a = true and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = false and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = true and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = true and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = false and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = false and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = false and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = true and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = false and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = false and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = true and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    // string and other

    tkr_parse(tkr, "{@ a = \"abc\" and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = \"abc\" and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = \"abc\" and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = \"abc\" and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"\" and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = \"\" and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"\" and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"\" and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = \"\" and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = \"\" and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = \"\" and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = \"\" and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = \"\" and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    // array and other

    tkr_parse(tkr, "{@ a = [1, 2] and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = [1, 2] and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = [1, 2] and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = [1, 2] and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = [] and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = [] and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [] and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = [] and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = [] and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = [] and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = [] and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = [] and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    // dict and other

    tkr_parse(tkr, "{@ a = {\"k\": 1} and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {\"k\": 1} and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = {\"k\": 1} and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = {\"k\": 1} and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = {\"k\": 1} and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = {} and nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = {} and false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = {} and true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {} and 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = {} and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {} and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = {} and \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {} and [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = {} and [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {} and {} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = {} and {\"k\":1} @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = {} and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = {} and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = {} and b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    //

    tkr_parse(tkr, "{@ a = \"abc\" and 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1 and \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 1 and f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_list(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    // success

    tkr_parse(tkr, "{@ a = nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\"\n b = a @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1, b = 2 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = 1 + 2, b = 3 * 4 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,12"));
    }

    tkr_parse(tkr, "{@ a = 1, b = 2, c = 3 @}{: a :},{: b :},{: c :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2,3"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = a = 1 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = a = 1, c = b = 1 @}{: a :},{: b :},{: c :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,1"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a, b = 1, 2 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = alias.set(\"\", \"\") @}{: a :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = alias.set(\"\", \"\")\n b = alias.set(\"\", \"\") @}{: a :},{: b :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil,nil"));
    }

    tkr_parse(tkr, "{@ a = opts.get(\"abc\") @}{: a :}");
    {
        char *argv[] = {
            "make",
            "-abc",
            "def",
            NULL,
        };
        opts_t *opts = opts_new();
        assert(opts_parse(opts, 3, argv));
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        ast_move_opts(ast, opts);
        (trv_traverse(ast, ctx));
        ast_move_opts(ast, NULL);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_test_list(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ 1, 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ 1, \"abc\", var, alias.set(\"\", \"\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 0 \n a += 1, b += 2 @}{: a :} {: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_negative_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: -1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "-1"));
    }

    tkr_parse(tkr, "{: 1 + -1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: -1 + -1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "-2"));
    }

    tkr_parse(tkr, "{: 1 - -1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: -1 - -1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: 1-1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_dot_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: \"ABC\".lower() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{: \"abc\".upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ABC"));
    }

    tkr_parse(tkr, "{: \"ABC\".lower().upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ABC"));
    }

    tkr_parse(tkr, "{: alias.set(\"a\", \"b\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_get_alias_value(ctx, "a"), "b"));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_dot_1(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "@}{: string.variable.upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "STRING"));
    }

    trv_cleanup;
}

static void
test_trv_dot_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    arr = [1, 2]\n"
    "    dst = []\n"
    "    dst.push(arr[1])\n"
    "@}{: dst[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_dot_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    arr = [1, 2]\n"
    "    dst = []\n"
    "    dst.push(arr.pop())\n"
    "@}{: dst[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_dot_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    arr = [[1, 2], [3, 4]]\n"
    "    dst = []\n"
    "    n = dst.push(arr.pop().pop()).pop()\n"
    "@}{: n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    trv_cleanup;
}

static void
test_trv_dot_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    arr = [[[[[[[[1, 2]]]]]]]]\n"
    "    dst = []\n"
    "    n = dst.push(arr.pop().pop().pop().pop().pop().pop().pop().pop()).pop()\n"
    "@}{: n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_call(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): return 1 end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ puts(1) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ def f(): return 1 end \n funcs = { \"a\": f } @}{: funcs[\"a\"]() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def a(n): return n*2 end \n def b(): return a end @}{: b()(2) :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ def f(a): return a end @}{: f(1) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def f(a, b): return a + b end @}{: f(1, 2) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ def f(): return true end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): return 0 end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ def f(): return 1 + 2 end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ def f(): @}abc{@ end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcnil"));
    }

    tkr_parse(tkr, "{@ def f(): @}abc{@ a = 1 @}def{@ end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcdefnil"));
    }

    tkr_parse(tkr, "{@ def f(): @}abc{@ a = 1 @}{: a :}{@ end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc1nil"));
    }

    tkr_parse(tkr, "{@ def f(a): @}{: a :}{@ b = 123 @}{: b :}{@ end @}{: f(\"abc\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc123nil"));
    }

    tkr_parse(tkr,
        "{@\n"
        "    def usage():\n"
        "@}abc{@\n"
        "    end\n"
        "@}{: usage() :}"
    );
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcnil"));
    }

    tkr_parse(tkr,
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
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "hi\n"));
    }

    tkr_parse(tkr,
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
    );
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "hi\n"));
    }

    tkr_parse(tkr,
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
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_string(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /********
    * upper *
    ********/

    tkr_parse(tkr, "{: \"abc\".upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ABC"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n @}{: a.upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ABC"));
    }

    tkr_parse(tkr, "{: nil.upper() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"upper\" is not defined"));
    }

    /********
    * lower *
    ********/

    tkr_parse(tkr, "{: \"ABC\".lower() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"ABC\" \n @}{: a.lower() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{: nil.lower() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"lower\" is not defined"));
    }

    /*************
    * capitalize *
    *************/

    tkr_parse(tkr, "{: \"abc\".capitalize() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "Abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" \n @}{: a.capitalize() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "Abc"));
    }

    tkr_parse(tkr, "{: nil.capitalize() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"capitalize\" is not defined"));
    }

    /********
    * snake *
    ********/

    tkr_parse(tkr, "{: \"abcDef\".snake() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc_def"));
    }

    tkr_parse(tkr, "{@ a = \"abcDef\" \n @}{: a.snake() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc_def"));
    }

    tkr_parse(tkr, "{: nil.snake() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"snake\" is not defined"));
    }

    /********
    * camel *
    ********/

    tkr_parse(tkr, "{: \"camel_case\".camel() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "camelCase"));
    }

    tkr_parse(tkr, "{@ a = \"camel_case\" \n @}{: a.camel() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "camelCase"));
    }

    tkr_parse(tkr, "{: nil.camel() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"camel\" is not defined"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_functions(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /********
    * alias *
    ********/

    tkr_parse(tkr, "{@ alias.set(\"abc\", \"def\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
        const char *value = alinfo_getc_value(alinfo, "abc");
        assert(value);
        assert(!strcmp(value, "def"));
    }

    tkr_parse(tkr, "{@ alias.set(\"abc\", \"def\", \"ghi\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
        const char *value = alinfo_getc_value(alinfo, "abc");
        assert(value);
        assert(!strcmp(value, "def"));
        const char *desc = alinfo_getc_desc(alinfo, "abc");
        assert(desc);
        assert(!strcmp(desc, "ghi"));
    }

    /*******
    * opts *
    *******/

    tkr_parse(tkr, "{: opts.get(\"abc\") :}");
    {
        opts_t *opts = opts_new();
        char *argv[] = {
            "make",
            "--abc",
            "def",
            NULL,
        };
        opts_parse(opts, 3, argv);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        ast_move_opts(ast, opts);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{: opts.has(\"abc\") :}");
    {
        opts_t *opts = opts_new();
        char *argv[] = {
            "make",
            "--abc",
            NULL,
        };
        opts_parse(opts, 2, argv);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        ast_move_opts(ast, opts);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{: opts.has(\"def\") :}");
    {
        opts_t *opts = opts_new();
        char *argv[] = {
            "make",
            "--abc",
            NULL,
        };
        opts_parse(opts, 2, argv);
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        ast_move_opts(ast, opts);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    /*******
    * puts *
    *******/

    tkr_parse(tkr, "{@ puts() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\n"));
    }

    tkr_parse(tkr, "{@ puts(1) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ puts(1, 2) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2\n"));
    }

    tkr_parse(tkr, "{@ puts(1, \"abc\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 abc\n"));
    }

    tkr_parse(tkr, "{@ puts(\"abc\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc\n"));
    }

    /********
    * eputs *
    ********/

    tkr_parse(tkr, "{@ eputs() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stderr_buf(ctx), "\n"));
    }

    tkr_parse(tkr, "{@ eputs(1) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stderr_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ eputs(1, 2) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stderr_buf(ctx), "1 2\n"));
    }

    tkr_parse(tkr, "{@ eputs(1, \"abc\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stderr_buf(ctx), "1 abc\n"));
    }

    tkr_parse(tkr, "{@ eputs(\"abc\") @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stderr_buf(ctx), "abc\n"));
    }

    /*****
    * id *
    *****/

    tkr_parse(tkr, "{: id(1) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_modules_opts_0(void) {
    trv_ready;

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{: opts.args(0) :},{: opts.args(1) :}");
    {
        int argc = 2;
        char *argv[] = {
            "cmd",
            "aaa",
            NULL
        };
        opts_t *opts = opts_new();
        opts_parse(opts, argc, argv);
        ast_clear(ast);
        ast_move_opts(ast, mem_move(opts));
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "cmd,aaa"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ alias.set(1, 2, 3) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't invoke alias.set. key is not string"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@ alias.set() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't invoke alias.set. too few arguments"));
    }

    tkr_parse(tkr, "{@ alias.set(1, 2, 3) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't invoke alias.set. key is not string"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_alias_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@ alias.set() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
    }

    trv_cleanup;
}

static void
test_trv_builtin_modules_array_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    arr = [1, 2]"
    "    dst = []\n"
    "    dst.push(arr[1])\n"
    "@}{: dst[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_functions_type_dict(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ d = {\"a\": 1} @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def f(d): end \n f({\"a\": 1}) @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def f(d): end @}{: f(1) :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{@ def f(d): end @}{: f({\"a\": 1}) :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
    }

    tkr_parse(tkr, "{: type({ \"a\": 1 }) :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<dict>"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_functions_type(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: type(nil) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<nil>"));
    }

    tkr_parse(tkr, "{: type(1) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<int>"));
    }

    tkr_parse(tkr, "{: type(true) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<bool>"));
    }

    tkr_parse(tkr, "{: type(\"string\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<str>"));
    }

    tkr_parse(tkr, "{: type([1, 2]) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<array>"));
    }

    tkr_parse(tkr, "{: type({ \"a\": 1 }) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<dict>"));
    }

    tkr_parse(tkr, "{@ def f(): end @}{: type(f) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<func>"));
    }

    tkr_parse(tkr, "{@ import \":tests/lang/modules/hello.cap\" as mod @}{: type(mod) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n<module>"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_functions_puts_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ puts(1) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_builtin_functions_len_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: len([1, 2]) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: len([]) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: len(\"12\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: len(\"\") :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_traverse(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /*******
    * test *
    *******/

    // digit or objects

    tkr_parse(tkr, "{@ a = 0 or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = 0 or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 0 or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = 0 or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = 0 or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = 0 or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): return true end \n a = 0 or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 0 or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = 0 or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = 1 or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1 or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    // bool or objects

    tkr_parse(tkr, "{@ a = false or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = true or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = false or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = false or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = true or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = false or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = false or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = false or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = true or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): return true end \n a = false or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): return 0 end \n a = false or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = false or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = false or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = true or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = true or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    // nil or objects

    tkr_parse(tkr, "{@ a = nil or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = nil or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = nil or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = nil or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = nil or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = nil or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = nil or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = nil or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): return true end \n a = nil or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = nil or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = nil or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    // string or objects

    tkr_parse(tkr, "{@ a = \"abc\" or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"\" or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"\" or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = \"\" or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"\" or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = \"\" or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"\" or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"\" or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"def\" or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = \"abc\" or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = \"abc\" or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = \"abc\" or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = \"\" or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): return true end \n a = \"\" or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ def f(): return nil end \n a = \"\" or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ def f(): return nil end \n a = \"abc\" or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"\" or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = \"\" or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    // array or objects

    tkr_parse(tkr, "{@ a = [1, 2] or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ a = [] or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = [] or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ b = 0 \n a = [] or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ b = 1 \n a = [1, 2] or b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = [] or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = [1, 2] or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@ def f(): return 1 end \n a = [] or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def f(): return 0 end \n a = [] or f() @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    // func or objects

    tkr_parse(tkr, "{@ def f(): end \n a = f or nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or [] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = f or [1, 2] @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    // other

    tkr_parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }


    tkr_parse(tkr, "{@ a = 1 \n b = 0 or a @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ def f(): end\n"
        "a = 0 or f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function)"));
    }

    tkr_parse(tkr, "{@ a = 1 or 0 or 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = not nil @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = not 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = not 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = not \"\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = not \"abc\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ def f(): end \n a = not f @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    /*******
    * expr *
    *******/

    tkr_parse(tkr, "{@ a = 1 + 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = 1 + 2 + 3 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6"));
    }

    tkr_parse(tkr, "{@ a = 2 - 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 3 - 2 - 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = 1 + 2 - 3 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" + \"def\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcdef"));
    }

    tkr_parse(tkr, "{@ a = \"123\" \n b = \"abc\" + a + \"def\" @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc123def"));
    }

    /*******
    * term *
    *******/

    tkr_parse(tkr, "{@ a = 2 * 3 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6"));
    }

    tkr_parse(tkr, "{@ a = 2 * 3 * 4 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "24"));
    }

    tkr_parse(tkr, "{@ a = 4 / 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 4 / 2 / 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 4 / (2 / 2) @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{@ a = 1 + ( 2 - 3 ) * 4 / 4 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    /**********
    * asscalc *
    **********/

    tkr_parse(tkr, "{@ a += 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 + 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 1 + 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 1 + (a += 1) @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 \n a += 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = \"a\"\n"
        "a += \"b\" @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ab"));
    }

    tkr_parse(tkr, "{@\n"
        "a = \"x\"\n"
        "def f():\n"
        "   a += \"y\"\n"
        "end\n"
        "f()\n"
        "@}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "xy"));
    }

    tkr_parse(tkr, "{@\n"
        "    def add(a):\n"
        "        a += \"x\"\n"
        "    end\n"
        "\n"
        "   a = \"\"\n"
        "   add(a)\n"
        "@}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    /*******************
    * import statement *
    *******************/

    tkr_parse(tkr, "{@ import alias @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
    }

    tkr_parse(tkr, "{@ import my.alias @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
    }

    /***************
    * if statement *
    ***************/

    tkr_parse(tkr, "{@ if 1: a = 1 end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: a = 1 end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "abc{@ if 1: @}def{@ end @}ghi");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcdefghi"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}abc{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ if 1: @}def{@ end @}ghi{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcdefghi"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ elif 1: @}def{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ elif 0: @}def{@ else: @}ghi{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ghi"));
    }

    /****************
    * for statement *
    ****************/

    tkr_parse(tkr,
        "{@\n"
        "    for a != 0:\n"
        "        break\n"
        "    end\n"
        "@}\n");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i = 0; i != 4; i += 1:\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i = 0, j = 0; i != 4; i += 1, j += 1:\n"
        "   a += 1\n"
        "end @}{: a :} {: i :} {: j :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4 4 4"));
    }

    tkr_parse(tkr, "{@ for i = 0; i != 4; i += 1: @}a{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "aaaa"));
    }

    tkr_parse(tkr, "{@ for i, j = 0, 0; i != 4; i += 1, j += 2: end @}{: i :},{: j :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4,8"));
    }

    tkr_parse(tkr, "{@ i, a = 0, 0 \n for i != 4: a += i \n i += 1 end @}{: i :},{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4,6"));
    }

    tkr_parse(tkr,
        "{@ for i = 0; i != 4; i += 1: @}"
        "hige\n"
        "{@ end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    tkr_parse(tkr,
        "{@ i = 0 for i != 4: @}"
        "hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    tkr_parse(tkr,
        "{@ i = 0 for: @}"
        "{@ if i == 4: break end @}hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "hige\nhige\nhige\nhige\n"));
    }

    /*******
    * jump *
    *******/

    tkr_parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   break\n"
        "end @}{: i :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       break\n"
        "   end\n"
        "end @}{: i :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   continue\n"
        "   a += 1\n"
        "end @}{: i :},{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4,0"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       continue\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       continue\n"
        "   elif i == 3:\n"
        "       continue\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0, b = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   a += 1"
        "   if i == 2:\n"
        "       continue\n"
        "   end\n"
        "   b += 1\n"
        "end @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4,3"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil\n"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return 1\n"
        "end\n"
        "a = func()"
        "@}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return 1\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)\n"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   puts(\"a\")\n"
        "   return 1\n"
        "   puts(\"b\")\n"
        "end\n"
        "puts(func())"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "a\n1\n"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "   return a\n"
        "end\n"
        "x = func()\n"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "a, b = func()\n"
        "@}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   return 1, 2\n"
        "end\n"
        "a = func()\n"
        "@}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    tkr_parse(tkr, "{@\n"
        "def func(a):\n"
        "   return a, a\n"
        "end\n"
        "a, b = func(1)\n"
        "@}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1"));
    }

    /***********
    * func_def *
    ***********/

    tkr_parse(tkr, "{@ def func(): end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}{: a :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "func()"
        "@}{: a :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func(a):\n"
        "   b = a\n"
        "end\n"
        "func(1)"
        "@}{: a :},{: b :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"c\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "c = 1\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@\n"
        "c = 1\n"
        "def func(a, b):\n"
        "   puts(c)\n"
        "   c = a + b\n"
        "   puts(c)\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n3\n1"));
    }

    /*******************
    * escape character *
    *******************/

    tkr_parse(tkr, "{: \"abc\ndef\n\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc\ndef\n"));
    }

    tkr_parse(tkr, "{: \"\tabc\tdef\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\tabc\tdef"));
    }

    // done
    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

/**
 * A test of assign to variable and refer variable
 * object is copy? or refer?
 */
static void
test_trv_assign_and_reference_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = 0\n"
    "@}{: i :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: i :},{: j :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,0"));
    }

    tkr_parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,0,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = 1\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i, j = 1, 1\n"
    "@}{: i :},{: j :},{: id(i) != id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_4(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = 1\n"
    "   j, k = i, i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :},{: id(i) == id(k) :},{: id(j) == id(k) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,true,true,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_5(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = 1, 2\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array),(array),true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_6(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = [1, 2]\n"
    "   j = 3\n"
    "   i[0] = j\n"
    "@}{: i[0] :},{: i[1] :},{: j :},{: id(i[0]) == id(j) :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3,2,3,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_7(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i, j = [1, 2]\n"
    "@}{: i :},{: j :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_8(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i, j = k, l = 1, 2\n"
    "@}{: i :},{: j :},{: k :},{: l :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2,1,2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_9(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = { \"a\": 1 }\n"
    "   j = i\n"
    "@}{: i :},{: j :},{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict),(dict),true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_10(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   i = f(1)"
    "@}{: i :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_11(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   def f():\n"
    "       return 1, 2\n"
    "   end\n"
    "   i, j = f()\n"
    "@}{: i :},{: j :},{: id(i) != id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_12(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a, a\n"
    "   end\n"
    "   k = 1\n"
    "   i, j = f(k)\n"
    "@}{: i :},{: j :},{: id(i) != id(j) :},{: id(k) != id(i) :},{: id(k) != id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,true,true,true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_13(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_and_reference_14(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   i = 0\n"
    "@}{: i :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_15(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   i = j = 0\n"
    "@}{: id(i) == id(j) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    trv_cleanup;
}

static void
test_trv_assign_and_reference_16(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   f(1)"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

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
test_trv_code_block(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ \n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ \n\n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ \n\n1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ 1\n\n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ \n\n1\n\n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@@}{@@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@@}{@@}{@@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "\n{@\n@}\n{@\n@}\n");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\n\n\n"));
    }

    tkr_parse(tkr, "\n{@\n\n\n@}\n{@\n\n\n@}\n");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\n\n\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_ref_block(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 1\n :}");
    {
        assert(tkr_has_error_stack(tkr));
        assert(!strcmp(tkr_getc_first_error_message(tkr), "syntax error. unsupported character \"\n\""));
    }

    tkr_parse(tkr, "{: \n1 :}");
    {
        assert(tkr_has_error_stack(tkr));
        assert(!strcmp(tkr_getc_first_error_message(tkr), "syntax error. unsupported character \"\n\""));
    }

    tkr_parse(tkr, "\n{: 1 :}\n");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\n1\n"));
    }

    tkr_parse(tkr, "{@@}{: 1 :}{@@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 1 :}{@@}{: 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "12"));
    }

    tkr_parse(tkr, "{: 2 * 3 + 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "7"));
    }

    tkr_parse(tkr, "{: \"ab\" * 4 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abababab"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_text_block(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "1");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "1{@@}2");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "12"));
    }

    tkr_parse(tkr, "1{@@}2{@@}3");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    tkr_parse(tkr, "1{: 2 :}3{: 4 :}5");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "12345"));
    }

    tkr_parse(tkr, "1{@@}{: 2 :}{@@}3");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_import_stmt_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /**********************
    * import as statement *
    **********************/

    tkr_parse(tkr, "{@ import \":tests/lang/modules/hello.cap\" as hello @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    tkr_parse(tkr, "{@ import \n \":tests/lang/modules/hello.cap\" as hello @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found path in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \":tests/lang/modules/hello.cap\" \n as hello @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found keyword 'as' in compile import as statement"));
    }

    tkr_parse(tkr, "{@ import \":tests/lang/modules/hello.cap\" as \n hello @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found alias in compile import as statement"));
    }

    tkr_parse(tkr,
        "{@ import \":tests/lang/modules/hello.cap\" as hello \n"
        "hello.world() @}"
    );
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nhello, world\n"));
    }

    tkr_parse(tkr,
        "{@ import \":tests/lang/modules/count.cap\" as count \n"
        "@}{: count.n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45"));
    }

    /************************
    * from import statement *
    ************************/

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import f1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import f1 \n f1() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nf1\n"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import ( f1, f2 ) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import ( f1, f2, ) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import (\nf1,\nf2,\n) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nf1\nf2\n"));
    }

    tkr_parse(tkr,
        "{@ from \n \":tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found path in compile from import statement"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" \n import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import in compile from import statement"));
    }

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import \n ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found import variables in compile from import statement"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_import_stmt_1(void) {
    trv_ready;

    tkr_parse(tkr,
        "{@ import \":tests/lang/modules/count.cap\" as count \n"
        "@}{: count.n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_2(void) {
    trv_ready;

    tkr_parse(tkr,
        "{@\n"
        "   if 1:\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45"));
    }

    tkr_parse(tkr,
        "{@\n"
        "   if 0:\n"
        "   else:\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45"));
    }

    tkr_parse(tkr,
        "{@\n"
        "   if 0:\n"
        "   elif 1:\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "   end"
        "@}{: count.n :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45"));
    }

    tkr_parse(tkr,
        "{@\n"
        "   for i = 0; i < 2; i += 1:\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "       puts(count.n)\n"
        "   end"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45\n45\n"));
    }

    tkr_parse(tkr,
        "{@\n"
        "   def func():\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "       puts(count.n)\n"
        "   end\n"
        "   func()\n"
        "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "45\n"));
    }

    tkr_parse(tkr,
        "{@\n"
        "   def func():\n"
        "       import \":tests/lang/modules/count.cap\" as count\n"
        "   end"
        "   func()\n"
        "@}{: count :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"count\" is not defined in ref block"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@ import \":tests/lang/modules/hello.cap\" as hello @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\n"));
    }

    trv_cleanup;
}

static void
test_trv_import_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr,
        "{@ import \":tests/lang/modules/hello.cap\" as hello \n"
        "hello.world() @}"
    );
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nhello, world\n"));
    }

    trv_cleanup;
}

static void
test_trv_from_import_stmt_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import f1 \n f1() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nf1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_from_import_stmt_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr,
        "{@ import \":tests/lang/modules/hello.cap\" as hello \n"
        "hello.world() @}"
    );
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nhello, world\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_from_import_stmt_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr,
        "{@ from \":tests/lang/modules/funcs.cap\" import ( f1, f2 ) \n "
        "   f1() \n f2() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        assert(!ast_has_errors(ast));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nf1\nf2\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1:\n puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: puts(1) \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1:\n\n puts(1) \n\nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if \n1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1\n: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ \n if 1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: puts(1) end \n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 1: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: \n@}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: \n\n@}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}1{@ \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}1{@ \n\nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 1: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: \nif 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: if 1: \nputs(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: if 1: puts(1) \nend end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 1: if 1: puts(1) end \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }
    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ \nif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: \n@}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: \n@}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}1{@ end @}{@ end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_4(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ i = 1 \n if i: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ i = 1 @}{@ if i: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   def f():\n"
    "       i = 1\n"
    "       if i:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   f()\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   i = 1\n"
    "   def f():\n"
    "       if i:\n"
    "           puts(1)\n"
    "       end\n"
    "   end\n"
    "   f()\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_5(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/if.cap\" as mod \n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_if_stmt_6(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "   from \"/tests/lang/modules/if-2.cap\" import f1 \n"
    "   f1()\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_7(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "   from \"/tests/lang/modules/if-2.cap\" import f2\n"
    "   f2()\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_8(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/if-3.cap\" as if3\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_9(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(n):\n"
    "       puts(n)\n"
    "       return n\n"
    "   end\n"
    "   if f(1):\n"
    "       puts(2)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n2\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   if 2 * 3 + 1:\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   i = 2\n"
    "   if i:\n"
    "       j = 3\n"
    "       if j:\n"
    "           puts(i * j)\n"
    "       end\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "24\n2000\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2000\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2000\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "20\n2000\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "200\n20\n2000\n"));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_10(void) {
    trv_ready;

    tkr_parse(tkr, "{@ \nif\n0\n:\nend\n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_if_stmt_11(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   if 1:\n"
    "       i = 0\n"
    "@}{: i :}{@"
    "       j = 1\n"
    "@}{: j :}{@\n"
    "       k = 2\n"
    "@}{: k :}{@\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "012"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ if 0: elif 1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: \nelif 1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1:\n puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: puts(1) \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: puts(1) end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif \n1: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1\n: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ \nif 0: @}{@ elif 1: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0:\n @}{@ elif 1: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ \nelif 1: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: \n@}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}1{@ \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}1{@ end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ \nif 0: @}{@ elif 1: @}1{@ end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ \nif 0: elif 1: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: \nelif 1: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1:\n if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: if 1:\n puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: if 1: puts(1)\n end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end\n end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: elif 1: if 1: puts(1) end end\n @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_elif_stmt_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ \nelif 1: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1:\n @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1:\n @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ elif 1: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_elif_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"i\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6\n"));
    }

    tkr_parse(tkr, "{@\n"
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
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1230\n"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"i\" is not defined"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_6(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "       puts(2)\n"
    "   elif 1:\n"
    "       puts(3)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "23\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "31\n"));
    }

    trv_cleanup;
}

static void
test_trv_elif_stmt_7(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   if 0:\n"
    "       puts(1)\n"
    "   elif 1:\n"
    "@}1{@\n"
    "@}2{@\n"
    "@}3{@\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123456"));
    }

    trv_cleanup;
}

static void
test_trv_else_stmt_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 0: else: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ \nif 0: else: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: \nelse: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else:\n puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: puts(1) \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: puts(1) end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else\n: puts(1) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_else_stmt_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ \nif 0: @}{@ else: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0:\n @}{@ else: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ \nelse: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else\n: @}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: \n@}1{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}1{@ \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}1{@ end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_else_stmt_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ if 0: else: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ \nif 0: else: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: \nelse: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else\n: if 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: \nif 1: puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: if 1:\n puts(1) end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: if 1: puts(1)\n end end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: if 1: puts(1) end \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ if 0: else: if 1: puts(1) end end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_else_stmt_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ \nif 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: \n@}{@ else: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ \nelse: @}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: \n@}{@ if 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ \nif 1: @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1:\n @}1{@ end @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ \nend @}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end \n@}{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ if 0: @}{@ else: @}{@ if 1: @}1{@ end @}{@ end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_else_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@"
    "   if 0:\n"
    "   else:\n"
    "@}1{@\n"
    "@}2{@\n"
    "@}3{@\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "123"));
    }

    trv_cleanup;
}
static void
test_trv_for_stmt_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ for i=0; i<2; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ size=0 for i=size; i<2; i += 1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ \nfor i=0; i<2; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2; i +=1: \nputs(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2; i +=1: puts(i)\n end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2; i +=1: puts(i) end \n@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for \ni=0; i<2; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0\n; i<2; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; \ni<2; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2\n; i +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2; \ni +=1: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ for i=0; i<2; i +=1\n: puts(i) end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_for_stmt_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ i=0 for i<2: puts(i)\ni+=1 end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ i=0 for i<2: \nputs(i)\ni+=1 end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ i=0 for i<2: puts(i)\ni+=1 \nend @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ i=0 for \ni<2: puts(i)\ni+=1 end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    tkr_parse(tkr, "{@ i=0 for i<2\n: puts(i)\ni+=1 end @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_for_stmt_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ for i, j = 0, 0; i != 4; i += 1, j += 2: end @}{: i :},{: j :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4,8"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_for_stmt_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ for i = 0; i < 2; i += 1: @}{: i :},{@ end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,1,"));
    }

    tkr_parse(tkr, "{@\n"
    "def func():\n"
    "   for i = 0; i < 2; i += 1: @}"
    "{: i :}\n"
    "{@ end \n"
    "end \n"
    "\n"
    " func() @}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }
    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_for_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def hiphop(rap, n):\n"
    "       puts(rap * n)\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\nyo\nyoyo\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       def hiphop(rap, n):\n"
    "           puts(rap * n)\n"
    "       end\n"
    "\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\nyo\nyoyo\n"));
    }

    tkr_parse(tkr, "{@\n"
    "   def hiphop(rap, n):\n"
    "       for i = n-1; i >= 0; i -= 1:\n"
    "           puts(rap * i)\n"
    "       end\n"
    "   end\n"
    "\n"
    "   hiphop(\"yo\", 3)"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "yoyo\nyo\n\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 3; i += 1:\n"
    "@}{: i :}{@\n"
    "   end\n"
    "\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "012"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_6(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "    i = 0\n"
    "    j = i\n"
    "    a = [j, j+1, j+2]\n"
    "@}{: a[0] :},{: a[1] :},{: a[2] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,1,2"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_7(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "for i = 0; i < 4; i += 1:\n"
    "   j = i\n"
    "end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_8(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "for i = 0; i < 2; i += 1:\n"
    "   j = i\n"
    "   k = i\n"
    "   puts(j, k)\n"
    "end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 0\n1 1\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_9(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "for i = 0; i < 2; i += 1:\n"
    "   j = i\n"
    "   k = i\n"
    "@}{: i :}{@"
    "   l = i\n"
    "   m = i\n"
    "end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "01"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "000111"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_10(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "for \n i = 0 \n ; \n i < 2 \n ; \n i += 1 \n : \n"
    "end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@\n"
    "i = 0\n"
    "for \n i < 2 \n : \n"
    "   puts(i)\n"
    "   i += 1\n"
    "end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_for_stmt_11(void) {
}

static void
test_trv_for_stmt_12(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def hiphop(rap, n):\n"
    "       puts(rap * n)\n"
    "   end\n"
    "\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       hiphop(\"yo\", i)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "\nyo\nyoyo\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ for: break end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ for:\n break end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ for: break \nend @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       break\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n4\n10\n1\n4\n10\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(i)\n"
    "       if 1:\n"
    "           break\n"
    "       end\n"
    "       puts(10)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n"));
    }

    trv_cleanup;
}

static void
test_trv_break_stmt_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   break\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid break statement. not in loop"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid break statement. not in loop"));
    }

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       def f():\n"
    "           break\n"
    "       end\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid break statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ j=0 for i=0; i<2; i+=1: continue\n j=i end @}{: j :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       continue\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       puts(0)\n"
    "       if 1:\n"
    "           continue\n"
    "       end\n"
    "       puts(1)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\n"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n10\n10\n1\n0\n10\n10\n1\n"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid continue statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_continue_stmt_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   continue\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid continue statement. not in loop"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid continue statement. not in loop"));
    }

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       def f():\n"
    "           continue\n"
    "       end\n"
    "       puts(i)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid continue statement. not in loop"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): return 1 end @}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_return_stmt_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   def f():\n"
    "       puts(1)\n"
    "       return 2\n"
    "       puts(3)\n"
    "   end\n"
    "@}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_return_stmt_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f():\n"
    "       for i = 0; i < 2; i += 1:\n"
    "           puts(i)\n"
    "           return 1\n"
    "           puts(i)\n"
    "       end\n"
    "   end\n"
    "@}{: f() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\n1"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n2"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n2"));
    }

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n1\n2"));
    }

    trv_cleanup;
}

static void
test_trv_return_stmt_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   return\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid return statement. not in function"));
    }

    tkr_parse(tkr, "{@\n"
    "   if 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid return statement. not in function"));
    }

    tkr_parse(tkr, "{@\n"
    "   if 0:\n"
    "   else:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid return statement. not in function"));
    }

    tkr_parse(tkr, "{@\n"
    "   if 0:\n"
    "   elif 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid return statement. not in function"));
    }

    tkr_parse(tkr, "{@\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       return\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid return statement. not in function"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): end @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(a, b): puts(a, b) end f(1, 2) @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "@}{: c :}");
    {
        ast_clear(ast);
        (cc_compile(ast, tkr_get_tokens(tkr)));
        (trv_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): end \n a = not f @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_4(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ i = 1 \n def f(): puts(i) end \n f() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ast->ref_context), "1\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_5(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(arg): end \n f() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "arguments not same length"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_6(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@\n"
    "   def f(n, desc):\n"
    "       c = true\n"
    "       indent = n * \"    \""
    "@}{: indent :}abc{@"
    "   end\n"
    "   f(1, \"desc\")\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "    abc"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_7(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "from \"/tests/lang/modules/func-def.cap\" import draw\n"
    "draw(1, \"desc\")\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "    program\n\n    comment\n"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_func_def_8(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   f([1, 2, 3])\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2 3\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_9(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   i = 0\n"
    "   f([i, i+1, i+2])\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_func_def_10(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(arr):\n"
    "       puts(arr[0], arr[1], arr[2])\n"
    "   end\n"
    "   for i = 0; i < 3; i += 1:\n"
    "       f([i, i+1, i+2])\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 1 2\n1 2 3\n2 3 4\n"));
    }

    trv_cleanup;
}

static void
test_trv_assign_list_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_list_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = 1, b = 2 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_list_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = b = 1 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_assign_list_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = b = 1, c = 2 @}{: a :},{: b :},{: c :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,1,2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_multi_assign_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a, b = 1, 2 @}{: a :},{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_or_test_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 or 0 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_and_test_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 and 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_not_test_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: not 0 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 == 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 != 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 < 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 2 > 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_4(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 <= 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_comparison_5(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 2 >= 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_asscalc_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 1 \n a += b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@ 0 += 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (1)"));
    }

    tkr_parse(tkr, "{@ true += 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (2)"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += \"b\" @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right hand operand (4)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_asscalc_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@ a = 0 \n a -= 1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "-1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 1 \n a -= b @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "-1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a -= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "-1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a -= false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@ 1 -= 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand type (1)"));
    }

    tkr_parse(tkr, "{@ true -= 1 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand type (2)"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a -= \"c\" @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right hand operand type (4)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_asscalc_2(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@ a = 2 \n a *= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{@ a = 2 @}{: (a *= 2) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{@ a = 2 \n a *= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 2 \n a *= false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n a *= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abab"));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n a *= 0 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n a *= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "ab"));
    }

    tkr_parse(tkr, "{@ a = \"ab\" \n a *= false @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@ a = \"ab\" \n a *= -1 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't mul by negative value"));
    }

    tkr_parse(tkr, "{@ 1 *= 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (1)"));
    }

    tkr_parse(tkr, "{@ true *= 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (2)"));
    }

    tkr_parse(tkr, "{@ a = 2 \n a *= \"b\" @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right hand operand (4)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_asscalc_3(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@ a = 4 \n a /= 2 @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 4  @}{: (a /= 2) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 4 \n a /= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{@ a = true \n a /= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = false \n a /= true @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@ 4 /= 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (1)"));
    }

    tkr_parse(tkr, "{@ true /= 2 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (2)"));
    }

    tkr_parse(tkr, "{@ a = 4 \n a /= false @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    tkr_parse(tkr, "{@ a = 4 \n a /= 0 @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    tkr_parse(tkr, "{@ a = 4 \n a /= \"b\" @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right hand operand (4)"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_asscalc_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] += 1\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_5(void) {
    trv_ready;

    return;  // TODO test

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] += a[0] += 1\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_6(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [\"aaa\", 2]\n"
    "   a[0] += \"bbb\"\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "aaabbb"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_7(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] += 1\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_8(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] += true\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_9(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] -= 1\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_10(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] -= true\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_11(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_12(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [\"abc\", 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        showbuf();
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcabc"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_13(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] *= 2\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_14(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [true, 2]\n"
    "   a[0] *= true\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_15(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= 2\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_16(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= 0\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_17(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [4, 2]\n"
    "   a[0] /= false\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_18(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [1, 2]\n"
    "   a[0] /= true\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_19(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= 2\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_20(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= true\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_21(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= 0\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_asscalc_22(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [2, 2]\n"
    "   a[0] %= false\n"
    "@}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "zero division error"));
    }

    trv_cleanup;
}

static void
test_trv_expr_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 + 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 1 b = a @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_expr_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 - 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_expr_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@ a = 1 \n b = a - 1 @}{: b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = f(a)\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = f(a)[0]\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4a(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [f, 2, 3]\n"
    "r = a[0](1)\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

/*

    a.b[0].c(0).d[0][0](0)

*/

static void
test_trv_expr_4b(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "import \"/tests/lang/modules/func.cap\" as mod\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [mod, 2, 3]\n"
    "r = a[0].arrMod.array[0]\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_4c(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "import \"/tests/lang/modules/func.cap\" as mod\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "a = [mod, 2, 3]\n"
    "r = a[0].arrMod.funcArray[0](0)\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "d = { \"a\": 1, \"b\": 2 }\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = a[f(a)[0]]\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@\n"
    "a = [1, 2, 3]\n"
    "d = { \"a\": 1, \"b\": 2 }\n"
    "def f(arg):\n"
    "   return arg\n"
    "end\n"
    "r = a[f(a)[0] * 2] * 3 + f(10)\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "19"));
    }

    trv_cleanup;
}

static void
test_trv_expr_6(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "import \"/tests/lang/modules/array.cap\" as mod\n"
    "\n"
    "r = mod.array[0]\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_expr_7(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "import \"/tests/lang/modules/func.cap\" as mod\n"
    "\n"
    "r = mod.func(1)\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_expr_8(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    tkr_parse(tkr, "{@\n"
    "import \"/tests/lang/modules/func.cap\" as mod\n"
    "\n"
    "r = mod.arrMod.array[0]\n"
    "@}{: r :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_term_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 2 * 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "4"));
    }

    tkr_parse(tkr, "{: 2 * \"abc\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcabc"));
    }

    tkr_parse(tkr, "{: \"abc\" * 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abcabc"));
    }

    tkr_parse(tkr, "{: 0 * \"abc\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    tkr_parse(tkr, "{: -1 * \"abc\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "can't mul string by negative value"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_term_1(void) {
    trv_ready;

    tkr_parse(tkr, "{: 4 / 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }
/*
    tkr_parse(tkr, "{: 3 / 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));  // TODO: imp float!
    }
*/
    trv_cleanup;
}

static void
test_trv_term_2(void) {
    trv_ready;

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{: 4 % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: 3 % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{: 4 % nil :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid right hand operand (0)"));
    }

    tkr_parse(tkr, "{: nil % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid left hand operand (0)"));
    }

    trv_cleanup;
}

static void
test_trv_term_3(void) {
    trv_ready;

    tkr_parse(tkr, "{: 2 * 2 / 4 % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 4 / 2 * 2 % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: 3 % 2 * 3 / 3 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 3 * 2 / 3 * 3 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "6"));
    }

    tkr_parse(tkr, "{: 4 / 2 * 2 / 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: 3 % 2 * 2 % 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    tkr_parse(tkr, "{: 3 * 2 % 2 * 2 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_call_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ def f(): end f() @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_call_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       puts(a[0], a[1])\n"
    "   end\n"
    "   a = [1, 2]\n"
    "   f(a)\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       puts(a[0], a[1])\n"
    "   end\n"
    "   for i = 0; i < 2; i += 1:\n"
    "       a = [i, i+1]\n"
    "       f(a)\n"
    "   end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 1\n1 2\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 1 2\n1 2 3\n"));
    }

    trv_cleanup;
}

static void
test_trv_call_4(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "    from \"/tests/lang/modules/list.cap\" import arrayToUl\n"
    "\n"
    "    for i = 0; i < 4; i += 1:\n"
    "       j = i\n"
    "       a = [j, j+1, j+2]\n"
    "       arrayToUl(a)\n"
    "    end\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx),
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

    tkr_parse(tkr, "{@\n"
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
    "@}{: f4(2) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2\n4"));
    }

    trv_cleanup;
}

static void
test_trv_index_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = [0, 1] @}{: a[0] :},{: a[1] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0,1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_index_1(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ a = [0, 1] @}{: a[0] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_array_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ a = [0, 1] @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_array_1(void) {

    return;  // TODO

    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   i = 0\n"
    "   a = [i, 1]\n"
    "   a[0] += 1\n"
    "   puts(i)\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0"));
    }

    trv_cleanup;
}

static void
test_trv_array_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   i = 0\n"
    "   a = [i, 1]\n"
    "   puts(i)\n"
    "   puts(a[0])\n"
    "   puts(id(i) != id(a[0]))"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0\n0\ntrue\n"));
    }

    trv_cleanup;
}

static void
test_trv_array_3(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   i = 0\n"
    "   s = \"abc\"\n"
    "   n = nil\n"
    "   l = [0, 1, 2]\n"
    "   d = {\"a\": 1, \"b\": 2}\n"
    "   a = [i, s, n, l, d]\n"
    "   puts(a[0], a[1], a[2], a[3][0], a[4][\"a\"])\n"
    "   puts(id(i) != id(a[0]))\n"
    "   puts(id(s) != id(a[1]))\n"
    "   puts(id(n) != id(a[2]))\n"
    "   puts(id(l) == id(a[3]))\n"
    "   puts(id(d) == id(a[4]))\n"
    "   l[0] = 3\n"
    "   puts(l[0] == a[3][0])\n"
    "   d[\"a\"] = 3\n"
    "   puts(d[\"a\"] == a[4][\"a\"])\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "0 abc nil 0 1\ntrue\ntrue\ntrue\ntrue\ntrue\ntrue\ntrue\n"));
    }

    trv_cleanup;
}

static void
test_trv_array_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   a = [\n"
    "       1,\n"
    "       2, 3,\n"
    "       4,\n"
    "   ]\n"
    "@}{: a[0] :}{: a[1] :}{: a[2] :}{: a[3] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1234"));
    }

    trv_cleanup;
}

static void
test_trv_nil(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: nil :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_false(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: false :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "false"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_true(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: true :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_digit(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_string(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{: \"abc\" :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "abc"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_dict_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@ d = {\"a\":1, \"b\":2} @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), ""));
    }

    trv_cleanup;
}

static void
test_trv_dict_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   d = {\n"
    "       \"a\" \n : \n 1 \n, \n"
    "       \"b\" \n : \n 2 \n, \n"
    "   }\n"
    "@}{: d[\"a\"] :}{: d[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "12"));
    }

    trv_cleanup;
}

static void
test_trv_dict_2(void) {
    trv_ready;

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@\n"
    "   d = {\"a\": 1}\n"
    "@}{: d[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "not found key \"b\""));
    }

    trv_cleanup;
}

static void
test_trv_dict_3(void) {
    trv_ready;

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@ a = { 1: 1 } @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "key is not string in dict elem"));
    }

    tkr_parse(tkr, "{@ a = { \"k\": 1 } \n a[0] @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "index isn't string"));
    }

    tkr_parse(tkr, "{@ k = 1 \n a = { k: 1 } @}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "invalid key type in variable of dict"));
    }

    // success

    tkr_parse(tkr, "{@ a = { \"key\": 1 } @}{: a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(dict)"));
    }

    tkr_parse(tkr, "{@ a = { \"key\": 1 } @}{: a[\"key\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = { \"key\": \"val\" } @}{: a[\"key\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "val"));
    }

    tkr_parse(tkr, "{@ a = { \"key\": [1, 2] } @}{: a[\"key\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(array)"));
    }

    // tkr_parse(tkr, "{@ a = { \"key\": 1 }[\"key\"] @}{: a :}");
    // {
    ast_clear(ast);
    //     cc_compile(ast, tkr_get_tokens(tkr));
    //     ctx_clear(ctx);
    //     trv_traverse(ast, ctx);
    //     assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    // }

    tkr_parse(tkr, "{@ a = { \"k1\": 1, \"k2\": 2 } @}{: a[\"k1\"] :},{: a[\"k2\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = { \"k1\": { \"k2\": 1 } } @}{: a[\"k1\"][\"k2\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ k = \"key\" \n a = { k: 1 } @}{: a[k] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    trv_cleanup;
}

static void
test_trv_identifier(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ i = 1 @}{: i :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_builtin_array_0(void) {
    config_t *config = config_new();
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(mem_move(opt));
    ast_t *ast = ast_new(config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);

    tkr_parse(tkr, "{@ arr = [1, 2] \n arr.push(3) @}{: len(arr) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{: len([1, 2].push(3)) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = [1, 2] @}{: a.pop() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = [] @}{: a.pop() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{: [1, 2].pop() :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    ctx_del(ctx);
    gc_del(gc);
    ast_del(ast);
    tkr_del(tkr);
    config_del(config);
}

static void
test_trv_module_0(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/module.cap\" as mod\n"
    "   puts(\"done\")\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "imported\nimported module.cap\ndone\n"));
    }

    trv_cleanup;
}

static void
test_trv_chain_object(void) {
    trv_ready;

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));

    /*****
    * ok *
    *****/

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a = 1\n"
    "@}{: string.a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1"));
    }

    /*******
    * fail *
    *******/

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "@}{: string.a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}{: string.a :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "   string.a.b = 1\n"
    "@}{: string.a.b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    tkr_parse(tkr, "{@\n"
    "   import \"/tests/lang/modules/string.cap\" as string\n"
    "@}{: string.a.b :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        (trv_traverse(ast, ctx));
        assert(ast_has_errors(ast));
        assert(!strcmp(ast_getc_first_error_message(ast), "\"a\" is not defined"));
    }

    trv_cleanup;
}

static void
test_trv_etc_0(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "***i love life***\n*\n*\n*\ni\n \nl\no\nv\ne\n \nl\ni\nf\ne\n*\n*\n*\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_1(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "1\n2\n,\n3\n4\n,\n"));
    }

    trv_cleanup;
}

static void
test_trv_etc_2(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
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
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "<html>\n"
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

    tkr_parse(tkr, "{@\n"
    "   d = {\"a\": 1}"
    "   a = [d, 2]\n"
    "   a[0][\"a\"] += 1\n"
    "@}{: a[0][\"a\"] :},{: d[\"a\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2,2"));
    }

    trv_cleanup;
}

static void
test_trv_etc_4(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(a):\n"
    "       return a\n"
    "   end\n"
    "   a = f(f)\n"
    "@}{: id(a) == id(f) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"a\"]\n"
    "   end\n"
    "   d = {\"a\": f}\n"
    "   a = d[\"a\"](d)"
    "@}{: a :},{: id(a) == id(f) :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "(function),true"));
    }

    trv_cleanup;
}

static void
test_trv_etc_5(void) {
    trv_ready;

    tkr_parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"a\"] + arg[\"b\"]\n"
    "   end\n"
    "   a = 1, b = 2\n"
    "   c = f({ \"a\" : a, \"b\": b })"
    "@}{: c :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       return arg[\"d\"][\"a\"] + arg[\"c\"][\"b\"]\n"
    "   end\n"
    "   d = { \"a\": 1, \"b\": 2 }\n"
    "   c = f({ \"d\" : d, \"c\": d })\n"
    "@}{: c :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       arg[\"d\"][\"a\"] += 1\n"
    "       arg[\"c\"][\"b\"] += 1\n"
    "   end\n"
    "   d = { \"a\": 1, \"b\": 2 }\n"
    "   c = f({ \"d\" : d, \"c\": d })\n"
    "@}{: d[\"a\"] :},{: d[\"b\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2,3"));
    }

    tkr_parse(tkr, "{@\n"
    "   def f(arg):\n"
    "       e = arg[\"d\"]\n"
    "       e[\"a\"] += 1\n"
    "   end\n"
    "   d = { \"a\": 1 }\n"
    "   c = f({ \"d\" : d })\n"
    "@}{: d[\"a\"] :}");
    {
        ast_clear(ast);
        cc_compile(ast, tkr_get_tokens(tkr));
        ctx_clear(ctx);
        trv_traverse(ast, ctx);
        assert(!ast_has_errors(ast));
        assert(!strcmp(ctx_getc_stdout_buf(ctx), "2"));
    }

    trv_cleanup;
}

static const struct testcase
traverser_tests[] = {
    {"trv_assign_and_reference_0", test_trv_assign_and_reference_0},
    {"trv_assign_and_reference_1", test_trv_assign_and_reference_1},
    {"trv_assign_and_reference_2", test_trv_assign_and_reference_2},
    {"trv_assign_and_reference_3", test_trv_assign_and_reference_3},
    {"trv_assign_and_reference_4", test_trv_assign_and_reference_4},
    {"trv_assign_and_reference_5", test_trv_assign_and_reference_5},
    {"trv_assign_and_reference_6", test_trv_assign_and_reference_6},
    {"trv_assign_and_reference_7", test_trv_assign_and_reference_7},
    {"trv_assign_and_reference_8", test_trv_assign_and_reference_8},
    {"trv_assign_and_reference_9", test_trv_assign_and_reference_9},
    {"trv_assign_and_reference_10", test_trv_assign_and_reference_10},
    {"trv_assign_and_reference_11", test_trv_assign_and_reference_11},
    {"trv_assign_and_reference_12", test_trv_assign_and_reference_12},
    {"trv_assign_and_reference_13", test_trv_assign_and_reference_13},
    {"trv_assign_and_reference_14", test_trv_assign_and_reference_14},
    {"trv_assign_and_reference_15", test_trv_assign_and_reference_15},
    {"trv_assign_and_reference_16", test_trv_assign_and_reference_16},
    {"trv_assign_and_reference_all", test_trv_assign_and_reference_all},
    {"trv_code_block", test_trv_code_block},
    {"trv_ref_block", test_trv_ref_block},
    {"trv_text_block", test_trv_text_block},
    {"trv_import_stmt_0", test_trv_import_stmt_0},
    {"trv_import_stmt_1", test_trv_import_stmt_1},
    {"trv_import_stmt_2", test_trv_import_stmt_2},
    {"trv_import_stmt_3", test_trv_import_stmt_3},
    {"trv_import_stmt_4", test_trv_import_stmt_4},
    {"trv_from_import_stmt_1", test_trv_from_import_stmt_1},
    {"trv_from_import_stmt_2", test_trv_from_import_stmt_2},
    {"trv_from_import_stmt_3", test_trv_from_import_stmt_3},
    {"trv_if_stmt_0", test_trv_if_stmt_0},
    {"trv_if_stmt_1", test_trv_if_stmt_1},
    {"trv_if_stmt_2", test_trv_if_stmt_2},
    {"trv_if_stmt_3", test_trv_if_stmt_3},
    {"trv_if_stmt_4", test_trv_if_stmt_4},
    {"trv_if_stmt_5", test_trv_if_stmt_5},
    {"trv_if_stmt_6", test_trv_if_stmt_6},
    {"trv_if_stmt_7", test_trv_if_stmt_7},
    {"trv_if_stmt_8", test_trv_if_stmt_8},
    {"trv_if_stmt_9", test_trv_if_stmt_9},
    {"trv_if_stmt_10", test_trv_if_stmt_10},
    {"trv_if_stmt_11", test_trv_if_stmt_11},
    {"trv_elif_stmt_0", test_trv_elif_stmt_0},
    {"trv_elif_stmt_1", test_trv_elif_stmt_1},
    {"trv_elif_stmt_2", test_trv_elif_stmt_2},
    {"trv_elif_stmt_3", test_trv_elif_stmt_3},
    {"trv_elif_stmt_4", test_trv_elif_stmt_4},
    {"trv_elif_stmt_5", test_trv_elif_stmt_5},
    {"trv_elif_stmt_6", test_trv_elif_stmt_6},
    {"trv_elif_stmt_7", test_trv_elif_stmt_7},

    {"trv_else_stmt_0", test_trv_else_stmt_0},
    {"trv_else_stmt_1", test_trv_else_stmt_1},
    {"trv_else_stmt_2", test_trv_else_stmt_2},
    {"trv_else_stmt_3", test_trv_else_stmt_3},
    {"trv_else_stmt_4", test_trv_else_stmt_4},

    {"trv_for_stmt_0", test_trv_for_stmt_0},
    {"trv_for_stmt_1", test_trv_for_stmt_1},
    {"trv_for_stmt_2", test_trv_for_stmt_2},
    {"trv_for_stmt_3", test_trv_for_stmt_3},
    {"trv_for_stmt_4", test_trv_for_stmt_4},
    {"trv_for_stmt_5", test_trv_for_stmt_5},
    {"trv_for_stmt_6", test_trv_for_stmt_6},
    {"trv_for_stmt_7", test_trv_for_stmt_7},
    {"trv_for_stmt_8", test_trv_for_stmt_8},
    {"trv_for_stmt_9", test_trv_for_stmt_9},
    {"trv_for_stmt_10", test_trv_for_stmt_10},
    {"trv_for_stmt_11", test_trv_for_stmt_11},
    {"trv_for_stmt_12", test_trv_for_stmt_12},
    {"trv_break_stmt_0", test_trv_break_stmt_0},
    {"trv_break_stmt_1", test_trv_break_stmt_1},
    {"trv_break_stmt_2", test_trv_break_stmt_2},
    {"trv_break_stmt_3", test_trv_break_stmt_3},
    {"trv_continue_stmt_0", test_trv_continue_stmt_0},
    {"trv_continue_stmt_1", test_trv_continue_stmt_1},
    {"trv_continue_stmt_2", test_trv_continue_stmt_2},
    {"trv_continue_stmt_3", test_trv_continue_stmt_3},
    {"trv_continue_stmt_4", test_trv_continue_stmt_4},
    {"trv_continue_stmt_5", test_trv_continue_stmt_5},
    {"trv_return_stmt_0", test_trv_return_stmt_0},
    {"trv_return_stmt_1", test_trv_return_stmt_1},
    {"trv_return_stmt_2", test_trv_return_stmt_2},
    {"trv_return_stmt_3", test_trv_return_stmt_3},
    {"trv_return_stmt_4", test_trv_return_stmt_4},
    {"trv_func_def_0", test_trv_func_def_0},
    {"trv_func_def_1", test_trv_func_def_1},
    {"trv_func_def_2", test_trv_func_def_2},
    {"trv_func_def_3", test_trv_func_def_3},
    {"trv_func_def_4", test_trv_func_def_4},
    {"trv_func_def_5", test_trv_func_def_5},
    {"trv_func_def_6", test_trv_func_def_6},
    {"trv_func_def_7", test_trv_func_def_7},
    {"trv_func_def_8", test_trv_func_def_8},
    {"trv_func_def_9", test_trv_func_def_9},
    {"trv_func_def_10", test_trv_func_def_10},
    {"trv_assign_list_0", test_trv_assign_list_0},
    {"trv_assign_list_1", test_trv_assign_list_1},
    {"trv_assign_list_2", test_trv_assign_list_2},
    {"trv_assign_list_3", test_trv_assign_list_3},
    {"trv_multi_assign_0", test_trv_multi_assign_0},
    {"trv_or_test_0", test_trv_or_test_0},
    {"trv_and_test_0", test_trv_and_test_0},
    {"trv_not_test_0", test_trv_not_test_0},
    {"trv_comparison_0", test_trv_comparison_0},
    {"trv_comparison_1", test_trv_comparison_1},
    {"trv_comparison_2", test_trv_comparison_2},
    {"trv_comparison_3", test_trv_comparison_3},
    {"trv_comparison_4", test_trv_comparison_4},
    {"trv_comparison_5", test_trv_comparison_5},
    {"trv_asscalc_0", test_trv_asscalc_0},
    {"trv_asscalc_1", test_trv_asscalc_1},
    {"trv_asscalc_2", test_trv_asscalc_2},
    {"trv_asscalc_3", test_trv_asscalc_3},
    {"trv_asscalc_4", test_trv_asscalc_4},
    {"trv_asscalc_5", test_trv_asscalc_5},
    {"trv_asscalc_6", test_trv_asscalc_6},
    {"trv_asscalc_7", test_trv_asscalc_7},
    {"trv_asscalc_8", test_trv_asscalc_8},
    {"trv_asscalc_9", test_trv_asscalc_9},
    {"trv_asscalc_10", test_trv_asscalc_10},
    {"trv_asscalc_11", test_trv_asscalc_11},
    {"trv_asscalc_12", test_trv_asscalc_12},
    {"trv_asscalc_13", test_trv_asscalc_13},
    {"trv_asscalc_14", test_trv_asscalc_14},
    {"trv_asscalc_15", test_trv_asscalc_15},
    {"trv_asscalc_16", test_trv_asscalc_16},
    {"trv_asscalc_17", test_trv_asscalc_17},
    {"trv_asscalc_18", test_trv_asscalc_18},
    {"trv_asscalc_19", test_trv_asscalc_19},
    {"trv_asscalc_20", test_trv_asscalc_20},
    {"trv_asscalc_21", test_trv_asscalc_21},
    {"trv_asscalc_22", test_trv_asscalc_22},
    {"trv_expr_0", test_trv_expr_0},
    {"trv_expr_1", test_trv_expr_1},
    {"trv_expr_2", test_trv_expr_2},
    {"trv_expr_3", test_trv_expr_3},
    {"trv_expr_4", test_trv_expr_4},
    {"trv_expr_4a", test_trv_expr_4a},
    {"trv_expr_5", test_trv_expr_5},
    {"trv_expr_6", test_trv_expr_6},
    {"trv_expr_7", test_trv_expr_7},
    {"trv_expr_8", test_trv_expr_8},
    {"trv_term_0", test_trv_term_0},
    {"trv_term_1", test_trv_term_1},
    {"trv_term_2", test_trv_term_2},
    {"trv_term_3", test_trv_term_3},
    {"trv_call_0", test_trv_call_0},
    {"trv_call_1", test_trv_call_1},
    {"trv_call_2", test_trv_call_2},
    {"trv_call_3", test_trv_call_3},
    {"trv_call_4", test_trv_call_4},
    {"trv_call_5", test_trv_call_5},
    {"trv_index_0", test_trv_index_0},
    {"trv_index_1", test_trv_index_1},
    {"trv_array_0", test_trv_array_0},
    {"trv_array_1", test_trv_array_1},
    {"trv_array_2", test_trv_array_2},
    {"trv_array_3", test_trv_array_3},
    {"trv_array_4", test_trv_array_4},
    {"trv_nil", test_trv_nil},
    {"trv_false", test_trv_false},
    {"trv_true", test_trv_true},
    {"trv_digit", test_trv_digit},
    {"trv_string", test_trv_string},
    {"trv_dict_0", test_trv_dict_0},
    {"trv_dict_1", test_trv_dict_1},
    {"trv_dict_2", test_trv_dict_2},
    {"trv_dict_3", test_trv_dict_3},
    {"trv_identifier", test_trv_identifier},
    {"trv_traverse", test_trv_traverse},
    {"trv_comparison", test_trv_comparison},
    {"trv_array_index", test_trv_array_index},
    {"trv_text_block_old", test_trv_text_block_old},
    {"trv_ref_block_old", test_trv_ref_block_old},
    {"trv_assign_0", test_trv_assign_0},
    {"trv_assign_1", test_trv_assign_1},
    {"trv_assign_2", test_trv_assign_2},
    {"trv_atom_0", test_trv_atom_0},
    {"trv_array", test_trv_array},
    {"trv_index", test_trv_index},
    {"trv_string_index", test_trv_string_index},
    {"trv_multi_assign", test_trv_multi_assign},
    {"trv_and_test", test_trv_and_test},
    {"trv_assign_list", test_trv_assign_list},
    {"trv_test_list", test_trv_test_list},
    {"trv_dot_0", test_trv_dot_0},
    {"trv_dot_1", test_trv_dot_1},
    {"trv_dot_2", test_trv_dot_2},
    {"trv_dot_3", test_trv_dot_3},
    {"trv_dot_4", test_trv_dot_4},
    {"trv_dot_5", test_trv_dot_5},
    {"trv_negative_0", test_trv_negative_0},
    {"trv_call", test_trv_call},
    {"trv_func_def", test_trv_func_def},
    {"trv_builtin_modules_opts_0", test_trv_builtin_modules_opts_0},
    {"trv_builtin_modules_alias_0", test_trv_builtin_modules_alias_0},
    {"trv_builtin_modules_alias_1", test_trv_builtin_modules_alias_1},
    {"trv_builtin_modules_alias_2", test_trv_builtin_modules_alias_2},
    {"trv_builtin_modules_array_0", test_trv_builtin_modules_array_0},
    {"trv_builtin_functions", test_trv_builtin_functions},
    {"trv_builtin_functions_puts_0", test_trv_builtin_functions_puts_0},
    {"trv_builtin_functions_len_0", test_trv_builtin_functions_len_0},
    {"trv_builtin_functions_type", test_trv_builtin_functions_type},
    {"trv_builtin_functions_type_dict", test_trv_builtin_functions_type_dict},
    {"trv_builtin_string", test_trv_builtin_string},
    {"trv_builtin_array_0", test_trv_builtin_array_0},
    {"trv_module_0", test_trv_module_0},
    {"trv_chain_object", test_trv_chain_object},
    {"trv_etc_0", test_trv_etc_0},
    {"trv_etc_1", test_trv_etc_1},
    {"trv_etc_2", test_trv_etc_2},
    {"trv_etc_3", test_trv_etc_3},
    {"trv_etc_4", test_trv_etc_4},
    {"trv_etc_5", test_trv_etc_5},
    {0},
};

/**********
* symlink *
**********/

static void
test_symlink_norm_path(void) {
    config_t * config = config_new();

    char path[FILE_NPATH];
    assert(symlink_norm_path(NULL, NULL, 0, NULL) == NULL);
    assert(symlink_norm_path(config, NULL, 0, NULL) == NULL);
    assert(symlink_norm_path(config, path, 0, NULL) == NULL);
    assert(symlink_norm_path(config, path, sizeof path, NULL) == NULL);

#ifdef _CAP_WINDOWS
    assert(symlink_norm_path(config, path, sizeof path, "C:\\path\\to\\dir") == path);
    assert(strcmp(path, "C:\\path\\to\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "\\path\\to\\dir") == path);
    assert(strcmp(path, "\\path\\to\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "\\path\\..\\to\\dir") == path);
    assert(strcmp(path, "\\to\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "\\path\\..\\to\\..\\dir") == path);
    assert(strcmp(path, "\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "\\path\\to\\..\\..\\dir") == path);
    assert(strcmp(path, "\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "C:\\path\\to\\dir\\") == path);
    assert(strcmp(path, "C:\\path\\to\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "C:\\path\\..\\to\\dir") == path);
    assert(strcmp(path, "C:\\to\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "C:\\path\\..\\to\\..\\dir") == path);
    assert(strcmp(path, "C:\\dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "C:\\path\\to\\..\\..\\dir") == path);
    assert(strcmp(path, "C:\\dir") == 0);

#else
    assert(symlink_norm_path(config, path, sizeof path, "/path/to/dir") == path);
    assert(strcmp(path, "/path/to/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "/path/to/dir/") == path);
    assert(strcmp(path, "/path/to/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "path/to/dir") == path);
    assert(strcmp(path, "path/to/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "path/../to/dir") == path);
    assert(strcmp(path, "to/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "path/../to/../dir") == path);
    assert(strcmp(path, "dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "path/to/../../dir") == path);
    assert(strcmp(path, "dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "/path/../to/dir") == path);
    assert(strcmp(path, "/to/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "/path/../to/../dir") == path);
    assert(strcmp(path, "/dir") == 0);

    assert(symlink_norm_path(config, path, sizeof path, "/path/to/../../dir") == path);
    assert(strcmp(path, "/dir") == 0);
#endif

    config_del(config);
}

static const struct testcase
symlink_tests[] = {
    {"symlink_norm_path", test_symlink_norm_path},
    {0},
};

/**************
* error_stack *
**************/

static void
test_errstack_new(void) {
    errstack_t *stack = errstack_new();
    assert(stack);
    errstack_del(stack);
}

static void
test_errstack_pushb(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_len(stack) == 0);
    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_len(stack) == 2);

    const errelem_t *elem = errstack_getc(stack, 0);
    assert(elem);
    assert(!strcmp(elem->filename, "file1"));
    assert(elem->lineno == 1);
    assert(!strcmp(elem->funcname, "func1"));
    assert(!strcmp(elem->message, "this is message1"));

    elem = errstack_getc(stack, 1);
    assert(elem);
    assert(!strcmp(elem->filename, "file2"));
    assert(elem->lineno == 2);
    assert(!strcmp(elem->funcname, "func2"));
    assert(!strcmp(elem->message, "this is message2"));

    assert(errstack_getc(stack, 2) == NULL);

    errstack_del(stack);
}

static void
test_errstack_resize(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_len(stack) == 0);
    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_pushb(stack, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(stack, "file4", 4, "func4", "this is %s", "message4"));
    assert(errstack_pushb(stack, "file5", 5, "func5", "this is %s", "message5"));
    assert(errstack_len(stack) == 5);

    const errelem_t *elem = errstack_getc(stack, 0);
    assert(elem);
    assert(!strcmp(elem->filename, "file1"));
    assert(elem->lineno == 1);
    assert(!strcmp(elem->funcname, "func1"));
    assert(!strcmp(elem->message, "this is message1"));

    elem = errstack_getc(stack, 1);
    assert(elem);
    assert(!strcmp(elem->filename, "file2"));
    assert(elem->lineno == 2);
    assert(!strcmp(elem->funcname, "func2"));
    assert(!strcmp(elem->message, "this is message2"));

    elem = errstack_getc(stack, 2);
    assert(elem);
    assert(!strcmp(elem->filename, "file3"));
    assert(elem->lineno == 3);
    assert(!strcmp(elem->funcname, "func3"));
    assert(!strcmp(elem->message, "this is message3"));

    elem = errstack_getc(stack, 3);
    assert(elem);
    assert(!strcmp(elem->filename, "file4"));
    assert(elem->lineno == 4);
    assert(!strcmp(elem->funcname, "func4"));
    assert(!strcmp(elem->message, "this is message4"));

    elem = errstack_getc(stack, 4);
    assert(elem);
    assert(!strcmp(elem->filename, "file5"));
    assert(elem->lineno == 5);
    assert(!strcmp(elem->funcname, "func5"));
    assert(!strcmp(elem->message, "this is message5"));

    errstack_del(stack);
}

static void
test_errstack_trace(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));

    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    errstack_trace(stack, stderr);
    assert(strcmp(buf, "Error:\n    file1: 1: func1: This is message1.\n    file2: 2: func2: This is message2."));

    fseek(stderr, 0, SEEK_SET);
    setbuf(stderr, NULL);
    errstack_del(stack);
}

static void
test_errelem_show(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));

    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    const errelem_t *elem = errstack_getc(stack, 0);
    errelem_show(elem, stderr);
    assert(!strcmp(buf, "file1: 1: func1: This is message1.\n"));

    fseek(stderr, 0, SEEK_SET);
    buf[0] = '\0';

    elem = errstack_getc(stack, 1);
    errelem_show(elem, stderr);
    assert(!strcmp(buf, "file2: 2: func2: This is message2.\n"));

    setbuf(stderr, NULL);
    errstack_del(stack);
}

static void
test_errstack_extendf_other_0(void) {
    errstack_t *stack = errstack_new();
    errstack_t *other = errstack_new();

    assert(errstack_pushb(stack, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(stack, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_pushb(other, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(other, "file2", 2, "func2", "this is %s", "message2"));

    assert(errstack_len(stack) == 2);
    assert(errstack_len(other) == 2);

    assert(errstack_extendf_other(stack, other));
    assert(errstack_len(stack) == 4);
    assert(errstack_len(other) == 2);

    errstack_del(stack);
    errstack_del(other);
}

static void
test_errstack_extendf_other_1(void) {
    errstack_t *stack = errstack_new();
    errstack_t *other = errstack_new();

    assert(errstack_pushb(stack, "file5", 5, "func5", "this is %s", "message5"));
    assert(errstack_pushb(stack, "file6", 6, "func6", "this is %s", "message6"));

    assert(errstack_pushb(other, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(other, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_pushb(other, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(other, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_len(stack) == 2);
    assert(errstack_len(other) == 4);

    assert(errstack_extendf_other(stack, other));
    assert(errstack_len(stack) == 6);
    assert(errstack_len(other) == 4);

    errstack_del(stack);
    errstack_del(other);
}

static void
test_errstack_extendf_other_2(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_pushb(stack, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(stack, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_len(stack) == 4);

    assert(errstack_extendf_other(stack, stack));
    assert(errstack_len(stack) == 8);

    errstack_del(stack);
}

static void
test_errstack_extendb_other_0(void) {
    errstack_t *stack = errstack_new();
    errstack_t *other = errstack_new();

    assert(errstack_pushb(stack, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(stack, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_pushb(other, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(other, "file2", 2, "func2", "this is %s", "message2"));

    assert(errstack_len(stack) == 2);
    assert(errstack_len(other) == 2);

    assert(errstack_extendb_other(stack, other));
    assert(errstack_len(stack) == 4);
    assert(errstack_len(other) == 2);

    errstack_del(stack);
    errstack_del(other);
}

static void
test_errstack_extendb_other_1(void) {
    errstack_t *stack = errstack_new();
    errstack_t *other = errstack_new();

    assert(errstack_pushb(stack, "file5", 5, "func5", "this is %s", "message5"));
    assert(errstack_pushb(stack, "file6", 4, "func6", "this is %s", "message6"));

    assert(errstack_pushb(other, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(other, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_pushb(other, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(other, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_len(stack) == 2);
    assert(errstack_len(other) == 4);

    assert(errstack_extendb_other(stack, other));
    assert(errstack_len(stack) == 6);
    assert(errstack_len(other) == 4);

    errstack_del(stack);
    errstack_del(other);
}

static void
test_errstack_extendb_other_2(void) {
    errstack_t *stack = errstack_new();

    assert(errstack_pushb(stack, "file1", 1, "func1", "this is %s", "message1"));
    assert(errstack_pushb(stack, "file2", 2, "func2", "this is %s", "message2"));
    assert(errstack_pushb(stack, "file3", 3, "func3", "this is %s", "message3"));
    assert(errstack_pushb(stack, "file4", 4, "func4", "this is %s", "message4"));

    assert(errstack_len(stack) == 4);

    assert(errstack_extendb_other(stack, stack));
    assert(errstack_len(stack) == 8);

    errstack_del(stack);
}

static const struct testcase
errstack_tests[] = {
    {"errstack_new", test_errstack_new},
    {"errstack_pushb", test_errstack_pushb},
    {"errstack_resize", test_errstack_resize},
    {"errstack_trace", test_errstack_trace},
    {"errstack_extendf_other_0", test_errstack_extendf_other_0},
    {"errstack_extendf_other_1", test_errstack_extendf_other_1},
    {"errstack_extendf_other_2", test_errstack_extendf_other_2},
    {"errstack_extendb_other_0", test_errstack_extendb_other_0},
    {"errstack_extendb_other_1", test_errstack_extendb_other_1},
    {"errstack_extendb_other_2", test_errstack_extendb_other_2},
    {"errelem_show", test_errelem_show},
    {0},
};

/**********
* lang/gc *
**********/

static void
test_lang_gc_new(void) {
    gc_t *gc = gc_new();
    assert(gc);
    gc_del(gc);
}

static void
test_lang_gc_alloc(void) {
    gc_t *gc = gc_new();
    assert(gc);

    gc_item_t item = {0};
    gc_alloc(gc, &item, 100);

    assert(item.ptr);
    assert(item.ref_counts == 0);

    item.ref_counts++;
    item.ref_counts++;

    gc_free(gc, &item);
    assert(item.ptr);
    assert(item.ref_counts == 2);

    item.ref_counts--;
    gc_free(gc, &item);
    assert(item.ptr);
    assert(item.ref_counts == 1);

    item.ref_counts--;
    gc_free(gc, &item);
    assert(item.ptr == NULL);
    assert(item.ref_counts == 0);

    gc_del(gc);
}

static const struct testcase
lang_gc_tests[] = {
    {"gc_new", test_lang_gc_new},
    {"gc_alloc", test_lang_gc_alloc},
    {0},
};

/*******************
* lang/object_dict *
*******************/

static void
test_lang_objdict_move(void) {
    gc_t *gc = gc_new();
    object_dict_t *d = objdict_new(gc);

    object_t *obj1 = obj_new_int(gc, 1);
    obj_inc_ref(obj1);
    objdict_move(d, "abc", obj1);
    assert(objdict_len(d) == 1);

    object_t *obj2 = obj_new_int(gc, 1);
    obj_inc_ref(obj2);
    objdict_move(d, "def", obj2);
    assert(objdict_len(d) == 2);

    object_dict_item_t *item1 = objdict_get(d, "abc");
    assert(obj1 == item1->value);

    object_dict_item_t *item2 = objdict_get(d, "def");
    assert(obj2 == item2->value);

    objdict_del(d);
    gc_del(gc);
}

static void
test_lang_objdict_set(void) {
    gc_t *gc = gc_new();
    object_dict_t *d = objdict_new(gc);

    object_t *obj1 = obj_new_int(gc, 1);
    obj_inc_ref(obj1);
    objdict_move(d, "abc", obj1);
    assert(objdict_len(d) == 1);

    object_t *obj2 = obj_new_int(gc, 1);
    obj_inc_ref(obj2);
    objdict_move(d, "def", obj2);
    assert(objdict_len(d) == 2);

    object_dict_item_t *item1 = objdict_get(d, "abc");
    assert(obj1 == item1->value);

    object_dict_item_t *item2 = objdict_get(d, "def");
    assert(obj2 == item2->value);

    objdict_del(d);
    gc_del(gc);
}

static void
test_lang_objdict_pop(void) {
    /**********
    * pop one *
    **********/

    gc_t *gc = gc_new();
    object_dict_t *d = objdict_new(gc);
    object_t *obj = obj_new_int(gc, 0);

    obj_inc_ref(obj);
    objdict_move(d, "abc", obj);
    assert(objdict_len(d) == 1);
    object_t *popped = objdict_pop(d, "abc");
    assert(popped);
    assert(objdict_len(d) == 0);
    assert(obj == popped);

    objdict_del(d);
    gc_del(gc);

    /***********
    * pop many *
    ***********/

    gc = gc_new();
    d = objdict_new(gc);

    for (int32_t i = 0; i < 10; ++i) {
        object_t *obj = obj_new_int(gc, i);
        char key[10];
        snprintf(key, sizeof key, "obj%d", i);
        obj_inc_ref(obj);
        objdict_move(d, key, obj);
    }
    assert(objdict_len(d) == 10);

    for (int32_t i = 0; i < 10; ++i) {
        char key[10];
        snprintf(key, sizeof key, "obj%d", i);
        object_t *popped = objdict_pop(d, key);
        assert(popped);
    }
    assert(objdict_len(d) == 0);

    objdict_del(d);
    gc_del(gc);
}

static const struct testcase
lang_object_dict_tests[] = {
    {"move", test_lang_objdict_move},
    {"set", test_lang_objdict_set},
    {"pop", test_lang_objdict_pop},
    {0},
};

/*************
* cd command *
*************/

static void
test_cdcmd_default(void) {
    config_t *config = config_new();
    int argc = 2;
    char *argv[] = {
        "cd",
        "tests/path/to/dir/",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));
    assert(solve_path(config->var_cd_path, sizeof config->var_cd_path, "./tests/.cap/var/cd"));

    cdcmd_t *cdcmd = cdcmd_new(config, argc, argv);
    cdcmd_run(cdcmd);
    cdcmd_del(cdcmd);

    char line[1024];
    assert(file_readline(line, sizeof line, config->var_cd_path));
    assert(!strstr(line, "tests/.cap/var/cd"));
}

static const struct testcase
cdcmd_tests[] = {
    {"default", test_cdcmd_default},
    {0},
};

/**************
* pwd command *
**************/

static void
test_pwdcmd_default(void) {
    config_t *config = config_new();
    int argc = 1;
    char *argv[] = {
        "pwd",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests/path/to/dir"));

    char stdout_buf[1024];
    setbuf(stdout, stdout_buf);

    pwdcmd_t *pwdcmd = pwdcmd_new(config, argc, argv);
    pwdcmd_run(pwdcmd);
    pwdcmd_del(pwdcmd);

    setbuf(stdout, NULL);

    assert(strstr(stdout_buf, "/tests/path/to/dir"));
}

static void
test_pwdcmd_nomalize_opt(void) {
    config_t *config = config_new();
    int argc = 2;
    char *argv[] = {
        "pwd",
        "-n",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests/path/to/dir"));

    char stdout_buf[1024];
    setbuf(stdout, stdout_buf);

    pwdcmd_t *pwdcmd = pwdcmd_new(config, argc, argv);
    pwdcmd_run(pwdcmd);
    pwdcmd_del(pwdcmd);

    setbuf(stdout, NULL);

#ifdef _TESTS_WINDOWS
    assert(strstr(stdout_buf, "\\tests\\path\\to\\dir"));
#else
    assert(strstr(stdout_buf, "/tests/path/to/dir"));
#endif
}

static const struct testcase
pwdcmd_tests[] = {
    {"default", test_pwdcmd_default},
    {"normalize", test_pwdcmd_nomalize_opt},
    {0},
};

/*************
* ls command *
*************/

static void
test_lscmd_default(void) {

    return;  // TODO

    config_t *config = config_new();
    int argc = 1;
    char *argv[] = {
        "ls",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests/ls"));

    char buf[1024] = {0};
    setvbuf(stdout, buf, _IOFBF, sizeof buf);

    lscmd_t *lscmd = lscmd_new(config, argc, argv);
    lscmd_run(lscmd);
    lscmd_del(lscmd);

    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    assert(!strcmp(buf, "a\nb\nc\n"));
    printf("stdout[%s]\n", buf);
}

static const struct testcase
lscmd_tests[] = {
    {"default", test_lscmd_default},
    {0},
};

/**************
* cat command *
**************/

static void
test_catcmd_default(void) {
    config_t *config = config_new();
    int argc = 2;
    char *argv[] = {
        "cat",
        "/tests/resources/hello.txt",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    setbuf(stdout, stdout_buf);
    catcmd_run(catcmd);
    assert(!strcmp(stdout_buf, "hello\n"));
    setbuf(stdout, NULL);

    catcmd_del(catcmd);
    config_del(config);
}

static void
test_catcmd_indent_opt(void) {
    config_t *config = config_new();
    int argc = 4;
    char *argv[] = {
        "cat",
        "/tests/resources/hello.txt",
        "-i",
        "2",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    setbuf(stdout, stdout_buf);
    catcmd_run(catcmd);
    assert(!strcmp(stdout_buf, "        hello\n"));
    setbuf(stdout, NULL);

    catcmd_del(catcmd);
    config_del(config);
}

static void
test_catcmd_tab_opt(void) {
    config_t *config = config_new();
    int argc = 5;
    char *argv[] = {
        "cat",
        "/tests/resources/hello.txt",
        "-i",
        "2",
        "-t",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    setbuf(stdout, stdout_buf);
    catcmd_run(catcmd);
    assert(!strcmp(stdout_buf, "\t\thello\n"));
    setbuf(stdout, NULL);

    catcmd_del(catcmd);
    config_del(config);
}

static void
test_catcmd_tabspaces_opt(void) {
    config_t *config = config_new();
    int argc = 6;
    char *argv[] = {
        "cat",
        "/tests/resources/hello.txt",
        "-i",
        "2",
        "-T",
        "2",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    setbuf(stdout, stdout_buf);
    catcmd_run(catcmd);
    assert(!strcmp(stdout_buf, "    hello\n"));
    setbuf(stdout, NULL);

    catcmd_del(catcmd);
    config_del(config);
}

static void
test_catcmd_make_opt(void) {
    config_t *config = config_new();
    int argc = 3;
    char *argv[] = {
        "cat",
        "-m",
        "/tests/resources/hello.cap",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    setbuf(stdout, stdout_buf);
    catcmd_run(catcmd);
    assert(!strcmp(stdout_buf, "hello"));
    setbuf(stdout, NULL);

    catcmd_del(catcmd);
    config_del(config);
}

/**
 * TODO
 */
static void
test_catcmd_make_opt_1(void) {
    config_t *config = config_new();
    int argc = 4;
    char *argv[] = {
        "cat",
        "-m",
        "/tests/resources/hello.cap",
        "/tests/resources/hello.cap",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char buf[1024] = {0};
    catcmd_t *catcmd = catcmd_new(config, argc, argv);

    // _IOFBF is full buffering mode
    // stdout use line buffer mode default
    setvbuf(stdout, buf, _IOFBF, sizeof buf);
    // catcmd_run(catcmd);
    // assert(!strcmp(buf, "hellohello"));
    // setvbuf(stdout, NULL, _IOLBF, 0);
    puts("ababa"); // <- missing
    puts("higege");
    fflush(stdout);
    setbuf(stdout, NULL);

    // why not write "ababa" at buffer?
    fprintf(stderr, "stdout[%s]\n", buf);

    catcmd_del(catcmd);
    config_del(config);
}

static void
test_catcmd_all(void) {
    test_catcmd_default();
    test_catcmd_indent_opt();
    test_catcmd_tab_opt();
    test_catcmd_tabspaces_opt();
    test_catcmd_make_opt();
    // test_catcmd_make_opt_1();
}

static const struct testcase
catcmd_tests[] = {
    {"default", test_catcmd_default},
    {"indent", test_catcmd_indent_opt},
    {"tab", test_catcmd_tab_opt},
    {"tabspaces", test_catcmd_tabspaces_opt},
    {"make", test_catcmd_make_opt},
    {"make_1", test_catcmd_make_opt_1},
    {"all", test_catcmd_all},
    {0},
};

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    // commands
    {"cd", cdcmd_tests},
    {"pwd", pwdcmd_tests},
    {"ls", lscmd_tests},
    {"cat", catcmd_tests},

    // lib
    {"cstring_array", cstrarr_tests},
    {"string", string_tests},
    {"file", file_tests},
    {"cl", cl_tests},
    {"cmdline", cmdline_tests},
    {"error", error_tests},
    {"util", utiltests},
    {"path", pathtests},
    {"opts", lang_opts_tests},
    {"tokenizer", tokenizer_tests},
    {"compiler", compiler_tests},
    {"traverser", traverser_tests},
    {"symlink", symlink_tests},
    {"error_stack", errstack_tests},
    {"gc", lang_gc_tests},
    {"objdict", lang_object_dict_tests},
    {0},
};

struct opts {
    bool ishelp;
    int32_t argc;
    char **argv;
    int32_t optind;
};

static int32_t
parseopts(struct opts *opts, int argc, char *argv[]) {
    // Init opts
    *opts = (struct opts) {0};
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
run(const struct opts *opts) {
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
    struct opts opts;
    if (parseopts(&opts, argc, argv) != 0) {
        die("failed to parse options");
    }

    run(&opts);
    cleanup();

    return 0;
}
