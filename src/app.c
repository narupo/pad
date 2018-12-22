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
    cmdargs_t *cmdargs;
    config_t *config;
    struct opts opts;
};

typedef struct app app_t;

static bool
app_parse_opts(app_t *self) {
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
            err_error("invalid option");
            return false; break;
        }
    }

    if (self->argc < optind) {
        return false;
    }

    return true;
}

static void
app_del(app_t *self) {
    if (self) {
        config_del(self->config);
        cmdargs_del(self->cmdargs);
        free(self);
    }
}

static bool
app_init_config(app_t *self) {
    // In case of UNIX/Linux
    file_solve(self->config->var_cd_path, sizeof self->config->var_cd_path, "~/.cap/var/cd");
    file_solve(self->config->var_home_path, sizeof self->config->var_home_path, "~/.cap/var/home");

    // In case of Windows
    // TODO

    return true;
}

static app_t *
app_new(int argc, char *argv[]) {
    app_t *self = mem_ecalloc(1, sizeof(*self));

    self->argc = argc;
    self->argv = argv;

    self->config = config_new();
    if (!app_init_config(self)) {
        err_error("failed to configuration");
        app_del(self);
        return NULL;
    }

    self->cmdargs = cmdargs_new();
    if (!cmdargs_parse(self->cmdargs, self->argc, self->argv)) {
        err_error("failed to parse command arguments");
        app_del(self);
        return NULL;
    }

    if (!app_parse_opts(self)) {
        app_del(self);
        return NULL;
    }

    return self;
}

static void
app_usage(app_t *app) {
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
app_version(app_t *self) {
    fflush(stdout);
    fflush(stderr);

    printf("%s\n", VERSION);
    fflush(stdout);

    app_del(self);
    exit(0);
}

static bool
app_is_cap_cmdname(const app_t *self, const char *cmdname) {
    static const char *capcmdnames[] = {
        "home",
        "cd",
        "pwd",
        "ls",
        "cat",
        "run",
        "alias",
        NULL,
    };

    for (const char **p = capcmdnames; *p; ++p) {
        if (!strcmp(cmdname, *p)) {
            return true;
        }
    }

    return false;
}

static int
app_execute_command_by_name(app_t *self, const char *name) {
    if (!strcmp(name, "home")) {
        homecmd_t *homecmd = homecmd_new(self->config, self->cmdargs);
        self->config = NULL; // moved
        self->cmdargs = NULL; // moved
        int result = homecmd_run(homecmd);
        homecmd_del(homecmd);
        return result;
    } else if (!strcmp(name, "cd")) {

    } else if (!strcmp(name, "pwd")) {

    } else if (!strcmp(name, "ls")) {

    } else if (!strcmp(name, "cat")) {

    } else if (!strcmp(name, "run")) {

    } else if (!strcmp(name, "alias")) {

    } 

    err_error("invalid command name \"%s\"", name);
    return -1;
}

static int
app_execute_alias_by_name(app_t *self, const char *name) {
    puts("run alias");
    return 0;
}

static int
app_run(app_t *self) {
    if (self->opts.ishelp) {
        app_usage(self);
    }

    if (self->opts.isversion) {
        app_version(self);
    }

    if (self->argc < 2) {
        app_usage(self);
    }

    const char *cmdname = cmdargs_get_cmdname(self->cmdargs);
    if (!cmdname) {
        err_error("command name is null");
        return -1; // impossible
    }

    if (app_is_cap_cmdname(self, cmdname)) {
        return app_execute_command_by_name(self, cmdname);
    }

    return app_execute_alias_by_name(self, cmdname);
}

int
main(int argc, char *argv[]) {
    app_t *app = app_new(argc, argv);
    if (!app) {
        err_die("failed to start application");
    }

    int result = app_run(app);
    app_del(app);
    
    return result;
}
