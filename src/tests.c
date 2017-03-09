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
get_test_fcontent(void) {
    return "1234567\n";
}

static const char *
get_test_fcontent_nonewline(void) {
    return "1234567";
}

static const char *
get_test_finpath(void) {
    static const char *src = "/tmp/cap.test.file";
    static char path[1024];
    assert(cap_fsolve(path, sizeof path, src) != NULL);
    if (!cap_fexists(path)) {
        FILE *f = cap_fopen(path, "w");
        assert(f != NULL);
        fprintf(f, "%s", get_test_fcontent());
        assert(cap_fclose(f) == 0);
    }
    return path;
}

static FILE *
get_test_fin(void) {
    FILE *fin = cap_fopen(get_test_finpath(), "r");
    assert(fin != NULL);
    return fin;
}

static int
get_test_finsize(void) {
    return strlen(get_test_fcontent());
}

static const char *
get_test_dirpath(void) {
    static const char *src = "/tmp";
    static char path[1024];
    assert(cap_fsolve(path, sizeof path, src) != NULL);
    if (!cap_fexists(path)) {
        assert(cap_fmkdirq(path) == 0);
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
    FILE *f = cap_fopen(get_test_finpath(), "r");
    assert(f != NULL);
    assert(cap_fclose(f) == 0);
}

static void
test_file_fclosedir(void) {
    DIR *f = cap_fopendir(get_test_dirpath());
    assert(f != NULL);
    assert(cap_fclosedir(f) == 0);
}

static void
test_file_fopendir(void) {
    test_file_fclosedir();
}

static void
test_file_frealpath(void) {
    char path[1024];
    assert(cap_frealpath(path, sizeof path, "/tmp/../tmp") != NULL);
    assert(strcmp(path, "/tmp") == 0);
}

static void
test_file_fexists(void) {
    assert(cap_fexists("/tmp"));
    assert(!cap_fexists("/nothing/directory"));
}

static void
test_file_fmkdirmode(void) {
    assert(!cap_fexists("/nothing/directory"));
}

static void
test_file_fmkdirq(void) {
    assert(cap_fmkdirq("/tmp") != 0);
}

static void
test_file_ftrunc(void) {
    const char *path = "/tmp/cap.ftrunc";
    assert(!cap_fexists(path));
    assert(cap_ftrunc(path));
    assert(cap_fexists(path));
    assert(remove(path) == 0);
}

static void
test_file_fsolve(void) {
    char path[1024];
    assert(cap_fsolve(path, sizeof path, "/tmp/../tmp"));
    assert(strcmp(path, "/tmp") == 0);
}

static void
test_file_fsolvecp(void) {
    char *path = cap_fsolvecp("/tmp/../tmp");
    assert(path != NULL);
    assert(strcmp(path, "/tmp") == 0);
    free(path);
}

static void
test_file_fsolvefmt(void) {
    char path[1024];
    assert(cap_fsolvefmt(path, sizeof path, "/%s/../%s", "tmp", "tmp"));
    assert(strcmp(path, "/tmp") == 0);
}

static void
test_file_fisdir(void) {
    assert(cap_fisdir("/tmp"));
    assert(!cap_fisdir("/not/found/directory"));
}

static void
test_file_freadcp(void) {
    FILE *fin = cap_fopen(get_test_finpath(), "r");
    assert(fin != NULL);
    char *p = cap_freadcp(fin);
    cap_fclose(fin);
    assert(p != NULL);
    free(p);
}

static void
test_file_fsize(void) {
    FILE *fin = cap_fopen(get_test_finpath(), "r");
    assert(fin != NULL);
    assert(cap_fsize(fin) == get_test_finsize());
    assert(cap_fclose(fin) == 0);
}

static void
test_file_fsuffix(void) {
    const char *suf = cap_fsuffix("/this/is/text/file.txt");
    assert(suf != NULL);
    assert(strcmp(suf, "txt") == 0);
}

static void
test_file_fdirname(void) {
    char name[128];
    assert(cap_fdirname(name, sizeof name, "/dir/name/file") != NULL);
    assert(strcmp(name, "/dir/name") == 0);
}

static void
test_file_fbasename(void) {
    char name[128];
    assert(cap_fbasename(name, sizeof name, "/dir/name/file") != NULL);
    assert(strcmp(name, "file") == 0);
}

static void
test_file_fgetline(void) {
    FILE *fin = get_test_fin();
    assert(fin != NULL);
    char line[1024];
    assert(cap_fgetline(line, sizeof line, fin) != EOF);
    assert(strcmp(get_test_fcontent_nonewline(), line) == 0);
    assert(cap_fclose(fin) == 0);
}

static void
test_file_freadline(void) {
    char line[1024];
    assert(cap_freadline(line, sizeof line, get_test_finpath()) != NULL);
    assert(strcmp(line, get_test_fcontent_nonewline()) == 0);
}

static void
test_file_fwriteline(void) {
    assert(cap_fwriteline(get_test_fcontent_nonewline(), get_test_finpath()));
    test_file_freadline();
}

static void
test_file_dirnodedel(void) {
    struct cap_dir *dir = cap_diropen("/tmp");
    assert(dir != NULL);

    for (struct cap_dirnode *node; (node = cap_dirread(dir)); ) {
        const char *dname = cap_dirnodename(node);
        assert(dname != NULL);
        cap_dirnodedel(node);
    } 

    assert(cap_dirclose(dir) == 0);
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

/******
* env *
******/

static void
test_env_envget(void) {
    char buf[1024];
    assert(cap_envsetf("CAPTEST", "123") == 0);
    assert(cap_envget(buf, sizeof buf, "NOTHING") == NULL);
    assert(cap_envget(buf, sizeof buf, "CAPTEST"));
}

static void
test_env_envset(void) {
    char buf[1024];
    assert(cap_envset("CAPTEST", "123", 1) == 0);
    assert(cap_envget(buf, sizeof buf, "CAPTEST"));
    assert(strcmp(buf, "123") == 0);
}

static void
test_env_envsetf(void) {
    char buf[1024];
    assert(cap_envsetf("CAPTEST", "123") == 0);
    assert(cap_envget(buf, sizeof buf, "CAPTEST"));
    assert(strcmp(buf, "123") == 0);
}

static const struct testcase
envtests[] = {
    {"envget", test_env_envget},
    {"envset", test_env_envset},
    {"envsetf", test_env_envsetf},
    {},
};

/*******
* hash *
*******/

static void
test_hash_hashl(void) {
    // printf("%ld\n", cap_hashl("123"));
    assert(cap_hashl("123") == 586);
}

static const struct testcase
hashtests[] = {
    {"hashl", test_hash_hashl},
    {},
};

/*****
* cl *
*****/

static void
test_cl_cldel(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    cap_cldel(cl);
}

static void
test_cl_clescdel(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    size_t parrlen = cap_cllen(cl);
    char **parr = cap_clescdel(cl);
    assert(parr != NULL);
    freeargv(parrlen, parr);
}

static void
test_cl_clnew(void) {
    // test_cl_cldel
}

static void
test_cl_clresize(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_clcapa(cl) == 4);
    assert(cap_clresize(cl, 8));
    assert(cap_clcapa(cl) == 8);
    cap_cldel(cl);
}

static void
test_cl_clpush(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_cllen(cl) == 0);
    assert(cap_clpush(cl, "123"));
    assert(cap_clpush(cl, "223"));
    assert(cap_clpush(cl, "323"));
    assert(strcmp(cap_clgetc(cl, 1), "223") == 0);
    assert(cap_cllen(cl) == 3);
    cap_cldel(cl);
}

static void
test_cl_clgetc(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_clpush(cl, "123"));
    assert(strcmp(cap_clgetc(cl, 0), "123") == 0);
    cap_cldel(cl);
}

static void
test_cl_clclear(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_clpush(cl, "123"));
    assert(cap_clpush(cl, "223"));
    assert(cap_cllen(cl) == 2);
    cap_clclear(cl);
    assert(cap_cllen(cl) == 0);
    cap_cldel(cl);
}

static void
test_cl_clparsestropts(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_clparsestropts(cl, "cmd -h -ab 123 --help 223", CL_WRAP | CL_ESCAPE));
    assert(strcmp(cap_clgetc(cl, 0), "'cmd'") == 0);
    assert(strcmp(cap_clgetc(cl, 1), "'-h'") == 0);
    assert(strcmp(cap_clgetc(cl, 2), "'-ab'") == 0);
    assert(strcmp(cap_clgetc(cl, 3), "'123'") == 0);
    assert(strcmp(cap_clgetc(cl, 4), "'--help'") == 0);
    assert(strcmp(cap_clgetc(cl, 5), "'223'") == 0);
    cap_cldel(cl);
}

static void
test_cl_clparsestr(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);
    assert(cap_clparsestr(cl, "cmd -h -ab 123 --help 223"));
    assert(strcmp(cap_clgetc(cl, 0), "cmd") == 0);
    assert(strcmp(cap_clgetc(cl, 1), "-h") == 0);
    assert(strcmp(cap_clgetc(cl, 2), "-ab") == 0);
    assert(strcmp(cap_clgetc(cl, 3), "123") == 0);
    assert(strcmp(cap_clgetc(cl, 4), "--help") == 0);
    assert(strcmp(cap_clgetc(cl, 5), "223") == 0);
    cap_cldel(cl);
}

static void
test_cl_clparseargvopts(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);

    cap_cldel(cl);
}

static void
test_cl_clparseargv(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);

    cap_cldel(cl);
}

static void
test_cl_clshow(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);

    cap_cldel(cl);
}

static void
test_cl_cllen(void) {
    struct cap_cl *cl = cap_clnew();
    assert(cl != NULL);

    cap_cldel(cl);
}

static const struct testcase
cltests[] = {
    {"cldel", test_cl_cldel},
    {"clescdel", test_cl_clescdel},
    {"clnew", test_cl_clnew},
    {"clresize", test_cl_clresize},
    {"clgetc", test_cl_clgetc},
    {"clpush", test_cl_clpush},
    {"clclear", test_cl_clclear},
    {"clparsestropts", test_cl_clparsestropts},
    {"clparsestr", test_cl_clparsestr},
    {"clparseargvopts", test_cl_clparseargvopts},
    {"clparseargv", test_cl_clparseargv},
    {"clshow", test_cl_clshow},
    {"cllen", test_cl_cllen},
    {},
};

/********
* error *
********/

static void
test_error__log(void) {
    char buf[BUFSIZ] = {0};
    setbuf(stderr, buf);

    _cap_log("file", 100, "func", "warn", "msg");
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

    cap_error("this is error");
    assert(strcmp(buf, ": This is error.\n") == 0);
    
    setbuf(stderr, NULL);
}

static const struct testcase
errortests[] = {
    {"_log", test_error__log},
    {"die", test_error_die},
    {"error", test_error_error},
    {},
};

/******
* var *
******/

static void
test_var_init(void) {
    const char *vardir = "/tmp/var";
    if (!cap_fexists(vardir)) {
        assert(cap_fmkdirq(vardir) == 0);
    }
    assert(cap_varinit(vardir));
    
    struct cap_dir *d = cap_diropen(vardir);
    assert(d != NULL);

    char line[1024];
    char tmppath[1024];
    for (struct cap_dirnode *n; (n = cap_dirread(d)); ) {
        const char *fname = cap_dirnodename(n);
        if (fname[0] == '.' || strcmp(fname, "..") == 0) {
            cap_dirnodedel(n);
            continue;
        }

        if (strcmp(fname, "home") == 0) {
            assert(cap_fsolvefmt(tmppath, sizeof tmppath, "%s/%s", vardir, fname));
            assert(cap_freadline(line, sizeof line, tmppath));
            assert(strcmp(line, "/tmp") == 0);
        } else if (strcmp(fname, "cd") == 0) {
            assert(cap_fsolvefmt(tmppath, sizeof tmppath, "%s/%s", vardir, fname));
            assert(cap_freadline(line, sizeof line, tmppath));
            assert(strcmp(line, "/tmp") == 0);
        } else {
            assert(0 && "invalid file name");
        }

        cap_dirnodedel(n);
    }

    cap_dirclose(d);
}

static const struct testcase
vartests[] = {
    {"init", test_var_init},
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
        argv[i] = strdup("abc");
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
    assert(strcmp(buf, "abc\n") == 0);

    setbuf(stdout, NULL);
    freeargv(argc, argv);
}

static void
test_util_isoutofhome(void) {
    assert(isoutofhome("/not/found/dir"));
    assert(!isoutofhome(getenv("CAP_VARHOME")));
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
    const char *path = "/tmp/f";
    if (cap_fexists(path)) {
        assert(remove(path) == 0);
    }
    char cmd[1024];
    assert(cap_fsolvefmt(cmd, sizeof cmd, "/bin/sh -c \"touch %s\"", path));
    assert(safesystem(cmd) == 0);
    assert(cap_fexists(path));
}

static const struct testcase
utiltests[] = {
    {"test_util_freeargv", test_util_freeargv},
    {"test_util_showargv", test_util_showargv},
    {"test_util_isoutofhome", test_util_isoutofhome},
    {"test_util_randrange", test_util_randrange},
    {"test_util_safesystem", test_util_safesystem},
    {},
};

/*********
* socket *
*********/

static void
test_socket_sockshow(void) {
    struct cap_socket *s = cap_sockopen("localhost:80", "tcp-client");
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
    struct cap_socket *s = cap_sockopen("localhost:80", "tcp-client");
    assert(s != NULL);
    assert(strcmp(cap_sockhost(s), "localhost") == 0);
    assert(cap_sockclose(s) == 0);
}

static void
test_socket_sockport(void) {
    struct cap_socket *s = cap_sockopen("localhost:80", "tcp-client");
    assert(s != NULL);
    assert(strcmp(cap_sockport(s), "80") == 0);
    assert(cap_sockclose(s) == 0);
}

static void
test_socket_sockaccept(void) {
}

static void
test_socket_sockrecvstr(void) {
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

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    {"array", arraytests},
    {"string", stringtests},
    {"file", filetests},
    {"env", envtests},
    {"hash", hashtests},
    {"cl", cltests},
    {"error", errortests},
    {"var", vartests},
    {"util", utiltests},
    {"socket", sockettests},
    {},
};

static void
run(const struct opts *opts) {
    int ntest = 0;
    clock_t start = clock();

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        printf("module '%s'\n", m->name);
        for (const struct testcase *t = m->tests; t->name; ++t) {
            printf("testing '%s'\n", t->name);
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
