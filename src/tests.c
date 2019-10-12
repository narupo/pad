/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "tests.h"

/*********
* macros *
*********/

#define showbuf() printf("buf[%s]\n", ctx_getc_buf(ctx));

#define showdetail() printf("detail[%s]\n", ast_get_error_detail(ast));

#define ast_debug(stmt) \
    ast_set_debug(ast, true); \
    stmt; \
    ast_set_debug(ast, false); \

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
    {},
};

/*********
* string *
*********/

static void
test_cstring_cstr_app(void) {
    char dst[100] = {};

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
test_cstring_cstr_appfmt(void) {
    char dst[100] = {};

    assert(cstr_appfmt(dst, sizeof dst, NULL) == NULL);
    assert(cstr_appfmt(NULL, sizeof dst, "source") == NULL);
    assert(cstr_appfmt(dst, 0, "source") == NULL);

    assert(cstr_appfmt(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(cstr_appfmt(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(cstr_appfmt(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(cstr_appfmt(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);

    *dst = '\0';
    assert(cstr_appfmt(dst, sizeof dst, "n %d is %c", 10, 'i') != NULL);
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
test_str_escdel(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_escdel(NULL) == NULL);
    char *ptr = str_escdel(s);
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
test_str_newother(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    assert(str_newother(NULL) == NULL);
    string_t *o = str_newother(s);
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
test_str_appstream(void) {
    string_t *s = str_new();
    assert(s != NULL);


    char curdir[1024];
    char path[1024];
    assert(file_realpath(curdir, sizeof curdir, ".") != NULL);
    assert(file_solvefmt(path, sizeof path, "%s/src/tests.c", curdir) != NULL);

    FILE *fin = fopen(path, "r");
    assert(fin != NULL);
    assert(str_appstream(NULL, fin) == NULL);
    assert(str_appstream(s, NULL) == NULL);
    assert(str_appstream(s, fin) != NULL);
    assert(fclose(fin) == 0);

    str_del(s);
}

static void
test_str_appother(void) {
    string_t *s = str_new();
    assert(s != NULL);
    assert(str_set(s, "1234") != NULL);
    string_t *o = str_new();
    assert(o != NULL);
    assert(str_set(o, "1234") != NULL);
    assert(str_appother(NULL, o) == NULL);
    assert(str_appother(s, NULL) == NULL);
    assert(str_appother(s, o) != NULL);
    assert(strcmp(str_getc(s), "12341234") == 0);
    str_del(o);
    str_del(s);
}

static void
test_str_appfmt(void) {
    string_t *s = str_new();
    assert(s != NULL);
    char buf[1024];
    assert(str_appfmt(NULL, buf, sizeof buf, "%s", "test") == NULL);
    assert(str_appfmt(s, NULL, sizeof buf, "%s", "test") == NULL);
    assert(str_appfmt(s, buf, 0, "%s", "test") == NULL);
    assert(str_appfmt(s, buf, sizeof buf, NULL, "test") == NULL);
    assert(str_appfmt(s, buf, sizeof buf, "%s %d %c", "1234", 1, '2') != NULL);
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

static const struct testcase
string_tests[] = {
    {"cstr_app", test_cstring_cstr_app},
    {"cstr_appfmt", test_cstring_cstr_appfmt},
    {"cstr_cpywithout", test_cstring_cstr_cpywithout},
    {"cstr_edup", test_cstring_cstr_edup},
    {"str_del", test_str_del},
    {"str_escdel", test_str_escdel},
    {"str_new", test_str_new},
    {"str_newother", test_str_newother},
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
    {"str_appstream", test_str_appstream},
    {"str_appother", test_str_appother},
    {"str_appfmt", test_str_appfmt},
    {"str_rstrip", test_str_rstrip},
    {"str_lstrip", test_str_lstrip},
    {"str_strip", test_str_strip},
    {"str_findc", test_str_findc},
    {},
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

    char src[FILE_NPATH] = {0};
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
    {},
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
    assert(cl_parse_str_opts(cl, "cmd -h -ab 123 --help 223", CL_WRAP | CL_ESCAPE));
    assert(strcmp(cl_getc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cl_getc(cl, 1), "'-h'") == 0);
    assert(strcmp(cl_getc(cl, 2), "'-ab'") == 0);
    assert(strcmp(cl_getc(cl, 3), "'123'") == 0);
    assert(strcmp(cl_getc(cl, 4), "'--help'") == 0);
    assert(strcmp(cl_getc(cl, 5), "'223'") == 0);
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

static const struct testcase
error_tests[] = {
    {"fix_text", test_error_fix_text},
    {"_log", test_error__log},
    {"die", test_error_die},
    {"error", test_error_error},
    {},
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
        {},
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

static const struct testcase
utiltests[] = {
    {"test_util_freeargv", test_util_freeargv},
    {"test_util_showargv", test_util_showargv},
    {"test_util_isoutofhome", test_util_isoutofhome},
    {"test_util_randrange", test_util_randrange},
    {"test_util_safesystem", test_util_safesystem},
    {"test_util_argsbyoptind", test_util_argsbyoptind},
    {},
};


/*********
* socket *
*********/

/*
static void
test_socket_sockshow(void) {
    struct cap_socket *s = cap_sockopen((uint8_t *) "localhost:80", (uint8_t *) "tcp-client");
    assert(s != NULL);
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);
    cap_sockshow(s);
    setbuf(stderr, NULL);
    puts(buf);
    assert(cap_sockclose(s) == 0);
}

static void
test_socket_sockclose(void) {
    // test_socket_sockshow
}

static void
test_socket_sockopen(void) {
    // test_socket_sockshow
}

static void
test_socket_sockhost(void) {
    struct cap_socket *s = cap_sockopen((uint8_t *) "localhost:80", (uint8_t *) "tcp-client");
    assert(s != NULL);
    assert(strcmp((const char *) cap_sockhost(s), "localhost") == 0);
    assert(cap_sockclose(s) == 0);
}

static void
test_socket_sockport(void) {
    struct cap_socket *s = cap_sockopen((uint8_t *) "localhost:80",(uint8_t *) "tcp-client");
    assert(s != NULL);
    assert(strcmp((const char *) cap_sockport(s), "80") == 0);
    assert(cap_sockclose(s) == 0);
}

static void
test_socket_sockaccept(void) {
}

static void
test_socket_sockrecvstr(void) {
    struct cap_socket *s = cap_sockopen((uint8_t *) "https://google.com:80",(uint8_t *) "tcp-client");
    assert(s != NULL);
    assert(cap_socksendstr(s, (const uint8_t *) "GET / HTTP/1.1\r\n\r\n") >= 0);
    assert(cap_sockclose(s) == 0);    
}

static void
test_socket_socksendstr(void) {
}

static void
test_socket_socksend(void) {
}

static const struct testcase
sockettests[] = {
    {"sockshow", test_socket_sockshow},
    {"sockclose", test_socket_sockclose},
    {"sockopen", test_socket_sockopen},
    {"sockhost", test_socket_sockhost},
    {"sockport", test_socket_sockport},
    {"sockaccept", test_socket_sockaccept},
    {"sockrecvstr", test_socket_sockrecvstr},
    {"socksendstr", test_socket_socksendstr},
    {"socksend", test_socket_socksend},
    {},
};

static void
test_url_urldel(void) {
    struct cap_url *o = cap_urlnew();
    assert(o != NULL);
    cap_urldel(o);
}

static void
test_url_urlnew(void) {
    // cap_urldel
}

static void
test_url_urlparse(void) {
    const uint8_t *src = (const uint8_t *) "localhost:8000";
    struct cap_url *o = cap_urlnew();
    assert(o != NULL);
    assert(cap_urlparse(NULL, src) == NULL);
    assert(cap_urlparse(o, NULL) == NULL);
    assert(cap_urlparse(o, src));
    assert(cap_urlhost(NULL) == NULL);
    assert(memcmp(cap_urlhost(o), "localhost", strlen("localhost")) == 0);
    assert(cap_urlport(NULL) == -1);
    assert(cap_urlport(o) == 8000);
    cap_urldel(o);    
}

static void
test_url_urlhost(void) {
    // urlparse
}

static void
test_url_urlport(void) {
    // urlparse
}

static const struct testcase
urltests[] = {
    {"cap_urldel", test_url_urldel},
    {"cap_urlnew", test_url_urlnew},
    {"cap_urlparse", test_url_urlparse},
    {"cap_urlhost", test_url_urlhost},
    {"cap_urlport", test_url_urlport},
    {},
};
*/

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
        assert(tkr_has_error(tkr) == false);
        assert(tkr_tokens_len(tkr) == 0);
    }

    tkr_parse(tkr, "{@{");
    {
        assert(tkr_tokens_len(tkr) == 1);
        assert(tkr_has_error(tkr) == true);
        assert(strcmp(tkr_get_error_detail(tkr), "syntax error. unsupported character \"{\"") == 0);
    }
    
    tkr_parse(tkr, "{@");
    {
        assert(tkr_tokens_len(tkr) == 1);
        assert(tkr_has_error(tkr) == true);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
    }

    tkr_parse(tkr, "{@@");
    {
        assert(tkr_has_error(tkr) == true);
        assert(strcmp(tkr_get_error_detail(tkr), "invalid syntax. single '@' is not supported") == 0);
    }

    tkr_parse(tkr, "{@@}");
    {
        assert(tkr_tokens_len(tkr) == 2);
        token = tkr_tokens_getc(tkr, 0);
        assert(token->type == TOKEN_TYPE_LBRACEAT);
        token = tkr_tokens_getc(tkr, 1);
        assert(token->type == TOKEN_TYPE_RBRACEAT);
    }

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
        assert(tkr_has_error(tkr) == true);
        assert(strcmp(tkr_get_error_detail(tkr), "not closed by block") == 0);
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
        assert(tkr_has_error(tkr) == true);
        assert(strcmp(tkr_get_error_detail(tkr), "syntax error. unsupported character \"\n\"") == 0);
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

    tkr_del(tkr);
}

static const struct testcase
tokenizer_tests[] = {
    {"tkr_new", test_tkr_new},
    {"tkr_parse", test_tkr_parse},
    {0},
};

/******
* ast *
******/

static void
test_ast_show_error(const ast_t *ast) {
    if (ast_has_error(ast)) {
        printf("error detail[%s]\n", ast_get_error_detail(ast));
    }
}

static void
test_ast_parse(void) {
    // head
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(opt);
    ast_t *ast = ast_new();
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
    node_import_stmt_t *import_stmt;
    node_identifier_chain_t *identifier_chain;
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
    node_asscalc_t *asscalc;
    node_factor_t *factor;
    node_atom_t *atom;
    node_augassign_t *augassign;
    node_comp_op_t *comp_op;
    node_add_sub_op_t *add_sub_op;
    node_mul_div_op_t *mul_div_op;
    node_digit_t *digit;
    node_caller_t *caller;
    node_string_t *string;
    node_nil_t *nil;
    node_break_stmt_t *break_stmt;
    node_continue_stmt_t *continue_stmt;
    node_def_t *def;
    node_func_def_t *func_def;
    node_func_def_params_t *func_def_params;
    node_func_def_args_t *func_def_args;

    tkr_parse(tkr, "");
    ast_clear(ast);
    ast_parse(ast, tkr_get_tokens(tkr));
    root = ast_getc_root(ast);
    assert(root == NULL);

    /**********
    * formula *
    **********/

    tkr_parse(tkr, "{@ a = 1 @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;        
        assert(digit != NULL);
        assert(digit->lvalue == 1);    
    } 

    tkr_parse(tkr, "{@ a = \"abc\" @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));    
    } 

    tkr_parse(tkr, "{@ a = b = 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "b"));
        test = nodearr_get(assign->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);    
    } 

    tkr_parse(tkr, "{@ a = 1, b = 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier);
        assert(!strcmp(identifier->identifier, "b"));    
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit);
        assert(digit->lvalue == 2);
    }

    /*******
    * expr *
    *******/

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    }

    tkr_parse(tkr, "{@ 1 == 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        expr = nodearr_get(comparison->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    } 

    tkr_parse(tkr, "{@ 1 == 2 == 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_EQ);
        expr = nodearr_get(comparison->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = nodearr_get(comparison->nodearr, 3)->real;
        assert(comp_op->op == OP_EQ);
        expr = nodearr_get(comparison->nodearr, 4)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    } 

    tkr_parse(tkr, "{@ 1 != 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_NOT_EQ);
        expr = nodearr_get(comparison->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    }

    tkr_parse(tkr, "{@ 1 != 2 != 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        comp_op = nodearr_get(comparison->nodearr, 1)->real;
        assert(comp_op->op == OP_NOT_EQ);
        expr = nodearr_get(comparison->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        comp_op = nodearr_get(comparison->nodearr, 3)->real;
        assert(comp_op->op == OP_NOT_EQ);
        expr = nodearr_get(comparison->nodearr, 4)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    tkr_parse(tkr, "{@ 1 + 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    }

    tkr_parse(tkr, "{@ 1 + 2 + 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
        term = nodearr_get(expr->nodearr, 4)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    tkr_parse(tkr, "{@ 1 - 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    }

    tkr_parse(tkr, "{@ 1 - 2 - 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
        term = nodearr_get(expr->nodearr, 4)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    }

    tkr_parse(tkr, "{@ 1 * 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        factor = nodearr_get(term->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    } 

    tkr_parse(tkr, "{@ 1 * 2 * 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        factor = nodearr_get(term->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
        factor = nodearr_get(term->nodearr, 4)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);    
    } 

    tkr_parse(tkr, "{@ 1 / 2 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        factor = nodearr_get(term->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    }

    tkr_parse(tkr, "{@ 1 / 2 / 3 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        factor = nodearr_get(term->nodearr, 2)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
        factor = nodearr_get(term->nodearr, 4)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);    
    } 

    /**********
    * asscalc *
    **********/

    tkr_parse(tkr, "{@ a += 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_ADD_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);    
    }

    tkr_parse(tkr, "{@ a += \"b\" @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_ADD_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "b"));    
    }

    tkr_parse(tkr, "{@ a -= 1 @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        augassign = nodearr_get(asscalc->nodearr, 1)->real;
        assert(augassign->op == OP_SUB_ASS);
        expr = nodearr_get(asscalc->nodearr, 2)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);    
    }

    /* tkr_parse(tkr, "{@ a *= 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(augassign->op == OP_MUL_ASS);
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

    /* tkr_parse(tkr, "{@ a /= 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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

    tkr_parse(tkr, "{@ func() @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    } 

    /*******
    * atom *
    *******/

    tkr_parse(tkr, "{@ nil @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        assert(atom->nil->type == NODE_TYPE_NIL);
        nil = atom->nil->real;
        assert(nil);
    }     

    /***********
    * func_def *
    ***********/

    tkr_parse(tkr, "{@ def func(a, b): end @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
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

        elems = func_def->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
        assert(!ast_has_error(ast));
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

        elems = func_def->elems->real;
        formula = elems->formula->real;
        assign_list = formula->assign_list->real;
        assign = nodearr_get(assign_list->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "a"));
        test = nodearr_get(assign->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;        
        assert(digit != NULL);
        assert(digit->lvalue == 1);    
    } 

    /*********
    * caller *
    *********/

    tkr_parse(tkr, "{@ func() + 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    } 

    tkr_parse(tkr, "{@ my.func() @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "my"));
        identifier_chain = identifier_chain->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    } 

    tkr_parse(tkr, "{@ my.func(1) @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
    } 

    tkr_parse(tkr, "{@ my.func(1, 2) @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        factor = nodearr_get(term->nodearr, 0)->real;
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    } 

    tkr_parse(tkr, "{@ my.func(\"abc\") @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
    } 

    tkr_parse(tkr, "{@ my.func(\"abc\", \"def\") @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
    } 

    tkr_parse(tkr, "{@ my.func(\"abc\", \"def\", \"ghi\") @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "def"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, "ghi"));
    } 

    tkr_parse(tkr, "{@ my.func(\"\", \"\") @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
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
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(!strcmp(string->string, ""));
    } 

    /************
    * test_list *
    ************/

    tkr_parse(tkr, "{@ 1, 2 @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        formula = elems->formula->real;
        assert(formula->assign_list == NULL);
        assert(formula->multi_assign);
        multi_assign = formula->multi_assign->real;
        assign = nodearr_get(multi_assign->nodearr, 0)->real;
        test = nodearr_get(assign->nodearr, 0)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        assert(assign);
        assert(assign);
        test = nodearr_get(assign->nodearr, 1)->real;
        assert(test);
        assert(test->or_test);
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
    }

    tkr_parse(tkr, "{@ 1, 2, 3 @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 3);
    } 

    tkr_parse(tkr, "{@ \"abc\", \"def\" @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
    }

    tkr_parse(tkr, "{@ \"abc\", \"def\", \"ghi\" @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "ghi"));
    }

    tkr_parse(tkr, "{@ 1, \"def\" @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "def"));
    }

    tkr_parse(tkr, "{@ 1, var @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
    }

    tkr_parse(tkr, "{@ 1, var, \"abc\" @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
    }

    tkr_parse(tkr, "{@ 1, var, \"abc\", func() @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        test = nodearr_get(test_list->nodearr, 1)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        identifier = atom->identifier->real;
        assert(identifier != NULL);
        assert(!strcmp(identifier->identifier, "var"));
        test = nodearr_get(test_list->nodearr, 2)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        string = atom->string->real;
        assert(string != NULL);
        assert(!strcmp(string->string, "abc"));
        test = nodearr_get(test_list->nodearr, 3)->real;
        or_test = test->or_test->real;
        and_test = nodearr_get(or_test->nodearr, 0)->real;
        not_test = nodearr_get(and_test->nodearr, 0)->real;
        comparison = not_test->comparison->real;
        expr = nodearr_get(comparison->nodearr, 0)->real;
        term = nodearr_get(expr->nodearr, 0)->real;
        asscalc = nodearr_get(term->nodearr, 0)->real;
        factor = nodearr_get(asscalc->nodearr, 0)->real;
        atom = factor->atom->real;
        caller = atom->caller->real;
        assert(caller != NULL);
        identifier_chain = caller->identifier_chain->real;
        identifier = identifier_chain->identifier->real;
        assert(!strcmp(identifier->identifier, "func"));
    }

    /*******
    * test *
    *******/

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        ast_parse(ast, tkr_get_tokens(tkr));
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

    /*******************
    * import statement *
    *******************/

    tkr_parse(tkr, "{@ import module @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt->identifier_chain != NULL);
        assert(import_stmt->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(import_stmt->identifier_chain->real != NULL);
        identifier_chain = import_stmt->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain == NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "module"));
    }

    tkr_parse(tkr, "{@\n\nimport module @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt->identifier_chain != NULL);
        assert(import_stmt->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(import_stmt->identifier_chain->real != NULL);
        identifier_chain = import_stmt->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain == NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "module"));
    }

    tkr_parse(tkr, "{@ import module\n @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt->identifier_chain != NULL);
        assert(import_stmt->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(import_stmt->identifier_chain->real != NULL);
        identifier_chain = import_stmt->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain == NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "module"));
    }

    tkr_parse(tkr, "{@ import module\n\n @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt->identifier_chain != NULL);
        assert(import_stmt->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(import_stmt->identifier_chain->real != NULL);
        identifier_chain = import_stmt->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain == NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "module"));
    }

    tkr_parse(tkr, "{@ import abc.ghi @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        assert(stmt->if_stmt == NULL);
        assert(stmt->for_stmt == NULL);
        import_stmt = stmt->import_stmt->real;
        assert(import_stmt->identifier_chain != NULL);
        assert(import_stmt->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(import_stmt->identifier_chain->real != NULL);
        identifier_chain = import_stmt->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain != NULL);
        assert(identifier_chain->identifier_chain->type == NODE_TYPE_IDENTIFIER_CHAIN);
        assert(identifier_chain->identifier_chain->real != NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "abc"));
        identifier_chain = identifier_chain->identifier_chain->real;
        assert(identifier_chain->identifier != NULL);
        assert(identifier_chain->identifier->type == NODE_TYPE_IDENTIFIER);
        assert(identifier_chain->identifier->real != NULL);
        assert(identifier_chain->identifier_chain == NULL);
        identifier = identifier_chain->identifier->real;
        assert(identifier->identifier != NULL);
        assert(!strcmp(identifier->identifier, "ghi"));
    }

    tkr_parse(tkr, "{@ import @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root == NULL);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. not found import module"));
    }

    tkr_parse(tkr, "{@ import abc. @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root == NULL);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. not found identifier after \".\""));
    }

    tkr_parse(tkr, "{@ import");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root == NULL);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. not found import module"));
    }

    tkr_parse(tkr, "{@ import abc");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root == NULL);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. reached EOF in identifier chain"));
    }

    tkr_parse(tkr, "{@ import abc abc @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(root == NULL);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. invalid token at end of import statement"));
    }

    /***************
    * if statement *
    ***************/

    tkr_parse(tkr, "{@ if 1: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1 + 2: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 1);
        term = nodearr_get(expr->nodearr, 2)->real;
        factor = nodearr_get(term->nodearr, 0)->real;
        atom = factor->atom->real;
        digit = atom->digit->real;
        assert(digit != NULL);
        assert(digit->lvalue == 2);    
    }

    tkr_parse(tkr, "abc{@ if 1: end @}def");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@\n\nif 1:\n\nend\n\n@}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: else: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
        assert(else_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nelse:\n\nend @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt != NULL);
        assert(if_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(if_stmt->else_stmt->real != NULL);
        else_stmt = if_stmt->else_stmt->real;
        assert(else_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 1: elif 2: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elems == NULL);
        assert(elif_stmt->blocks == NULL);
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nelif 2:\n\nend @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elems == NULL);
        assert(elif_stmt->blocks == NULL);
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: elif 2: else: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elems == NULL);
        assert(elif_stmt->blocks == NULL);
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
        assert(else_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nelif 2:\n\nelse:\n\nend @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elems == NULL);
        assert(elif_stmt->blocks == NULL);
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
        assert(else_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->else_stmt == NULL);
        assert(if_stmt->elif_stmt != NULL);
        assert(if_stmt->elif_stmt->type == NODE_TYPE_ELIF_STMT);
        assert(if_stmt->elif_stmt->real != NULL);
        elif_stmt = if_stmt->elif_stmt->real;
        assert(elif_stmt->elems == NULL);
        assert(elif_stmt->blocks == NULL);
        assert(elif_stmt->elif_stmt == NULL);
        assert(elif_stmt->else_stmt != NULL);
        assert(elif_stmt->else_stmt->type == NODE_TYPE_ELSE_STMT);
        assert(elif_stmt->else_stmt->real != NULL);
        else_stmt = elif_stmt->else_stmt->real;
        assert(else_stmt->elems != NULL);
        assert(else_stmt->blocks == NULL);
    }

    tkr_parse(tkr, "{@ if 1: if 2: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems != NULL);
        assert(if_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(if_stmt->elems->real != NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = if_stmt->elems->real;
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
        assert(if_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 1:\n\nif 2:\n\nend\n\nend @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems != NULL);
        assert(if_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(if_stmt->elems->real != NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = if_stmt->elems->real;
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
        assert(if_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ if 1: if 2: end if 3: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems != NULL);
        assert(if_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(if_stmt->elems->real != NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = if_stmt->elems->real;
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
        assert(if_stmt->elems == NULL);
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks != NULL);
        assert(if_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(if_stmt->blocks->real != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = if_stmt->blocks->real;
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
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        assert(!ast_has_error(ast));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
        root = ast_getc_root(ast);
        program = root->real;
        blocks = program->blocks->real;
        code_block = blocks->code_block->real;
        elems = code_block->elems->real;
        stmt = elems->stmt->real;
        if_stmt = stmt->if_stmt->real;
        blocks = if_stmt->blocks->real;
        text_block = blocks->text_block->real;
        assert(text_block->text != NULL);
        assert(!strcmp(text_block->text, "abc"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 2: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks != NULL);
        assert(if_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(if_stmt->blocks->real != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = if_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ if 2: end @}def{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks != NULL);
        assert(if_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(if_stmt->blocks->real != NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        blocks = if_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        assert(for_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ for 1; 1; 1: if 1: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems != NULL);
        assert(for_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(for_stmt->elems->real != NULL);
        elems = for_stmt->elems->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    } 

    tkr_parse(tkr, "{@ for 1; 1; 1: for 1; 1; 1: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems != NULL);
        assert(for_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(for_stmt->elems->real != NULL);
        elems = for_stmt->elems->real;
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
        assert(for_stmt->elems == NULL);
    } 

    tkr_parse(tkr, "{@ for 1: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
    } 

    tkr_parse(tkr, "{@ for 1: if 1: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems != NULL);
        assert(for_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(for_stmt->elems->real != NULL);
        assert(for_stmt->blocks == NULL);
        elems = for_stmt->elems->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    } 

    tkr_parse(tkr, "{@ for 1: @}{@ if 1: end @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    } 

    tkr_parse(tkr, "{@ for 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
    } 

    tkr_parse(tkr, "{@ for: end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
    }

    tkr_parse(tkr, "{@ for: @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
    } 

    tkr_parse(tkr, "{@ for: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    } 

    tkr_parse(tkr, "{@ if 1: for 1; 1; 1: end end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(if_stmt->elems != NULL);
        assert(if_stmt->elems->type == NODE_TYPE_ELEMS);
        assert(if_stmt->elems->real != NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
        elems = if_stmt->elems->real;
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
        assert(for_stmt->elems == NULL);
    } 

    tkr_parse(tkr, "{@ for 1; 1; 1: @}{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks == NULL);
    } 

    tkr_parse(tkr, "{@ for 1; 1; 1: @}abc{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
        assert(if_stmt->elif_stmt == NULL);
        assert(if_stmt->else_stmt == NULL);
    } 

    tkr_parse(tkr, "{@ for 1; 1; 1: @}abc{@ if 1: end @}def{@ end @}");
    {
        ast_clear(ast);
        ast_parse(ast, tkr_get_tokens(tkr));
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
        assert(for_stmt->elems == NULL);
        assert(for_stmt->blocks != NULL);
        assert(for_stmt->blocks->type == NODE_TYPE_BLOCKS);
        assert(for_stmt->blocks->real != NULL);
        blocks = for_stmt->blocks->real;
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
        assert(if_stmt->elems == NULL);
        assert(if_stmt->blocks == NULL);
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        assert(for_stmt->elems != NULL);
        assert(for_stmt->blocks == NULL);
        elems = for_stmt->elems->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        break_stmt = stmt->break_stmt->real;
        assert(break_stmt);
    } 

    tkr_parse(tkr, "{@ for 1; 1; 1: continue end @}");
    {
        ast_clear(ast);
        (ast_parse(ast, tkr_get_tokens(tkr)));
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
        assert(for_stmt->elems != NULL);
        assert(for_stmt->blocks == NULL);
        elems = for_stmt->elems->real;
        assert(elems->stmt != NULL);
        stmt = elems->stmt->real;
        continue_stmt = stmt->continue_stmt->real;
        assert(continue_stmt);
    } 

    tkr_del(tkr);
    ast_del(ast);
    // tail
}

static void
test_ast_traverse(void) {
    tokenizer_option_t *opt = tkropt_new();
    tokenizer_t *tkr = tkr_new(opt);
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();

    /*************
    * text block *
    *************/

    tkr_parse(tkr, "abc");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    /************
    * ref block *
    ************/

    tkr_parse(tkr, "{: 1 :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{: 123 :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "123"));
    }

    tkr_parse(tkr, "{: \"abc\" :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in ref block"));
    }

    /* tkr_parse(tkr, "{: alias(\"dtl\", \"run bin/date-line.py\") :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "nil"));
    } */

    tkr_parse(tkr, "{: 1 + 1 :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{: 1 + 1 + 1 :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "3"));
    } 

    tkr_parse(tkr, "{@ a = 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4"));
    } 

    /*******
    * atom *
    *******/

    tkr_parse(tkr, "{@ nil @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    }

    tkr_parse(tkr, "{@ 1 @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    }    

    tkr_parse(tkr, "{@ \"abc\" @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    }    

    tkr_parse(tkr, "{@ var @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    }

    tkr_parse(tkr, "{@ alias.set() @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "can't invoke alias.set. need two arguments"));
    }

    tkr_parse(tkr, "{@ alias.set(1, 2, 3) @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "can't invoke alias.set. too many arguments"));
    } 

    /***************
    * multi_assign *
    ***************/

    // error

    tkr_parse(tkr, "{@ a, b = 1, 2, 3 @}{: a :} {: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "can't assign array to array. not same length"));
    }

    tkr_parse(tkr, "{@ a, b = 2 @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "can't assign element to array"));
    }

    // success

    tkr_parse(tkr, "{@ a, b = 1, 2 @}{: a :} {: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1 2"));
    }

    /**************
    * assign_list *
    **************/

    // error

    tkr_parse(tkr, "{@ a = 1, 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "syntax error. not found assign in assign list"));
    }

    // success

    tkr_parse(tkr, "{@ a = nil @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "nil"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = \"abc\"\n b = a @}{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a = 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1, b = 2 @}{: a :},{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1,2"));
    }

    tkr_parse(tkr, "{@ a = 1 + 2, b = 3 * 4 @}{: a :},{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "3,12"));
    }

    tkr_parse(tkr, "{@ a = 1, b = 2, c = 3 @}{: a :},{: b :},{: c :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1,2,3"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = a = 1 @}{: a :},{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1,1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = a = 1, c = b = 1 @}{: a :},{: b :},{: c :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1,1,1"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ a, b = 1, 2 @}{: a :},{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1,2"));
    } 

    tkr_parse(tkr, "{@ a = alias.set(\"\", \"\") @}{: a :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "nil"));
    } 

    tkr_parse(tkr, "{@ a = alias.set(\"\", \"\")\n b = alias.set(\"\", \"\") @}{: a :},{: b :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "nil,nil"));
    } 

    tkr_parse(tkr, "{@ a = opts.get(\"abc\") @}{: a :}");
    {
        char *argv[] = {
            "-abc",
            "def",
            NULL,
        };
        opts_t *opts = opts_new();
        assert(opts_parse(opts, 2, argv));
        ast_move_opts(ast, opts);
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        ast_move_opts(ast, NULL);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "def"));
    } 

    /************
    * test_list *
    ************/

    tkr_parse(tkr, "{@ 1, 2 @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    }

    tkr_parse(tkr, "{@ 1, \"abc\", var, alias.set(\"\", \"\") @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
    } 

    tkr_parse(tkr, "{@ a = 0 \n b = 0 \n a += 1, b += 2 @}{: a :} {: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1 2"));
    }

    /*******
    * test *
    *******/

    tkr_parse(tkr, "{@ a = 0 or 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 

    tkr_parse(tkr, "{@ a = 1 or 0 or 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 

    tkr_parse(tkr, "{@ a = 0 or \"abc\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    } 

    tkr_parse(tkr, "{@ a = 1 and 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1 and 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 1 and 2 and 3 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" and 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 1 and \"abc\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    /*************
    * comparison *
    *************/

    tkr_parse(tkr, "{@ a = 1 == 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 == 0 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 == 1 == 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 == 1 == 0 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = 0 != 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 != 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "true"));
    }

    tkr_parse(tkr, "{@ a = 1 != 1 != 0 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "false"));
    }

    tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "true"));
    }

    /**
     * Pythonでは合法
     * Rubyでは違法
     */
    /* tkr_parse(tkr, "{@ a = \"abc\" == \"abc\" == \"def\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "false"));
    } */

    /*******
    * expr *
    *******/

    tkr_parse(tkr, "{@ a = 1 + 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "3"));
    }
    
    tkr_parse(tkr, "{@ a = 1 + 2 + 3 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "6"));
    }
    
    tkr_parse(tkr, "{@ a = 2 - 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }
    
    tkr_parse(tkr, "{@ a = 3 - 2 - 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "0"));
    }
    
    tkr_parse(tkr, "{@ a = 1 + 2 - 3 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "0"));
    }
    
    /*******
    * term *
    *******/

    tkr_parse(tkr, "{@ a = 2 * 3 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "6"));
    }
    
    tkr_parse(tkr, "{@ a = 2 * 3 * 4 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "24"));
    } 
    
    tkr_parse(tkr, "{@ a = 4 / 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    }
    
    tkr_parse(tkr, "{@ a = 4 / 2 / 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 
    
    tkr_parse(tkr, "{@ a = 4 / (2 / 2) @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "4"));
    } 

    tkr_parse(tkr, "{@ a = 1 + ( 2 - 3 ) * 4 / 4 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "0"));
    }
    
    /**********
    * asscalc *
    **********/

    /* FAILED
    tkr_parse(tkr, "{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } */

    tkr_parse(tkr, "{@ a += 1 @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in add ass identifier"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 + 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    } 

    tkr_parse(tkr, "{@ a = 0 \n b = 1 + 1 @}{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0 \n b = 1 + (a += 1) @}{: b :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    }

    tkr_parse(tkr, "{@ a = 0 \n a += 1 \n a += 2 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "3"));
    }

    tkr_parse(tkr, "{@ a = \"a\"\n"
        "a += \"b\" @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "ab"));
    }

    tkr_parse(tkr, "{@\n"
        "a = \"x\"\n"
        "def f():\n"
        "   a += \"y\"\n"
        "end\n"
        "f()\n"
        "@}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "xy"));
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
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), ""));
    } 

    /* TODO: sub ass
    tkr_parse(tkr, "{@ a = 0 \n a -= 1 @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "-1"));
    }*/

    /*********
    * caller *
    *********/

    tkr_parse(tkr, "{@ my.func() @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"my.func\" is not callable"));
    } 

    /********************
    * builtin functions *
    ********************/

    tkr_parse(tkr, "{@ alias.set(\"abc\", \"def\") @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        const dict_t *almap = ctx_getc_almap(ctx);
        const dict_item_t *item = dict_getc(almap, "abc");
        assert(item);
        assert(!strcmp(item->key, "abc"));
        assert(!strcmp(item->value, "def"));
    }

    tkr_parse(tkr, "{: opts.get(\"abc\") :}");
    {
        opts_t *opts = opts_new();
        char *argv[] = {
            "--abc",
            "def",
            NULL,
        };
        opts_parse(opts, 2, argv);
        ast_move_opts(ast, opts);
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ puts() @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "\n"));
    }

    tkr_parse(tkr, "{@ puts(1) @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1\n"));
    }

    tkr_parse(tkr, "{@ puts(1, 2) @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1 2\n"));
    }

    tkr_parse(tkr, "{@ puts(1, \"abc\") @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "1 abc\n"));
    }

    tkr_parse(tkr, "{@ puts(\"abc\") @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc\n"));
    }

    /*******************
    * import statement *
    *******************/

    tkr_parse(tkr, "{@ import alias @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
    }

    tkr_parse(tkr, "{@ import my.alias @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
    }

    /***************
    * if statement *
    ***************/

    tkr_parse(tkr, "{@ if 1: a = 1 end @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 

    tkr_parse(tkr, "{@ if 0: elif 1: a = 1 end @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 

    tkr_parse(tkr, "{@ if 0: elif 0: else: a = 1 end @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
    } 

    tkr_parse(tkr, "{@ if 1: @}{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), ""));
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "abc{@ if 1: @}def{@ end @}ghi");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abcdefghi"));
    }

    tkr_parse(tkr, "{@ if 1: @}{@ if 1: @}abc{@ end @}{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abc"));
    }

    tkr_parse(tkr, "{@ if 1: @}abc{@ if 1: @}def{@ end @}ghi{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "abcdefghi"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ else: @}def{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ elif 1: @}def{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "def"));
    }

    tkr_parse(tkr, "{@ if 0: @}abc{@ elif 0: @}def{@ else: @}ghi{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!strcmp(ctx_getc_buf(ctx), "ghi"));
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in roll identifier lhs"));
    } 

    tkr_parse(tkr, "{@ a = 0\n"
        "for i = 0; i != 4; i += 1:\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4"));
    } 

    tkr_parse(tkr, "{@ a = 0\n"
        "for i = 0, j = 0; i != 4; i += 1, j += 1:\n"
        "   a += 1\n"
        "end @}{: a :} {: i :} {: j :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4 4 4"));
    } 

    tkr_parse(tkr, "{@ for i = 0; i != 4; i += 1: @}a{@ end @}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "aaaa"));
    } 

    tkr_parse(tkr, "{@ for i, j = 0, 0; i != 4; i += 1, j += 2: end @}{: i :},{: j :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4,8"));
    } 

    tkr_parse(tkr, "{@ i, a = 0, 0 \n for i != 4: a += i \n i += 1 end @}{: i :},{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        ast_traverse(ast, ctx);
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4,6"));
    } 

    tkr_parse(tkr,
        "{@ for i = 0; i != 4; i += 1: @}"
        "hige\n"
        "{@ end @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "hige\nhige\nhige\nhige\n"));
    } 

    tkr_parse(tkr,
        "{@ i = 0 for i != 4: @}"
        "hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "hige\nhige\nhige\nhige\n"));
    } 

    tkr_parse(tkr,
        "{@ i = 0 for: @}"
        "{@ if i == 4: break end @}hige\n{@ i += 1 @}"
        "{@ end @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "hige\nhige\nhige\nhige\n"));
    } 

    /*******
    * jump *
    *******/

    tkr_parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   break\n"
        "end @}{: i :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "0"));
    } 

    tkr_parse(tkr, "{@\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       break\n"
        "   end\n"
        "end @}{: i :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
    } 

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   continue\n"
        "   a += 1\n"
        "end @}{: i :},{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4,0"));
    } 

    tkr_parse(tkr, "{@ a = 0\n"
        "for i=0; i!=4; i+=1:\n"
        "   if i == 2:\n"
        "       continue\n"
        "   end\n"
        "   a += 1\n"
        "end @}{: a :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "3"));
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
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "2"));
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
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "4,3"));
    } 

    /***********
    * func_def *
    ***********/

    tkr_parse(tkr, "{@ def func(): end @}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "@}{: a :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func():\n"
        "   a = 1\n"
        "end\n"
        "func()"
        "@}{: a :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func(a):\n"
        "   b = a\n"
        "end\n"
        "func(1)"
        "@}{: a :},{: b :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"a\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(ast_has_error(ast));
        assert(!strcmp(ast_get_error_detail(ast), "\"c\" is not defined in ref block"));
    }

    tkr_parse(tkr, "{@\n"
        "c = 1\n"
        "def func(a, b):\n"
        "   c = a + b\n"
        "end\n"
        "func(1, 2)\n"
        "@}{: c :}");
    {
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(!strcmp(ctx_getc_buf(ctx), "1"));
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
        (ast_parse(ast, tkr_get_tokens(tkr)));
        (ast_traverse(ast, ctx));
        object_dict_t *varmap = ctx_get_varmap(ctx);
        assert(objdict_get(varmap, "func"));
        assert(!strcmp(ctx_getc_buf(ctx), "1\n3\n1"));
    }

    /*******************
    * escape character *
    *******************/

    tkr_parse(tkr, "{: \"abc\ndef\n\" :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "abc\ndef\n"));
    } 

    tkr_parse(tkr, "{: \"\tabc\tdef\" :}");
    {
        ast_parse(ast, tkr_get_tokens(tkr));
        (ast_traverse(ast, ctx));
        assert(!ast_has_error(ast));
        assert(!strcmp(ctx_getc_buf(ctx), "\tabc\tdef"));
    } 

    // done
    ctx_del(ctx);
    ast_del(ast);
    tkr_del(tkr);
}

static const struct testcase
ast_tests[] = {
    {"ast_parse", test_ast_parse},
    {"ast_traverse", test_ast_traverse},
    {0},
};

/**********
* context *
**********/

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

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    {"cstring_array", cstrarr_tests},
    {"string", string_tests},
    {"file", file_tests},
    {"cl", cl_tests},
    {"error", error_tests},
    {"util", utiltests},
    {"tokenizer", tokenizer_tests},
    {"ast", ast_tests},
    {"symlink", symlink_tests},
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
    *opts = (struct opts) {};
    optind = 0;
    opterr = 0;

    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
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

    if (opts->argc-opts->optind > 0) {
        start = clock();
        ntest = modtest(opts->argv[opts->optind]);
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
