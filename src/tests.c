/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "tests.h"

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
    assert(file_solvefmt(path, sizeof path, "%s/cap.ftrunc") != NULL);

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
    char buf[BUFSIZ] = {0};
    setbuf(stdout, buf);

    int argc = 2;
    char **argv = __create_testargv(argc);
    assert(argv != NULL);

    showargv(argc, argv);
    assert(strcmp(buf, "abc\nabc\n") == 0);

    setbuf(stdout, NULL);
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
    tokenizer_t *tkr = tkr_new();
    tkr_del(tkr);
}

static void
test_tkr_parse(void) {
    tokenizer_t *tkr = tkr_new();
    const token_t *token;

    tkr_parse(tkr, "abc");
    assert(tkr_tokens_len(tkr) == 1);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_TEXT_BLOCK);
    assert(strcmp(token_getc_text(token), "abc") == 0);

    tkr_parse(tkr, "abc{@@}bbc");
    assert(tkr_tokens_len(tkr) == 4);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_TEXT_BLOCK);
    assert(strcmp(token_getc_text(token), "abc") == 0);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_TEXT_BLOCK);
    assert(strcmp(token_getc_text(token), "bbc") == 0);

    // test of realloc of tokens
    tkr_parse(tkr, "{@......@}");
    assert(tkr_tokens_len(tkr) == 8);

    tkr_parse(tkr, "");
    assert(tkr_has_error(tkr) == false);
    assert(tkr_tokens_len(tkr) == 0);

    tkr_parse(tkr, "{@{");
    assert(tkr_tokens_len(tkr) == 1);
    assert(tkr_has_error(tkr) == true);
    assert(strcmp(tkr_get_error_detail(tkr), "syntax error. unsupported character \"{\"") == 0);
    
    tkr_parse(tkr, "{@");
    assert(tkr_tokens_len(tkr) == 1);
    assert(tkr_has_error(tkr) == true);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);

    tkr_parse(tkr, "{@@");
    assert(tkr_has_error(tkr) == true);
    assert(strcmp(tkr_get_error_detail(tkr), "invalid syntax. single '@' is not supported") == 0);

    tkr_parse(tkr, "{@@}");
    assert(tkr_tokens_len(tkr) == 2);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@.@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_DOT_OPE);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@..@}");
    assert(tkr_tokens_len(tkr) == 4);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_DOT_OPE);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_DOT_OPE);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@,@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_COMMA);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@,,@}");
    assert(tkr_tokens_len(tkr) == 4);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_COMMA);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_COMMA);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@(@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_LPAREN);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@)@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_RPAREN);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@()@}");
    assert(tkr_tokens_len(tkr) == 4);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_LPAREN);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RPAREN);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@a@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "a") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@abc@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "abc") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@abc123@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "abc123") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@abc_123@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "abc_123") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    // TODO: fix to digit
    tkr_parse(tkr, "{@123@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "123") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@\"\"@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@\"abc\"@}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "abc") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@\"abc\"\"bbc\"@}");
    assert(tkr_tokens_len(tkr) == 4);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "abc") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "bbc") == 0);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{@ import alias\n"
        "alias.set(\"dtl\", \"run bin/date-line\") @}");
    assert(tkr_tokens_len(tkr) == 12);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LBRACEAT);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "import") == 0);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "alias") == 0);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "alias") == 0);
    token = tkr_tokens_getc(tkr, 4);
    assert(token_get_type(token) == TOKEN_TYPE_DOT_OPE);
    token = tkr_tokens_getc(tkr, 5);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    assert(strcmp(token_getc_text(token), "set") == 0);
    token = tkr_tokens_getc(tkr, 6);
    assert(token_get_type(token) == TOKEN_TYPE_LPAREN);
    token = tkr_tokens_getc(tkr, 7);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "dtl") == 0);
    token = tkr_tokens_getc(tkr, 8);
    assert(token_get_type(token) == TOKEN_TYPE_COMMA);
    token = tkr_tokens_getc(tkr, 9);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    assert(strcmp(token_getc_text(token), "run bin/date-line") == 0);
    token = tkr_tokens_getc(tkr, 10);
    assert(token_get_type(token) == TOKEN_TYPE_RPAREN);
    token = tkr_tokens_getc(tkr, 11);
    assert(token_get_type(token) == TOKEN_TYPE_RBRACEAT);

    tkr_parse(tkr, "{{");
    assert(tkr_tokens_len(tkr) == 1);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    assert(tkr_has_error(tkr) == true);
    assert(strcmp(tkr_get_error_detail(tkr), "not closed by block") == 0);

    tkr_parse(tkr, "{{}}");
    assert(tkr_tokens_len(tkr) == 2);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

    tkr_parse(tkr, "{{\n}}");
    assert(tkr_tokens_len(tkr) == 1);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    assert(tkr_has_error(tkr) == true);
    assert(strcmp(tkr_get_error_detail(tkr), "unsupported to newline") == 0);

    tkr_parse(tkr, "{{abc}}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

    tkr_parse(tkr, "{{abc123}}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

    tkr_parse(tkr, "{{abc_123}}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

    // TODO: fix to digit
    tkr_parse(tkr, "{{ 123 }}");
    assert(tkr_tokens_len(tkr) == 3);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

    tkr_parse(tkr, "{{ alias.run(\"dtl\") }}");
    assert(tkr_tokens_len(tkr) == 8);
    token = tkr_tokens_getc(tkr, 0);
    assert(token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE);
    token = tkr_tokens_getc(tkr, 1);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 2);
    assert(token_get_type(token) == TOKEN_TYPE_DOT_OPE);
    token = tkr_tokens_getc(tkr, 3);
    assert(token_get_type(token) == TOKEN_TYPE_IDENTIFIER);
    token = tkr_tokens_getc(tkr, 4);
    assert(token_get_type(token) == TOKEN_TYPE_LPAREN);
    token = tkr_tokens_getc(tkr, 5);
    assert(token_get_type(token) == TOKEN_TYPE_DQ_STRING);
    token = tkr_tokens_getc(tkr, 6);
    assert(token_get_type(token) == TOKEN_TYPE_RPAREN);
    token = tkr_tokens_getc(tkr, 7);
    assert(token_get_type(token) == TOKEN_TYPE_RDOUBLE_BRACE);

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
    tokenizer_t *tkr = tkr_new();
    ast_t *ast = ast_new();
    const node_t *root;
    const bin_node_t *bin, *bin2;
    const code_block_node_t *cbn;
    const text_block_node_t *tbn;
    const formula_node_t *fn, *fn2;
    const caller_node_t *cn;
    const import_node_t *in;

    tkr_parse(tkr, "{@@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    assert(code_block_node_getc_formula(cbn) == NULL);

    tkr_parse(tkr, "abc");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_TEXT_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    tbn = node_getc_real(bin_node_getc_lhs(bin));
    assert(strcmp(text_block_node_getc_text(tbn), "abc") == 0);

    tkr_parse(tkr, "{@\nfunc()\n@}");
    // ast_set_debug(ast, true);
    ast_parse(ast, tkr_get_tokens(tkr));
    test_ast_show_error(ast);
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_CALLER);
    cn = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(caller_node_identifiers_getc(cn, 0), "func") == 0);

    tkr_parse(tkr, "{@\nfunc(\n@}");
    // ast_set_debug(ast, true);
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "syntax error. not supported token 21 in caller") == 0);
    // ast_set_debug(ast, false);

    tkr_parse(tkr, "{@\nfunc(\"aaa\",\n@}");
    // ast_set_debug(ast, true);
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "syntax error. not supported token 21 in caller") == 0);

    tkr_parse(tkr, "{@\nalias.set()\n@}");
    // ast_set_debug(ast, true);
    ast_parse(ast, tkr_get_tokens(tkr));
    test_ast_show_error(ast);
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_CALLER);
    cn = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(caller_node_identifiers_getc(cn, 0), "alias") == 0);
    assert(strcmp(caller_node_identifiers_getc(cn, 1), "set") == 0);

    tkr_parse(tkr, "{@\nalias.set(\"dtl\", \"run bin/date-line\")\n@}");
    // ast_set_debug(ast, true);
    ast_parse(ast, tkr_get_tokens(tkr));
    test_ast_show_error(ast);
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_CALLER);
    cn = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(caller_node_identifiers_getc(cn, 0), "alias") == 0);
    assert(strcmp(caller_node_identifiers_getc(cn, 1), "set") == 0);
    assert(strcmp(caller_node_args_getc(cn, 0), "dtl") == 0);
    assert(strcmp(caller_node_args_getc(cn, 1), "run bin/date-line") == 0);

    tkr_parse(tkr, "{@\nalias.get(\"dtl\")\n@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);

    tkr_parse(tkr, "{@\nimport alias\n@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);    
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "alias") == 0);

    tkr_parse(tkr, "{@\nimport\n@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "syntax error. invalid token in import") == 0);

    tkr_parse(tkr, "{@\nimport aaa\n@}{@\nimport bbb\n@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_BIN);    
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "aaa") == 0);
    bin2 = node_getc_real(bin_node_getc_rhs(bin));
    assert(node_getc_type(bin_node_getc_lhs(bin2)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin2)) == NODE_TYPE_INVALID);
    cbn = node_getc_real(bin_node_getc_lhs(bin2));
    assert(code_block_node_getc_formula(cbn) != NULL);
    assert(node_getc_type(code_block_node_getc_formula(cbn)) == NODE_TYPE_FORMULA);
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "bbb") == 0);

    tkr_parse(tkr, "aaa{@\nimport bbb\n@}ccc{@\nimport ddd\n@}eee");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_TEXT_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_BIN);
    tbn = node_getc_real(bin_node_getc_lhs(bin));
    assert(strcmp(text_block_node_getc_text(tbn), "aaa") == 0);
    bin2 = node_getc_real(bin_node_getc_rhs(bin));
    assert(node_getc_type(bin_node_getc_lhs(bin2)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin2)) == NODE_TYPE_BIN);
    cbn = node_getc_real(bin_node_getc_lhs(bin2));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "bbb") == 0);
    bin = node_getc_real(bin_node_getc_rhs(bin2));
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_TEXT_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_BIN);
    tbn = node_getc_real(bin_node_getc_lhs(bin));
    assert(strcmp(text_block_node_getc_text(tbn), "ccc") == 0);
    bin2 = node_getc_real(bin_node_getc_rhs(bin));
    assert(node_getc_type(bin_node_getc_lhs(bin2)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin2)) == NODE_TYPE_BIN);
    cbn = node_getc_real(bin_node_getc_lhs(bin2));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "ddd") == 0);
    bin = node_getc_real(bin_node_getc_rhs(bin2));
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_TEXT_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);
    tbn = node_getc_real(bin_node_getc_lhs(bin));
    assert(strcmp(text_block_node_getc_text(tbn), "eee") == 0);

    tkr_parse(tkr, "{@\nimport alias\nalias.set(\"dtl\", \"run bin/date-line\") @}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    root = ast_getc_root(ast);
    assert(node_getc_type(root) == NODE_TYPE_BIN);
    bin = node_getc_real(root);
    assert(node_getc_type(bin_node_getc_lhs(bin)) == NODE_TYPE_CODE_BLOCK);
    assert(node_getc_type(bin_node_getc_rhs(bin)) == NODE_TYPE_INVALID);    
    cbn = node_getc_real(bin_node_getc_lhs(bin));
    fn = node_getc_real(code_block_node_getc_formula(cbn));
    assert(fn != NULL);
    assert(node_getc_type(formula_node_getc_lhs(fn)) == NODE_TYPE_IMPORT);
    in = node_getc_real(formula_node_getc_lhs(fn));
    assert(strcmp(import_node_getc_package(in), "alias") == 0);
    assert(node_getc_type(formula_node_getc_rhs(fn)) == NODE_TYPE_FORMULA);
    fn2 = node_getc_real(formula_node_getc_rhs(fn));
    assert(node_getc_type(formula_node_getc_lhs(fn2)) == NODE_TYPE_CALLER);
    cn = node_getc_real(formula_node_getc_lhs(fn2));
    assert(strcmp(caller_node_identifiers_getc(cn, 0), "alias") == 0);
    assert(strcmp(caller_node_identifiers_getc(cn, 1), "set") == 0);
    assert(strcmp(caller_node_args_getc(cn, 0), "dtl") == 0);
    assert(strcmp(caller_node_args_getc(cn, 1), "run bin/date-line") == 0);
}

static void
test_ast_parse_context(void) {
    tokenizer_t *tkr = tkr_new();
    ast_t *ast = ast_new();
    context_t *ctx = ctx_new();

    tkr_parse(tkr, "{@\nimport alias\nalias.set(\"dtl\", \"run bin/date-line\") @}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    ast_traverse(ast, ctx);
    assert(strcmp(ctx_get_alias_value(ctx, "dtl"), "run bin/date-line") == 0);
    assert(ctx_get_imported_alias(ctx) == true);
    ctx_clear(ctx);

    tkr_parse(tkr, "{@\nalias.set(\"dtl\", \"run bin/date-line\") @}");
    ast_parse(ast, tkr_get_tokens(tkr));
    ast_traverse(ast, ctx);
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "import error. alias is not imported") == 0);
    ctx_clear(ctx);

    tkr_parse(tkr, "{@\nimport alias\nalias.set() @}");
    ast_parse(ast, tkr_get_tokens(tkr));
    ast_traverse(ast, ctx);
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "invalid argument. set method of alias need two arguments") == 0);
    ctx_clear(ctx);

    tkr_parse(tkr, "{@\nimport alias\nalias() @}");
    ast_parse(ast, tkr_get_tokens(tkr));
    ast_traverse(ast, ctx);
    assert(ast_has_error(ast) == true);
    assert(strcmp(ast_get_error_detail(ast), "call error. alias is not callable") == 0);
    ctx_clear(ctx);

    tkr_parse(tkr, "{@\n"
        "import alias\n"
        "alias.set(\"dtl\", \"run bin/date-line\")\n"
        "alias.set(\"aaa\", \"bbb\")\n"
        "@}");
    ast_parse(ast, tkr_get_tokens(tkr));
    assert(ast_has_error(ast) == false);
    ast_traverse(ast, ctx);
    assert(strcmp(ctx_get_alias_value(ctx, "dtl"), "run bin/date-line") == 0);
    assert(strcmp(ctx_get_alias_value(ctx, "aaa"), "bbb") == 0);
    assert(ctx_get_imported_alias(ctx) == true);
    ctx_clear(ctx);

    ctx_del(ctx);
    ast_del(ast);
    tkr_del(tkr);
}

static const struct testcase
ast_tests[] = {
    {"ast_parse", test_ast_parse},
    {"ast_parse_context", test_ast_parse_context},
    {0},
};

/**********
* context *
**********/

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
    // {"socket", sockettests},
    // {"url", urltests},
    {"tokenizer", tokenizer_tests},
    {"ast", ast_tests},
    {},
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
