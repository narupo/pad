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

/**
 * Application structure.
 */
struct app {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
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

    // init status
    self->opts = (struct opts){0};
    optind = 0;
    opterr = 0;
    
    // parse options
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
        freeargv(self->argc, self->argv);
        freeargv(self->cmd_argc, self->cmd_argv);
        free(self);
    }
}

static bool
app_init_config(app_t *self) {
    if (!file_solve(self->config->var_cd_path, sizeof self->config->var_cd_path, "~/.cap/var/cd")) {
        err_error("failed to create path of cd of variable");
        return false;
    }

    if (!file_solve(self->config->var_home_path, sizeof self->config->var_home_path, "~/.cap/var/home")) {
        err_error("failed to create path of home of variable");
        return false;
    }

    return true;
}

/**
 * Deploy Cap's environment at user's file system
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
app_deploy_env(const app_t *self) {
    char userhome[FILE_NPATH];
    if (!file_get_user_home(userhome, sizeof userhome)) {
        err_error("failed to get user's home directory. what is your file system?");
        return false;
    }

    // make application directory
    char appdir[FILE_NPATH];
    if (!file_solvefmt(appdir, sizeof appdir, "%s/.cap", userhome)) {
        err_error("faield to create application directory path");
        return false;
    }

    if (!file_exists(appdir)) {
        if (file_mkdirq(appdir) != 0) {
            err_error("failed to make application directory");
            return false;
        }
    }

    // make variable directory
    char vardir[FILE_NPATH];
    if (!file_solvefmt(vardir, sizeof vardir, "%s/var", appdir)) {
        err_error("failed to create path of variable");
        return false;
    }

    if (!file_exists(vardir)) {
        if (file_mkdirq(vardir) != 0) {
            err_error("failed to make variable directory");
            return false;
        }
    }

    // make variable files
    char tmp[FILE_NPATH];
    if (!file_solvefmt(tmp, sizeof tmp, "%s/cd", vardir)) {
        err_error("failed to create path of cd variable");
        return false;
    }
    if (!file_exists(tmp)) {
        if (!file_writeline(userhome, tmp)) {
            err_error("failed to write file to cd of variable");
            return false;
        }
    }

    if (!file_solvefmt(tmp, sizeof tmp, "%s/home", vardir)) {
        err_error("failed to create path of home variable");
        return false;
    }
    if (!file_exists(tmp)) {
        if (!file_writeline(userhome, tmp)) {
            err_error("failed to write file to home of variable");
            return false;
        }
    }

    return true;
}

static bool
app_parse_args(app_t *self, int argc, char *argv[]) {
    cstring_array_t *app_args = cstrarr_new();
    cstring_array_t *cmd_args = cstrarr_new();

    int m = 0;
    for (int i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            if (strcmp(arg, "cap") == 0) {
                cstrarr_push(app_args, arg);
            } else {
                err_error("invalid application name");
                cstrarr_push(app_args, "cap");
            }
            m = 10;
            break;
        case 10:
            if (arg[0] == '-') {
                cstrarr_push(app_args, arg);
            } else {
                cstrarr_push(cmd_args, arg);
                m = 20;
            }
            break;
        case 20:
            cstrarr_push(cmd_args, arg);
            break;
        }
    }

    self->argc = cstrarr_len(app_args);
    self->argv = cstrarr_escdel(app_args);
    self->cmd_argc = cstrarr_len(cmd_args);
    self->cmd_argv = cstrarr_escdel(cmd_args);

    return true;
}

static app_t *
app_new(int argc, char *argv[]) {
    app_t *self = mem_ecalloc(1, sizeof(*self));

    if (!app_parse_args(self, argc, argv)) {
        err_error("failed to parse arguments");
        app_del(self);
        return NULL;
    }

    if (!app_deploy_env(self)) {
        err_error("failed to deploy environment at file systems");
        app_del(self);
        return NULL;
    }

    self->config = config_new();
    if (!app_init_config(self)) {
        err_error("failed to configuration");
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
}

static void
app_version(app_t *self) {
    fflush(stdout);
    fflush(stderr);

    printf("%s\n", _CAP_VERSION);
    fflush(stdout);
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
        homecmd_t *homecmd = homecmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = homecmd_run(homecmd);
        homecmd_del(homecmd);
        return result;
    } else if (!strcmp(name, "cd")) {
        cdcmd_t *cdcmd = cdcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = cdcmd_run(cdcmd);
        cdcmd_del(cdcmd);
        return result;
    } else if (!strcmp(name, "pwd")) {
        pwdcmd_t *pwdcmd = pwdcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = pwdcmd_run(pwdcmd);
        pwdcmd_del(pwdcmd);
        return result;
    } else if (!strcmp(name, "ls")) {
        lscmd_t *lscmd = lscmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = lscmd_run(lscmd);
        lscmd_del(lscmd);
        return result;
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
        return 0;
    }

    if (self->opts.isversion) {
        app_version(self);
        return 0;
    }

    if (self->cmd_argc == 0) {
        app_usage(self);
        return 0;
    }

    const char *cmdname = self->cmd_argv[0];
    if (!cmdname) {
        err_error("command name is null");
        return 1; // impossible
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
