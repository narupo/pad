/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "app.h"

/**
 * Numbers
 */
enum {
    MAX_RECURSION_LIMIT = 8,
};

/**
 * Program option.
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

static int
app_run(app_t *self);

/**
 * Parse options
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
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

/**
 * Destruct module
 *
 * @param[in] self
 */
static void
app_del(app_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        freeargv(self->cmd_argc, self->cmd_argv);
        free(self);
    }
}

/**
 * Initialize config object
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
app_init_config(app_t *self) {
    self->config->scope = CAP_SCOPE_LOCAL;
    self->config->recursion_count = 0;

    if (!file_solve(self->config->var_cd_path, sizeof self->config->var_cd_path, "~/.cap/var/cd")) {
        err_error("failed to create path of cd of variable");
        return false;
    }

    if (!file_solve(self->config->var_home_path, sizeof self->config->var_home_path, "~/.cap/var/home")) {
        err_error("failed to create path of home of variable");
        return false;
    }

    if (!file_readline(self->config->cd_path, sizeof self->config->cd_path, self->config->var_cd_path)) {
        err_error("failed to read line from cd of variable");
        return false;
    }

    if (!file_readline(self->config->home_path, sizeof self->config->home_path, self->config->var_home_path)) {
        err_error("failed to read line from home of variable");
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
            err_error("failed to write line to cd of variable");
            return false;
        }
    }

    if (!file_solvefmt(tmp, sizeof tmp, "%s/home", vardir)) {
        err_error("failed to create path of home variable");
        return false;
    }
    if (!file_exists(tmp)) {
        if (!file_writeline(userhome, tmp)) {
            err_error("failed to write line to home of variable");
            return false;
        }
    }

    return true;
}

/**
 * Parse arguments
 *
 * @param[in]  self   
 * @param[in]  argc    
 * @param[in]  argv 
 *
 * @return success to true
 * @return failed to false
 */
static bool
app_parse_args(app_t *self, int argc, char *argv[]) {
    cstring_array_t *app_args = cstrarr_new();
    cstring_array_t *cmd_args = cstrarr_new();

    int m = 0;
    for (int i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            cstrarr_push(app_args, arg);
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

/**
 * Construct module
 *
 * @param[in] argc
 * @param[in] argv
 *
 * @return success to pointer to dynamic allocate memory to app_t
 * @return failed to NULL
 */
static app_t *
app_new(int argc, char *argv[]) {
    app_t *self = mem_ecalloc(1, sizeof(*self));

    if (!app_parse_args(self, argc, argv)) {
        err_error("failed to parse arguments");
        app_del(self);
        return NULL;
    }

    if (!app_parse_opts(self)) {
        err_error("failed to parse options");
        app_del(self);
        return NULL;
    }

    if (!app_deploy_env(self)) {
        err_error("failed to deploy environment at file system");
        app_del(self);
        return NULL;
    }

    self->config = config_new();
    if (!app_init_config(self)) {
        err_error("failed to configuration");
        app_del(self);
        return NULL;
    }

    return self;
}

/**
 * Show usage of module
 *
 * @param[in] app 
 */
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
        "    -h, --help       show usage\n"
        "    -V, --version    show version\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    home     Change home directory, or show\n"
        "    cd       Change current directory by relative of home\n"
        "    pwd      Show current directory\n"
        "    ls       Show list in current directory\n"
        "    cat      Concatenate files and show\n"
        "    run      Run script\n"
        "    alias    Run alias command\n"
        "    edit     Run editor with file name\n"
        "    mkdir    Make directory at environment\n"
        "    rm       Remove file or directory from environment\n"
        "    mv       Rename file on environment\n"
        "    cp       Copy file\n"
        "    touch    Create empty file\n"
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
        "    $ cap mkdir path/to/dir\n"
        "    $ cap edit path/to/dir/file.txt\n"
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

/**
 * Show version of module
 *
 * @param[in] self
 */
static void
app_version(app_t *self) {
    fflush(stdout);
    fflush(stderr);
    printf("%s\n", _CAP_VERSION);
    fflush(stdout);
}

/**
 * Check if the argument is the command name of Cap
 *
 * @param[in] self
 * @param[in] cmdname command name
 *
 * @return If the cmdname is the command name of Cap to true
 * @return If the cmdname not is the command name of Cap to false
 */
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
        "edit",
        "mkdir",
        "rm",
        "mv",
        "cp",
        "touch",
        NULL,
    };

    for (const char **p = capcmdnames; *p; ++p) {
        if (!strcmp(cmdname, *p)) {
            return true;
        }
    }

    return false;
}

/**
 * Execute command by command name
 *
 * @param[in] self
 * @param[in] name command name
 *
 * @return success to 0
 * @return failed to not 0
 */
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
        cdcmd_t *cmd = cdcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = cdcmd_run(cmd);
        cdcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "pwd")) {
        pwdcmd_t *cmd = pwdcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = pwdcmd_run(cmd);
        pwdcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "ls")) {
        lscmd_t *cmd = lscmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = lscmd_run(cmd);
        lscmd_del(cmd);
        return result;
    } else if (!strcmp(name, "cat")) {
        catcmd_t *cmd = catcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = catcmd_run(cmd);
        catcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "run")) {
        runcmd_t *cmd = runcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = runcmd_run(cmd);
        runcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "alias")) {
        alcmd_t *cmd = alcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = alcmd_run(cmd);
        alcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "edit")) {
        editcmd_t *cmd = editcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = editcmd_run(cmd);
        editcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "mkdir")) {
        mkdircmd_t *cmd = mkdircmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = mkdircmd_run(cmd);
        mkdircmd_del(cmd);
        return result;
    } else if (!strcmp(name, "rm")) {
        rmcmd_t *cmd = rmcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = rmcmd_run(cmd);
        switch (rmcmd_errno(cmd)) {
        case RMCMD_ERR_NOERR: break;
        default: err_error(rmcmd_what(cmd)); break;
        }
        rmcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "mv")) {
        mvcmd_t *cmd = mvcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = mvcmd_run(cmd);
        mvcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "cp")) {
        cpcmd_t *cmd = cpcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = cpcmd_run(cmd);
        cpcmd_del(cmd);
        return result;
    } else if (!strcmp(name, "touch")) {
        touchcmd_t *cmd = touchcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        self->config = NULL; // moved
        self->cmd_argv = NULL; // moved
        int result = touchcmd_run(cmd);
        touchcmd_del(cmd);
        return result;
    }

    err_error("invalid command name \"%s\"", name);
    return 1;
}

/**
 * Execute alias by alias name
 *
 * @param[in] self
 * @param[in] name alias name
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_execute_alias_by_name(app_t *self, const char *name) {
    almgr_t *almgr = almgr_new(self->config);

    // find alias value by name
    char val[1024];
    int scope = CAP_SCOPE_LOCAL;
    if (almgr_find_alias_value(almgr, val, sizeof val, name, scope) == NULL) {
        scope = CAP_SCOPE_GLOBAL;
        almgr_clear_error(almgr);
        if (almgr_find_alias_value(almgr, val, sizeof val, name, scope) == NULL) {
            if (almgr_has_error(almgr)) {
                err_error("not found alias \"%s\". %s", name, almgr_get_error_detail(almgr));
            } else {
                err_error("not found alias \"%s\"", name);
            }
            return 1;
        }
    }
    almgr_del(almgr);

    // create cap's command line with alias value
    string_t *cmdline = str_new();

    str_app(cmdline, "cap ");
    str_app(cmdline, val);
    str_app(cmdline, " ");
    for (int i = 1; i < self->cmd_argc; ++i) {
        str_app(cmdline, "\"");
        str_app(cmdline, self->cmd_argv[i]);
        str_app(cmdline, "\"");
        str_app(cmdline, " ");
    }
    str_popb(cmdline);

    // convert command to application's arguments
    cl_t *cl = cl_new();
    cl_parse_str(cl, str_getc(cmdline));
    str_del(cmdline);

    int argc = cl_len(cl);
    char **argv = cl_escdel(cl);

    // re-parse application's arguments
    freeargv(self->argc, self->argv);
    freeargv(self->cmd_argc, self->cmd_argv);

    if (!app_parse_args(self, argc, argv)) {
        err_error("failed to parse arguments");
        return 2;
    }
    freeargv(argc, argv);

    // reset scope (extends environment)
    self->config->scope = scope;

    // increment recursion count for safety
    self->config->recursion_count++;

    // run application
    if (self->config->recursion_count >= MAX_RECURSION_LIMIT) {
        err_error("reached recursion limit");
        return 3;
    }
    return app_run(self);
}

/**
 * Run module
 *
 * @param[in] self
 *
 * @return success to 0
 * @return failed to not 0
 */
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

/**
 * Main routine
 *
 * @param[in] argc
 * @param[in] argv
 *
 * @return success to 0
 * @return failed to not 0
 */
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
