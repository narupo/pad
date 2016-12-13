/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "tests.h"

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

/********
* tests *
********/

static struct testcase {
    const char *module;
    const char *func;
    void (*test)(void);
} testcases[] = {
    {"array", "arrnew", test_array_arrnew},
    {"array", "arrescdel", test_array_arrescdel},
    {"array", "arrpush", test_array_arrpush},
    {},
};

/*******
* main *
*******/

static void
run(const struct opts *opts) {
    int ntest = 0;
    clock_t start = clock();

    for (const struct testcase *p = testcases; p->module; ++p) {
        p->test();
        ++ntest;
    }
    
    clock_t end = clock();

    printf("Run %d test in %0.3lfs.\n", ntest, (double)(end-start)/CLOCKS_PER_SEC);
    printf("\n");
    printf("OK\n");

    fflush(stderr);
    fflush(stdout);
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
