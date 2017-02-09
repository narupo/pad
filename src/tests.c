/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2017
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

/*******
* opts *
*******/

struct opts {
    bool ishelp;
};

static int
parseopts(struct opts *opts, int argc, char *argv[]) {
    // Init opts
    *opts = (struct opts) {};

    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        // {"fname", required_argument, 0, 'f'},
        {},
    };
    optind = 0;

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
    
    return 0;
}

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
test_array_arrnew(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);
    cap_arrdel(arr);
}

void
test_array_arrescdel(void) {
    // test
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrescdel(NULL) == NULL);

    char **escarr = cap_arrescdel(arr);
    assert(escarr != NULL);

    int i;
    for (i = 0; escarr[i]; ++i) {
    }
    assert(i == 0);
    _freeescarr(escarr);

    // test
    arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrpush(arr, "0") != NULL);
    assert(cap_arrpush(arr, "1") != NULL);
    assert(cap_arrpush(arr, "2") != NULL);

    escarr = cap_arrescdel(arr);
    assert(_countescarr(escarr) == 3);
    assert(strcmp(escarr[0], "0") == 0);
    assert(strcmp(escarr[1], "1") == 0);
    assert(strcmp(escarr[2], "2") == 0);
    _freeescarr(escarr);
}

void
test_array_arrpush(void) {
    // test
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrpush(NULL, "1") == NULL);
    assert(cap_arrpush(arr, NULL) == NULL);
    assert(cap_arrpush(arr, "") != NULL);
    assert(cap_arrpush(arr, "1") != NULL);

    assert(cap_arrlen(arr) == 2);

    cap_arrdel(arr);
}

void
test_array_arrmove(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrmove(arr, NULL) != NULL);
    assert(cap_arrgetc(arr, 0) == NULL);

    char *ptr = strdup("string"); 
    assert(ptr != NULL);
    
    assert(cap_arrmove(arr, ptr) != NULL);
    assert(strcmp(cap_arrgetc(arr, 1), "string") == 0);

    cap_arrdel(arr);
}

void
test_array_arrsort(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrsort(NULL) == NULL);

    assert(cap_arrpush(arr, "1") != NULL);
    assert(cap_arrpush(arr, "2") != NULL);
    assert(cap_arrpush(arr, "0") != NULL);

    assert(cap_arrsort(arr) != NULL);
    assert(strcmp(cap_arrgetc(arr, 0), "0") == 0);
    assert(strcmp(cap_arrgetc(arr, 1), "1") == 0);
    assert(strcmp(cap_arrgetc(arr, 2), "2") == 0);

    cap_arrdel(arr);
}

void
test_array_arrgetc(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrgetc(NULL, 0) == NULL);
    assert(cap_arrgetc(arr, 0) == NULL);
    assert(cap_arrgetc(arr, -1) == NULL);

    assert(cap_arrpush(arr, "0") != NULL);
    assert(cap_arrpush(arr, "1") != NULL);
    assert(cap_arrpush(arr, "2") != NULL);

    assert(strcmp(cap_arrgetc(arr, 0), "0") == 0);
    assert(strcmp(cap_arrgetc(arr, 1), "1") == 0);
    assert(strcmp(cap_arrgetc(arr, 2), "2") == 0);
    assert(cap_arrgetc(arr, 3) == NULL);

    cap_arrdel(arr);
}

void
test_array_arrlen(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrlen(NULL) == 0);
    assert(cap_arrlen(arr) == 0);

    assert(cap_arrpush(arr, "0") != NULL);
    assert(cap_arrpush(arr, "1") != NULL);
    assert(cap_arrpush(arr, "2") != NULL);
    assert(cap_arrlen(arr) == 3);

    cap_arrdel(arr);
}

void
test_array_arrshow(void) {
    struct cap_array *arr = cap_arrnew();
    assert(arr != NULL);

    assert(cap_arrshow(NULL, stdout) == NULL);
    assert(cap_arrshow(arr, NULL) == NULL);
    assert(cap_arrshow(arr, stdout) != NULL);

    cap_arrdel(arr);
}

static const struct testcase
arraytests[] = {
    {"arrnew", test_array_arrnew},
    {"arrescdel", test_array_arrescdel},
    {"arrpush", test_array_arrpush},
    {"arrmove", test_array_arrmove},
    {"arrsort", test_array_arrsort},
    {"arrgetc", test_array_arrgetc},
    {"arrlen", test_array_arrlen},
    {"arrshow", test_array_arrshow},
    {},
};

/*********
* string *
*********/

static void
test_string_capstrncat(void) {
    char dst[100] = {};

    assert(capstrncat(dst, sizeof dst, NULL) == NULL);
    assert(capstrncat(NULL, sizeof dst, "source") == NULL);
    assert(capstrncat(dst, 0, "source") == NULL);

    assert(capstrncat(dst, 3, "source") != NULL);
    assert(strcmp(dst, "so") == 0);

    *dst = '\0';
    assert(capstrncat(dst, sizeof dst, "source") != NULL);
    assert(strcmp(dst, "source") == 0);
    assert(capstrncat(dst, sizeof dst, " is available.") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
    assert(capstrncat(dst, sizeof dst, "") != NULL);
    assert(strcmp(dst, "source is available.") == 0);
}

static void
test_string_capstrcpywithout(void) {
    char dst[100];

    assert(capstrcpywithout(NULL, sizeof dst, "abc123def456", "") == NULL);
    assert(capstrcpywithout(dst, 0, "abc123def456", "") == NULL);
    assert(capstrcpywithout(dst, sizeof dst, NULL, "") == NULL);
    assert(capstrcpywithout(dst, sizeof dst, "abc123def456", NULL) == NULL);

    assert(capstrcpywithout(dst, sizeof dst, "abc123def456", "") != NULL);
    assert(strcmp(dst, "abc123def456") == 0);
    assert(capstrcpywithout(dst, sizeof dst, "abc123def456", "123456") != NULL);
    assert(strcmp(dst, "abcdef") == 0);
    assert(capstrcpywithout(dst, sizeof dst, "abc123def456", "abcdef") != NULL);
    assert(strcmp(dst, "123456") == 0);
}

static void
test_string_strdel(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    cap_strdel(s);
}

static void
test_string_strescdel(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    char *ptr = cap_strescdel(s);
    assert(ptr != NULL);
    free(ptr);
}

static void
test_string_strnew(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    cap_strdel(s);
}

static void
test_string_strnewother(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    struct cap_string *o = cap_strnewother(s);
    assert(o != NULL);
    assert(strcmp(cap_strgetc(o), "1234") == 0);
    cap_strdel(o);
    cap_strdel(s);
}

static void
test_string_strlen(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strlen(s) == 0);
    assert(cap_strapp(s, "abc") != NULL);
    assert(cap_strlen(s) == 3);
    cap_strdel(s);
}

static void
test_string_strcapa(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strcapa(s) == 4);
    assert(cap_strapp(s, "1234") != NULL);
    assert(cap_strcapa(s) == 8);
    cap_strdel(s);
}

static void
test_string_strgetc(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(strcmp(cap_strgetc(s), "") == 0);
    assert(cap_strapp(s, "1234") != NULL);
    assert(strcmp(cap_strgetc(s), "1234") == 0);
    cap_strdel(s);
}

static void
test_string_strempty(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strempty(s) == true);
    assert(cap_strapp(s, "1234") != NULL);
    assert(cap_strempty(s) == false);
    cap_strdel(s);
}

static void
test_string_strclear(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strapp(s, "1234") != NULL);
    assert(cap_strlen(s) == 4);
    cap_strclear(s);
    assert(cap_strlen(s) == 0);
    cap_strdel(s);
}

static void
test_string_strset(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    assert(strcmp(cap_strgetc(s), "1234") == 0);
    assert(cap_strset(s, "12") != NULL);
    assert(strcmp(cap_strgetc(s), "12") == 0);
    cap_strdel(s);
}

static void
test_string_strresize(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strcapa(s) == 4);
    assert(cap_strresize(s, 4*2) != NULL);
    assert(cap_strcapa(s) == 8);
    cap_strdel(s);
}

static void
test_string_strpushb(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strpushb(s, '1') != NULL);
    assert(cap_strpushb(s, '2') != NULL);
    assert(strcmp(cap_strgetc(s), "12") == 0);
    cap_strdel(s);
}

static void
test_string_strpopb(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    assert(strcmp(cap_strgetc(s), "1234") == 0);
    assert(cap_strpopb(s) == '4');
    assert(cap_strpopb(s) == '3');
    assert(strcmp(cap_strgetc(s), "12") == 0);
    cap_strdel(s);
}

static void
test_string_strpushf(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strpushf(s, '1') != NULL);
    assert(cap_strpushf(s, '2') != NULL);
    assert(strcmp(cap_strgetc(s), "21") == 0);
    cap_strdel(s);
}

static void
test_string_strpopf(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    assert(cap_strpopf(s) == '1');
    assert(cap_strpopf(s) == '2');
    assert(strcmp(cap_strgetc(s), "34") == 0);
    cap_strdel(s);
}

static void
test_string_strapp(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strapp(s, "1234") != NULL);
    assert(strcmp(cap_strgetc(s), "1234") == 0);
    cap_strdel(s);
}

static void
test_string_strappstream(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);

    char path[1024];
    assert(cap_frealpath(path, sizeof path, ".") != NULL);
    assert(capstrncat(path, sizeof path, "/tests.c") != NULL);
    // printf("path[%s]\n", path);

    FILE *fin = fopen(path, "r");
    assert(fin != NULL);
    assert(cap_strappstream(s, fin) != NULL);
    assert(fclose(fin) == 0);

    cap_strdel(s);
}

static void
test_string_strappother(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    struct cap_string *o = cap_strnew();
    assert(o != NULL);
    assert(cap_strset(o, "1234") != NULL);
    assert(cap_strappother(s, o) != NULL);
    assert(strcmp(cap_strgetc(s), "12341234") == 0);
    cap_strdel(o);
    cap_strdel(s);
}

static void
test_string_strappfmt(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    char buf[1024];
    assert(cap_strappfmt(s, buf, sizeof buf, "%s %d %c", "1234", 1, '2') != NULL);
    assert(strcmp(cap_strgetc(s), "1234 1 2") == 0);
    cap_strdel(s);
}

static void
test_string_strrstrip(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    assert(cap_strrstrip(s, "34") != NULL);
    assert(strcmp(cap_strgetc(s), "12") == 0);
    cap_strdel(s);
}

static void
test_string_strlstrip(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    assert(cap_strlstrip(s, "12") != NULL);
    assert(strcmp(cap_strgetc(s), "34") == 0);
    cap_strdel(s);
}

static void
test_string_strstrip(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "--1234--") != NULL);
    assert(cap_strstrip(s, "-") != NULL);
    assert(strcmp(cap_strgetc(s), "1234") == 0);
    cap_strdel(s);
}

static void
test_string_strfindc(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    assert(cap_strset(s, "1234") != NULL);
    const char *fnd = cap_strfindc(s, "23");
    // printf("fnd[%s]\n", fnd);
    assert(fnd != NULL);
    assert(strcmp(fnd, "234") == 0);
    cap_strdel(s);
}

static const struct testcase
stringtests[] = {
    {"capstrncat", test_string_capstrncat},
    {"capstrcpywithout", test_string_capstrcpywithout},
    {"strdel", test_string_strdel},
    {"strescdel", test_string_strescdel},
    {"strnew", test_string_strnew},
    {"strnewother", test_string_strnewother},
    {"strlen", test_string_strlen},
    {"strcapa", test_string_strcapa},
    {"strgetc", test_string_strgetc},
    {"strempty", test_string_strempty},
    {"strclear", test_string_strclear},
    {"strset", test_string_strset},
    {"strresize", test_string_strresize},
    {"strpushb", test_string_strpushb},
    {"strpopb", test_string_strpopb},
    {"strpushf", test_string_strpushf},
    {"strpopf", test_string_strpopf},
    {"strapp", test_string_strapp},
    {"strappstream", test_string_strappstream},
    {"strappother", test_string_strappother},
    {"strappfmt", test_string_strappfmt},
    {"strrstrip", test_string_strrstrip},
    {"strlstrip", test_string_strlstrip},
    {"strstrip", test_string_strstrip},
    {"strfindc", test_string_strfindc},
    {},
};

/*******
* file *
*******/

static const char *
get_test_finpath(void) {
    static const char *src = "/tmp/cap.test.file";
    static char path[1024];
    assert(cap_fsolve(path, sizeof path, src) != NULL);
    if (!cap_fexists(path)) {
        FILE *f = cap_fopen(path, "w");
        assert(f != NULL);
        fprintf(f, "123abc456def\n");
        fprintf(f, "123abc456def\n");
        fprintf(f, "123abc456def\n");
        assert(cap_fclose(f) == 0);
    }
    return path;
}

static void
test_file_fclose(void) {
    FILE* f = cap_fopen(get_test_finpath(), "r");
    assert(f != NULL);
    assert(cap_fclose(f) == 0);
}

static void
test_file_fopen(void) {
    test_file_fclose();
}

static void
test_file_fcopy(void) {
    FILE* f = cap_fopen(get_test_finpath(), "r");
    assert(f != NULL);
    assert(cap_fclose(f) == 0);
}

static void
test_file_fclosedir(void) {
}

static void
test_file_fopendir(void) {
}

static void
test_file_frealpath(void) {
}

static void
test_file_fexists(void) {
}

static void
test_file_fmkdirmode(void) {
}

static void
test_file_fmkdirq(void) {
}

static void
test_file_ftrunc(void) {
}

static void
test_file_fsolve(void) {
}

static void
test_file_fsolvecp(void) {
}

static void
test_file_fsolvefmt(void) {
}

static void
test_file_fisdir(void) {
}

static void
test_file_freadcp(void) {
}

static void
test_file_fsize(void) {
}

static void
test_file_fsuffix(void) {
}

static void
test_file_fdirname(void) {
}

static void
test_file_fbasename(void) {
}

static void
test_file_fgetline(void) {
}

static void
test_file_freadline(void) {
}

static void
test_file_fwriteline(void) {
}

static void
test_file_dirnodedel(void) {
}

static void
test_file_dirnodename(void) {
}

static void
test_file_dirclose(void) {
}

static void
test_file_diropen(void) {
}

static void
test_file_dirread(void) {
}

static const struct testcase
filetests[] = {
    {"fclose", test_file_fclose},
    {"fopen", test_file_fopen},
    {"fcopy", test_file_fcopy},
    {"fclosedir", test_file_fclosedir},
    {"fopendir", test_file_fopendir},
    {"frealpath", test_file_frealpath},
    {"fexists", test_file_fexists},
    {"fmkdirmode", test_file_fmkdirmode},
    {"fmkdirq", test_file_fmkdirq},
    {"ftrunc", test_file_ftrunc},
    {"fsolve", test_file_fsolve},
    {"fsolvecp", test_file_fsolvecp},
    {"fsolvefmt", test_file_fsolvefmt},
    {"fisdir", test_file_fisdir},
    {"freadcp", test_file_freadcp},
    {"fsize", test_file_fsize},
    {"fsuffix", test_file_fsuffix},
    {"fdirname", test_file_fdirname},
    {"fbasename", test_file_fbasename},
    {"fgetline", test_file_fgetline},
    {"freadline", test_file_freadline},
    {"fwriteline", test_file_fwriteline},
    {"dirnodedel", test_file_dirnodedel},
    {"dirnodename", test_file_dirnodename},
    {"dirclose", test_file_dirclose},
    {"diropen", test_file_diropen},
    {"dirread", test_file_dirread},
    {},
};

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    {"array", arraytests},
    {"string", stringtests},
    {"file", filetests},
    {},
};

static void
run(const struct opts *opts) {
    int ntest = 0;
    clock_t start = clock();

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        // printf("module '%s'\n", m->name);
        for (const struct testcase *t = m->tests; t->name; ++t) {
            // printf("testing '%s'\n", t->name);
            t->test();
            ++ntest;
        }
    }
    
    clock_t end = clock();

    fflush(stdout);

    fprintf(stderr, "Run %d test in %0.3lfs.\n", ntest, (double)(end-start)/CLOCKS_PER_SEC);
    fprintf(stderr, "\n");
    fprintf(stderr, "OK\n");
    fflush(stderr);
}

int
main(int argc, char *argv[]) {
    struct opts opts;
    if (parseopts(&opts, argc, argv) != 0) {
        die("failed to parse options");
    }

    run(&opts);

    return 0;
}
