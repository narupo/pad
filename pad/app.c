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
struct PadAppOpts {
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
    struct PadAppOpts opts;
    PadErrStack *errstack;
} PadApp;

static int
PadApp_Run(PadApp *self, int argc, char *argv[]);

/**
 * parse options
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
PadApp_ParseOpts(PadApp *self) {
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {"debug", no_argument, 0, 'd'},
        {0},
    };

    // init status
    self->opts = (struct PadAppOpts){0};
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
PadApp_Del(PadApp *self) {
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
PadApp_DeployEnv(const PadApp *self) {
    char userhome[PAD_FILE__NPATH];
    if (!PadFile_GetUserHome(userhome, sizeof userhome)) {
        Pad_PushErr("failed to get user's home directory. what is your file system?");
        return false;
    }

    // make application directory
    char appdir[PAD_FILE__NPATH];
    if (!PadFile_SolveFmt(appdir, sizeof appdir, "%s/.pad", userhome)) {
        Pad_PushErr("faield to create application directory path");
        return false;
    }

    if (!PadFile_IsExists(appdir)) {
        if (PadFile_MkdirQ(appdir) != 0) {
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
 * @return success to pointer to dynamic allocate memory to PadApp
 * @return failed to NULL
 */
static PadApp *
PadApp_New(void) {
    PadApp *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        PadApp_Del(self);
        return NULL;
    }

    self->config = PadConfig_New();
    if (!self->config) {
        PadApp_Del(self);
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
PadApp_Usage(PadApp *app) {
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
PadApp_Version(PadApp *self) {
    fflush(stdout);
    fflush(stderr);
    printf("%s\n", PAD__VERSION);
    fflush(stdout);
}

static bool
PadApp_ParseArgs(PadApp *self, int argc, char *argv[]) {
    PadDistriArgs dargs = {0};
    PadDistriArgs_Distribute(&dargs, argc, argv);
    self->argc = dargs.argc;
    self->argv = dargs.argv;
    self->cmd_argc = dargs.cmd_argc;
    self->cmd_argv = dargs.cmd_argv;
    return true;
}

static bool
PadApp_Init(PadApp *self, int argc, char *argv[]) {
    if (!PadConfig_Init(self->config)) {
        Pad_PushErr("failed to configuration");
        return false;
    }

    if (!PadApp_ParseArgs(self, argc, argv)) {
        Pad_PushErr("failed to parse arguments");
        return false;
    }

    if (!PadApp_ParseOpts(self)) {
        Pad_PushErr("failed to parse options");
        return false;
    }

    if (!PadApp_DeployEnv(self)) {
        Pad_PushErr("failed to deploy environment at file system");
        return false;
    }

    return true;
}

static void
PadApp_TraceKit(const PadApp *self, const PadKit *kit, FILE *fout) {
    if (self->opts.is_debug) {
        PadKit_TraceErrDebug(kit, fout);
    } else {
        PadKit_TraceErr(kit, fout);
    }
}

static int
_PadApp_Run(PadApp *self) {
    char *content = PadFile_ReadCopy(stdin);
    if (!content) {
        Pad_PushErr("failed to read from stdin");
        return 1;
    }

    PadKit *kit = PadKit_New(self->config);
    PadCtx *ctx = PadKit_GetCtx(kit);
    PadCtx_SetUseBuf(ctx, false);  // no use stdout/stderr buffer

    if (!PadKit_CompileFromStr(kit, content)) {
        PadApp_TraceKit(self, kit, stderr);
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
PadApp_RunArgs(PadApp *self) {
    int argc = self->cmd_argc;
    char **argv = self->cmd_argv;
    if (!argc) {
        Pad_PushErr("invalid arguments");
        return 1;
    }

    const char *path = argv[0];
    if (!PadFile_IsExists(path)) {
        Pad_PushErr("not found \"%s\"", path);
        return 1;
    }

    PadKit *kit = PadKit_New(self->config);
    PadKit_SetUseBuf(kit, false);  // no use stdout/stderr buffer
    
    if (!PadKit_CompileFromPathArgs(kit, path, argc, argv)) {
        PadApp_TraceKit(self, kit, stderr);
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
PadApp_Run(PadApp *self, int argc, char *argv[]) {
    if (!PadApp_Init(self, argc, argv)) {
        return 1;
    }

    if (self->opts.is_help) {
        PadApp_Usage(self);
        return 0;
    }

    if (self->opts.is_version) {
        PadApp_Version(self);
        return 0;
    }

    if (self->cmd_argc >= 1) {
        return PadApp_RunArgs(self);
    }

    return _PadApp_Run(self);
}

/**
 * stack trace
 * 
 * @param[in] *self
 */
static void
PadApp_Trace(const PadApp *self) {
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

    PadApp *app = PadApp_New();
    if (!app) {
        PadErr_Die("failed to start application");
    }

    int result = PadApp_Run(app, argc, argv);
    if (result != 0) {
        PadApp_Trace(app);
    }

    PadApp_Del(app);

    fflush(stdout);
    return result;
}
