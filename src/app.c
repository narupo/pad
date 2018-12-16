/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "app.h"

/**
 * Program option values for command.
 *
 */
struct opts {
    bool ishelp;
    bool isversion;
};

struct app {
    int argc;
    char **argv;
    struct opts opts;
    struct env *env;
};

static bool
app_parse_opts(struct app *self) {
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {},
    };

    self->opts = (struct opts){0};
    optind = 0;
    opterr = 0;
    
    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hV", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->opts.ishelp = true; break;
        case 'V': self->opts.isversion = true; break;
        case '?':
        default:
            cap_error("invalid option");
            return false; break;
        }
    }

    if (self->argc < optind) {
        return false;
    }

    return true;
}

static void
app_del(struct app *self) {
    if (self) {
        env_del(self->env);
        free(self);
    }
}

static void
app_init_env(struct app *self) {
    if (self->env) {
        env_del(self->env);
    }

    self->env = env_new();
    env_set(self->env, "CAP_VERSION", "0.22.0");
}

static struct app *
app_new(int argc, char *argv[]) {
    struct app *self = cap_ecalloc(1, sizeof(*self));

    self->argc = argc;
    self->argv = argv;

    app_init_env(self);

    if (!app_parse_opts(self)) {
        app_del(self);
        return NULL;
    }

    return self;
}

static void
app_usage(struct app *app) {
    static const char usage[] =
        "Cap is simple snippet manager.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap [options] [command] [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -V, --version    show version.\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    home     change home directory, or show.\n"
        "    cd       change current directory by relative of home.\n"
        "    pwd      show current directory.\n"
        "    ls       show list in current directory.\n"
        "    cat      catenate files and show.\n"
        "    run      run script.\n"
        "    alias    run alias command.\n"
    ;
    static const char *examples[] = {
        "    $ cap home\n"
        "    $ cap pwd\n"
        "    $ cap ls\n"
        ,
        "    $ cap alias dog \"cat animal/dog\"\n"
        "    $ cap dog\n"
        ,
        "    $ cap cat path/to/code/file.c\n"
        "    $ cap ls path/to/code\n"
        ,
    };
    const int exmlen = sizeof(examples)/sizeof(*examples);
    const char *example = NULL;

    srand(time(NULL));
    example = examples[randrange(0, exmlen-1)];

    fprintf(stderr,
        "%s\n"
        "Examples:\n\n"
        "%s\n"
    , usage, example);
    
    app_del(app);
    exit(0);
}

static void
app_version(struct app *self) {
    fflush(stdout);
    fflush(stderr);

    printf("%s\n", env_get(self->env, "CAP_VERSION"));
    fflush(stdout);

    app_del(self);
    exit(0);
}

static int
app_run(struct app *self) {
    if (self->opts.ishelp) {
        app_usage(self);
    }

    if (self->opts.isversion) {
        app_version(self);
    }

    if (self->argc < 2) {
        app_usage(self);
    }

    return 0;
}

int
main(int argc, char *argv[]) {
    struct app *app = app_new(argc, argv);
    if (!app) {
        cap_die("failed to start application");
    }

    int result = app_run(app);
    app_del(app);
    
    return result;
}
