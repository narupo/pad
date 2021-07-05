/**
 * Pad
 *
 * license: MIT
 *  author: narupo
 *   since: 2016
 */
#include <pad/app.h>

/**
 * program option
 */
struct PadOpts {
    bool is_help;
    bool is_version;
    bool is_debug;
};

/**
 * application structure
 */
typedef struct {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
    PadConfig *config;
    struct PadOpts opts;
    PadErrStack *errstack;
} app_t;

static int
app_run(app_t *self, int argc, char *argv[]);

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
        {"debug", no_argument, 0, 'd'},
        {0},
    };

    // init status
    self->opts = (struct PadOpts){0};
    optind = 0;
    opterr = 0;

    // parse options
    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hVd", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->opts.is_help = true; break;
        case 'V': self->opts.is_version = true; break;
        case 'd': self->opts.is_debug = true; break;
        case '?':
        default:
            Pad_PushErr("invalid option");
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
        PadConfig_Del(self->config);
        PadErrStack_Del(self->errstack);
        free(self);
    }
}

/**
 * deploy Pad's environment at user's file system
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
app_deploy_env(const app_t *self) {
    char userhome[FILE_NPATH];
    if (!file_get_user_home(userhome, sizeof userhome)) {
        Pad_PushErr("failed to get user's home directory. what is your file system?");
        return false;
    }

    // make application directory
    char appdir[FILE_NPATH];
    if (!file_solvefmt(appdir, sizeof appdir, "%s/.pad", userhome)) {
        Pad_PushErr("faield to create application directory path");
        return false;
    }

    if (!file_exists(appdir)) {
        if (file_mkdirq(appdir) != 0) {
            Pad_PushErr("failed to make application directory");
            return false;
        }
    }

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
app_new(void) {
    app_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        app_del(self);
        return NULL;
    }

    self->config = PadConfig_New();
    if (!self->config) {
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
        "Pad is programming language.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    pad [options]... [file] [arguments]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage\n"
        "    -V, --version    show version\n"
        "    -d, --debug      debug mode\n"
        "\n"
    ;
    fprintf(stderr,
        "%s"
    , usage);
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
    printf("%s\n", PAD_VERSION);
    fflush(stdout);
}

static bool
app_parse_args(app_t *self, int argc, char *argv[]) {
    PadDistriArgs dargs = {0};
    PadDistriArgs_Distribute(&dargs, argc, argv);
    self->argc = dargs.argc;
    self->argv = dargs.argv;
    self->cmd_argc = dargs.cmd_argc;
    self->cmd_argv = dargs.cmd_argv;
    return true;
}

static bool
app_init(app_t *self, int argc, char *argv[]) {
    if (!PadConfig_Init(self->config)) {
        Pad_PushErr("failed to configuration");
        return false;
    }

    if (!app_parse_args(self, argc, argv)) {
        Pad_PushErr("failed to parse arguments");
        return false;
    }

    if (!app_parse_opts(self)) {
        Pad_PushErr("failed to parse options");
        return false;
    }

    if (!app_deploy_env(self)) {
        Pad_PushErr("failed to deploy environment at file system");
        return false;
    }

    return true;
}

static void
trace_kit(const app_t *self, const PadKit *kit, FILE *fout) {
    if (self->opts.is_debug) {
        PadKit_TraceErrDebug(kit, fout);
    } else {
        PadKit_TraceErr(kit, fout);
    }
}

static int
_app_run(app_t *self) {
    char *content = file_readcp(stdin);
    if (!content) {
        Pad_PushErr("failed to read from stdin");
        return 1;
    }

    PadKit *kit = PadKit_New(self->config);
    PadCtx *ctx = PadKit_GetCtx(kit);
    PadCtx_SetUseBuf(ctx, false);  // no use stdout/stderr buffer

    if (!PadKit_CompileFromStr(kit, content)) {
        trace_kit(self, kit, stderr);
        Pad_PushErr("failed to compile from stdin");
        return 1;
    }

    fflush(stdout);
    fflush(stderr);

    PadKit_Del(kit);
    free(content);
    return 0;
}

static int
app_run_args(app_t *self) {
    int argc = self->cmd_argc;
    char **argv = self->cmd_argv;
    if (!argc) {
        Pad_PushErr("invalid arguments");
        return 1;
    }

    const char *path = argv[0];
    if (!file_exists(path)) {
        Pad_PushErr("not found \"%s\"", path);
        return 1;
    }

    PadKit *kit = PadKit_New(self->config);
    PadCtx *ctx = PadKit_GetCtx(kit);
    PadCtx_SetUseBuf(ctx, false);  // no use stdout/stderr buffer
    
    if (!PadKit_CompileFromPathArgs(kit, path, argc, argv)) {
        trace_kit(self, kit, stderr);
        Pad_PushErr("failed to compile \"%s\"", path);
        return 1;
    }

    fflush(stdout);
    fflush(stderr);

    PadKit_Del(kit);
    return 0;
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
app_run(app_t *self, int argc, char *argv[]) {
    if (!app_init(self, argc, argv)) {
        return 1;
    }

    if (self->opts.is_help) {
        app_usage(self);
        return 0;
    }

    if (self->opts.is_version) {
        app_version(self);
        return 0;
    }

    if (self->cmd_argc >= 1) {
        return app_run_args(self);
    }

    return _app_run(self);
}

/**
 * stack trace
 * 
 * @param[in] *self
 */
static void
app_trace(const app_t *self) {
    if (PadErrStack_Len(self->errstack)) {
        fflush(stdout);
        PadErrStack_TraceSimple(self->errstack, stderr);
        fflush(stderr);        
    }
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
    // set locale for unicode object (char32_t, char16_t)
    setlocale(LC_CTYPE, "");

    app_t *app = app_new();
    if (!app) {
        err_die("failed to start application");
    }

    int result = app_run(app, argc, argv);
    if (result != 0) {
        app_trace(app);
    }

    app_del(app);

    fflush(stdout);
    return result;
}
