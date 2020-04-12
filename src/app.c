/**
 * Cap
 *
 * license: MIT
 *  author: Aizawa Yuta
 *   since: 2016
 */
#include <app.h>

/**
 * numbers
 */
enum {
    MAX_RECURSION_LIMIT = 8,
    NOT_FOUND = -1,
};

/**
 * program option
 */
struct opts {
    bool ishelp;
    bool isversion;
};

/**
 * application structure
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
 * parse options
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
        {0},
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
 * destruct module
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
 * deploy Cap's environment at user's file system
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
    char path[FILE_NPATH];
    if (!file_solvefmt(path, sizeof path, "%s/cd", vardir)) {
        err_error("failed to create path of cd variable");
        return false;
    }
    if (!file_exists(path)) {
        if (!file_writeline(userhome, path)) {
            err_error("failed to write line to cd of variable");
            return false;
        }
    }

    if (!file_solvefmt(path, sizeof path, "%s/home", vardir)) {
        err_error("failed to create path of home variable");
        return false;
    }
    if (!file_exists(path)) {
        if (!file_writeline(userhome, path)) {
            err_error("failed to write line to home of variable");
            return false;
        }
    }

    if (!file_solvefmt(path, sizeof path, "%s/editor", vardir)) {
        err_error("failed to create path of home variable");
        return false;
    }
    if (!file_exists(path)) {
        if (!file_writeline("", path)) {
            err_error("failed to write line to home of variable");
            return false;
        }
    }

    // make snippets directory
    char codesdir[FILE_NPATH];
    if (!file_solvefmt(codesdir, sizeof codesdir, "%s/codes", appdir)) {
        err_error("failed to solve path for snippet codes directory");
        return false;
    }
    if (!file_exists(codesdir)) {
        if (file_mkdirq(codesdir) != 0) {
            err_error("failed to make directory for snippet codes directory");
            return false;
        }
    }

    return true;
}

/**
 * parse arguments
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
    distribute_args_t dargs = {0};
    if (!distribute_args(&dargs, argc, argv)) {
        return false;
    }

    self->argc = dargs.argc;
    self->argv = dargs.argv;
    self->cmd_argc = dargs.cmd_argc;
    self->cmd_argv = dargs.cmd_argv;
    return true;
}

/**
 * construct module
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
    if (!config_init(self->config)) {
        err_error("failed to configuration");
        app_del(self);
        return NULL;
    }

    return self;
}

/**
 * show usage of module
 *
 * @param[in] app 
 */
static void
app_usage(app_t *app) {
    static const char usage[] =
        "Cap is shell for snippet codes.\n"
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
        "    home       change home directory, or show\n"
        "    cd         change current directory by relative of home\n"
        "    pwd        show current directory\n"
        "    ls         show list in current directory\n"
        "    cat        concatenate files and show\n"
        "    make       compile template language\n"
        "    run        run script\n"
        "    exec       execute command line\n"
        "    alias      run alias command\n"
        "    edit       run editor with file name\n"
        "    editor     set editor or show\n"
        "    mkdir      make directory at environment\n"
        "    rm         remove file or directory from environment\n"
        "    mv         rename file on environment\n"
        "    cp         copy file\n"
        "    touch      create empty file\n"
        "    snippet    save or show snippet codes\n"
        "    link       create symbolic link\n"
        "    sh         run shell\n"
        "    find       find snippet\n"
    ;
    static const char *examples[] = {
        "    $ cap home\n"
        "    $ cap pwd\n"
        "    $ cap ls\n"
        ,
        "    $ cap editor /usr/bin/vim\n"
        "    $ cap edit my.txt\n"
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
 * show version of module
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
 * check if the argument is the command name of Cap
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
        "exec",
        "alias",
        "edit",
        "editor",
        "mkdir",
        "rm",
        "mv",
        "cp",
        "touch",
        "snippet",
        "link",
        "hub",
        "make",
        "sh",
        "find",
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
 * execute command by command name
 *
 * @param[in] self
 * @param[in] name command name
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_execute_command_by_name(app_t *self, const char *name) {
#define routine(cmd) { \
        cmd##_t *cmd = cmd##_new(self->config, self->cmd_argc, self->cmd_argv); \
        if (!cmd) { \
            return 1; \
        } \
        result = cmd##_run(cmd); \
        cmd##_del(cmd); \
    } \

    int result = 0;

    if (cstr_eq(name, "home")) {
        routine(homecmd);
    } else if (cstr_eq(name, "cd")) {
        routine(cdcmd);
    } else if (cstr_eq(name, "pwd")) {
        routine(pwdcmd);
    } else if (cstr_eq(name, "ls")) {
        routine(lscmd);
    } else if (cstr_eq(name, "cat")) {
        routine(catcmd);
    } else if (cstr_eq(name, "run")) {
        routine(runcmd);
    } else if (cstr_eq(name, "exec")) {
        routine(execcmd);
    } else if (cstr_eq(name, "alias")) {
        routine(alcmd);
    } else if (cstr_eq(name, "edit")) {
        routine(editcmd);
    } else if (cstr_eq(name, "editor")) {
        routine(editorcmd);
    } else if (cstr_eq(name, "mkdir")) {
        routine(mkdircmd);
    } else if (cstr_eq(name, "rm")) {
        rmcmd_t *cmd = rmcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        result = rmcmd_run(cmd);
        switch (rmcmd_errno(cmd)) {
        case RMCMD_ERR_NOERR: break;
        default: err_error(rmcmd_what(cmd)); break;
        }
        rmcmd_del(cmd);
    } else if (cstr_eq(name, "mv")) {
        routine(mvcmd);
    } else if (cstr_eq(name, "cp")) {
        routine(cpcmd);
    } else if (cstr_eq(name, "touch")) {
        routine(touchcmd);
    } else if (cstr_eq(name, "snippet")) {
        routine(snptcmd);
    } else if (cstr_eq(name, "link")) {
        routine(linkcmd);
    } else if (cstr_eq(name, "make")) {
        routine(makecmd);
    } else if (cstr_eq(name, "sh")) {
        routine(shcmd);
    } else if (cstr_eq(name, "find")) {
        routine(findcmd);
    } else {
        err_error("invalid command name \"%s\"", name);
        result = 1;
    }

    return result;
}

/**
 * execute alias by alias name
 *
 * @param[in]  self
 * @param[in]  name alias name
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_execute_alias_by_name(app_t *self, bool *found, const char *name) {
    almgr_t *almgr = almgr_new(self->config);

    // find alias value by name
    // find first from local scope
    // not found to find from global scope
    char val[1024];
    if (almgr_find_alias_value(almgr, val, sizeof val, name, CAP_SCOPE_LOCAL) == NULL) {
        almgr_clear_error(almgr);
        if (almgr_find_alias_value(almgr, val, sizeof val, name, CAP_SCOPE_GLOBAL) == NULL) {
            *found = false;
            return 1;
        }
    }
    almgr_del(almgr);
    *found = true;

    // create cap's command line with alias value
    string_t *cmdline = str_new();

    str_app(cmdline, "cap ");
    str_app(cmdline, val);
    str_app(cmdline, " ");
    char escarg[1024];
    for (int i = 1; i < self->cmd_argc; ++i) {
        const char * arg = self->cmd_argv[i];
        str_app(cmdline, "\"");
        escape(escarg, sizeof escarg, arg, "\"");
        str_app(cmdline, escarg);
        str_app(cmdline, "\"");
        str_app(cmdline, " ");
    }
    str_popb(cmdline);

    // convert command to application's arguments
    cl_t *cl = cl_new();
    cl_parse_str_opts(cl, str_getc(cmdline), 0);
    str_del(cmdline);

    int argc = cl_len(cl);
    char **argv = cl_escdel(cl);

    // re-parse application's arguments
    freeargv(self->argc, self->argv);
    freeargv(self->cmd_argc, self->cmd_argv);

    if (!app_parse_args(self, argc, argv)) {
        err_error("failed to parse arguments");
        return 1;
    }
    freeargv(argc, argv);

    // increment recursion count for safety
    self->config->recursion_count++;

    // run application
    if (self->config->recursion_count >= MAX_RECURSION_LIMIT) {
        err_error("reached recursion limit");
        return 1;
    }

    return app_run(self);
}

static cstring_array_t *
pushf_argv(int argc, char *argv[], const char *front) {
    cstring_array_t *arr = cstrarr_new();

    cstrarr_pushb(arr, front);

    for (int32_t i = 0; i < argc; ++i) {
        cstrarr_pushb(arr, argv[i]);
    }

    return mem_move(arr);
}

/**
 * execute run command with command arguments
 *
 * @param[in] self
 *
 * @return success to 0 else other
 */
static int
execute_run(app_t *self, int argc, char *argv[]) {
    runcmd_t *cmd = runcmd_new(self->config, argc, argv);
    if (!cmd) {
        return 1;
    }

    int result = runcmd_run(cmd);
    runcmd_del(cmd);

    return result;
}

static char *
load_path_var_from_resource(app_t *self, const char *rcpath) {
    char *src = file_readcp_from_path(rcpath);

    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(self->config);
    gc_t *gc = gc_new();
    context_t *ctx = ctx_new(gc);
    opts_t *opts = opts_new();

    tkr_parse(tkr, src);
    free(src);
    src = NULL;
    if (tkr_has_error(tkr)) {
        err_error("%s", tkr_get_error_detail(tkr));
        return NULL;
    }

    ast_move_opts(ast, opts);
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_error(ast)) {
        err_error("%s", ast_get_error_detail(ast));
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_error(ast)) {
        err_error("%s", ast_get_error_detail(ast));
        return NULL;        
    }

    tkr_del(tkr);
    ast_del(ast);

    object_dict_t *varmap = ctx_get_varmap_at_global(ctx);
    const object_dict_item_t *item = objdict_getc(varmap, "PATH");
    if (!item) {
        ctx_del(ctx);
        gc_del(gc);
        return NULL;
    }

    printf("%s", ctx_getc_buf(ctx));
    fflush(stdout);

    char *path = cstr_edup(str_getc(item->value->string));

    ctx_del(ctx);
    gc_del(gc);

    return path;
}

static cstring_array_t *
split_path_var(const char *path) {
    cstring_array_t *dirs = cstrarr_new();
    string_t *s = str_new();

    for (const char *p = path; *p; ++p) {
        if (*p == ':') {
            if (str_len(s)) {
                cstrarr_pushb(dirs, str_getc(s));
                str_clear(s);
            }
        } else {
            str_pushb(s, *p);
        }
    }

    if (str_len(s)) {
        cstrarr_pushb(dirs, str_getc(s));
    }

    str_del(s);
    return dirs;
}

static int
execute_program_by_dirname(app_t *self, bool *found, const char *cap_dirname) {
    *found = false;

    const char *cmdname = self->cmd_argv[0];
    char cap_fpath[FILE_NPATH];
    snprintf(cap_fpath, sizeof cap_fpath, "%s/%s", cap_dirname, cmdname);

    const char *org = get_origin(self->config, cap_fpath);
    char real_path[FILE_NPATH];
    if (!file_solvefmt(real_path, sizeof real_path, "%s/%s", org, cap_fpath)) {
        err_error("failed to solve in execute program in directory");
        return 1;
    }
    if (!file_exists(real_path)) {
        return 1;
    }
    *found = true;

    cstring_array_t *args = cstrarr_new();
    cstrarr_pushb(args, "run");
    cstrarr_pushb(args, cap_fpath);
    for (int32_t i = 1; i < self->cmd_argc; ++i) {
        cstrarr_pushb(args, self->cmd_argv[i]);
    }

    int argc = cstrarr_len(args);
    char **argv = cstrarr_escdel(args);
    return execute_run(self, argc, argv);
}

static int
execute_program_by_caprc(app_t *self, bool *found, const char *org) {
    *found = false;
    char rcpath[FILE_NPATH];

    if (!file_solvefmt(rcpath, sizeof rcpath, "%s/.caprc", org)) {
        return 1;
    }

    if (!file_exists(rcpath)) {
        return 1;
    }

    char *path = load_path_var_from_resource(self, rcpath);
    if (!path) {
        return 1;
    }

    cstring_array_t *dirs = split_path_var(path);
    free(path);
    if (!cstrarr_len(dirs)) {
        return 1;
    }

    for (int32_t i = 0; i < cstrarr_len(dirs); ++i) {
        const char *cap_dirname = cstrarr_getc(dirs, i);

        *found = false;
        int result = execute_program_by_dirname(self, found, cap_dirname);
        if (*found) {
            return result;
        }
    }

    return 1;
}

static int
execute_program_by_path_in_resource(app_t *self, bool *found, const char *cmdname) {
    if (cmdname[0] == '.') {
        *found = false;
        return 1;
    }

    int result;

    *found = false;
    result = execute_program_by_caprc(self, found, self->config->cd_path);
    if (*found) {
        return result;
    }

    *found = false;
    result = execute_program_by_caprc(self, found, self->config->home_path);
    if (*found) {
        return result;
    }

    puts("not found program in caprc");
    return 1;
}

/**
 * run module
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

    bool found = false;
    int result = app_execute_alias_by_name(self, &found, cmdname);
    if (found) {
        return result;
    }

    found = false;
    result = execute_snippet(self->config, &found, cmdname, self->cmd_argc-1, self->cmd_argv+1);
    if (found) {
        return result;
    }

    found = false;
    result = execute_program_by_path_in_resource(self, &found, cmdname);
    if (found) {
        return result;
    }

    cstring_array_t *new_argv = pushf_argv(self->cmd_argc, self->cmd_argv, "run");
    int argc = cstrarr_len(new_argv);
    char **argv = cstrarr_escdel(new_argv);
    return execute_run(self, argc, argv);
}

/**
 * main routine
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
