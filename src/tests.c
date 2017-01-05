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
test_string_strdel(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    cap_strdel(s);
}

static void
test_string_strnew(void) {
    struct cap_string *s = cap_strnew();
    assert(s != NULL);
    cap_strdel(s);
}

static const struct testcase
stringtests[] = {
    {"strdel", test_string_strdel},
    {"strnew", test_string_strnew},
    {},
};

/*******
* main *
*******/

static const struct testmodule
testmodules[] = {
    {"array", arraytests},
    {"string", stringtests},
    {},
};

static void
run(const struct opts *opts) {
    int ntest = 0;
    clock_t start = clock();

    for (const struct testmodule *m = testmodules; m->name; ++m) {
        for (const struct testcase *t = m->tests; t->name; ++t) {
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
