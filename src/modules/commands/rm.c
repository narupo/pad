#include "modules/commands/rm.h"

extern int opterr;
extern int optind;

struct opts {
    bool is_help;
    bool is_recursive;
};

struct rmcmd {
    config_t *config;
    int argc;
    int optind;
    rmcmd_errno_t errno_;
    char **argv;
    struct opts opts;
    char what[1024];
    char parpath[FILE_NPATH]; // parent path. path of cd or home
};

static bool
rmcmd_parse_opts(rmcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"recursive", no_argument, 0, 'r'},
        {},
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hr", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'r': self->opts.is_recursive = true; break;
        case '?':
        default:
            strappfmt(self->what, sizeof self->what, "unknown option.");
            self->errno_ = RMCMD_ERR_UNKNOWN_OPTS;
            return false;
        }
    }

    if (self->argc < optind) {
        strappfmt(self->what, sizeof self->what, "failed to parse option.");
        self->errno_ = RMCMD_ERR_PARSE_OPTS;
        return false;
    }

    self->optind = optind;
    return true;
}

void
rmcmd_del(rmcmd_t *self) {
    if (!self) {
        return;
    }
    freeargv(self->argc, self->argv);
    config_del(self->config);
    free(self);
}

rmcmd_t *
rmcmd_new(config_t *move_config, int argc, char **move_argv) {
    rmcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!rmcmd_parse_opts(self)) {
        return self;
    }

    return self;
}

static void
rmcmd_show_usage(rmcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "   cap rm [path] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "   -h, --help         show usage\n"
        "   -r, --recursive    remove directories and their contents recursively\n"
        "\n"
    );
    fflush(stderr);
}

rmcmd_errno_t
rmcmd_errno(const rmcmd_t *self) {
    return self->errno_;
}

const char *
rmcmd_what(const rmcmd_t *self) {
    return self->what;
}

static bool
rmcmd_remove_r(rmcmd_t *self, const char *dirpath) {
    if (isoutofhome(self->config->var_home_path, dirpath)) {
        strappfmt(self->what, sizeof self->what, "\"%s\" is out of home.", dirpath);
        self->errno_ = RMCMD_ERR_OUTOFHOME;
        return false;
    }

    if (!file_isdir(dirpath)) {
        strappfmt(self->what, sizeof self->what, "\"%s\" is not a directory.", dirpath);
        self->errno_ = RMCMD_ERR_OPENDIR;
        return false;
    }

    file_dir_t *dir = file_diropen(dirpath);
    if (!dir) {
        strappfmt(self->what, sizeof self->what, "failed to open directory \"%s\".", dirpath);
        self->errno_ = RMCMD_ERR_OPENDIR;
        return false;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *dirname = file_dirnodename(node);
        if (!strcmp(dirname, ".") || !strcmp(dirname, "..")) {
            continue;
        }

        char path[FILE_NPATH];
        if (!file_solvefmt(path, sizeof path, "%s/%s", dirpath, dirname)) {
            strappfmt(self->what, sizeof self->what, "failed to solve path by \"%s\".", dirname);
            self->errno_ = RMCMD_ERR_SOLVEPATH;
        }

        if (file_isdir(path)) {
            if (!rmcmd_remove_r(self, path)) {
                return false;
            }
            // directory is empty, remove directory
            if (file_remove(path) != 0) {
                strappfmt(self->what, sizeof self->what, "failed to remove file \"%s\".", path);
                self->errno_ = RMCMD_ERR_REMOVE_FILE;
                return false;
            }            
        } else {
            if (file_remove(path) != 0) {
                strappfmt(self->what, sizeof self->what, "failed to remove file \"%s\".", path);
                self->errno_ = RMCMD_ERR_REMOVE_FILE;
                return false;
            }
        }
    }

    if (file_dirclose(dir) != 0) {
        strappfmt(self->what, sizeof self->what, "failed to close directory \"%s\".", dirpath);
        self->errno_ = RMCMD_ERR_CLOSEDIR;        
        return false;
    }

    return true;
}

static int
rmcmd_rmr(rmcmd_t *self) {
    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        char path[FILE_NPATH];
    
        if (!file_solvefmt(path, sizeof path, "%s/%s", self->parpath, argpath)) {
            strappfmt(self->what, sizeof self->what, "failed to solve path from \"%s\".", argpath);
            self->errno_ = RMCMD_ERR_SOLVEPATH;
            return 1;
        }

        if (!rmcmd_remove_r(self, path)) {
            strappfmt(self->what, sizeof self->what, "could not delete recusively.");
            return 1;
        }
    }

    return 0;
}

static int
rmcmd_rm(rmcmd_t *self) {
    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        char path[FILE_NPATH];
    
        if (!file_solvefmt(path, sizeof path, "%s/%s", self->parpath, argpath)) {
            strappfmt(self->what, sizeof self->what, "failed to solve path.");
            self->errno_ = RMCMD_ERR_SOLVEPATH;
            return 1;
        }

        if (isoutofhome(self->config->var_home_path, path)) {
            strappfmt(self->what, sizeof self->what, "\"%s\" is out of home.", path);
            self->errno_ = RMCMD_ERR_OUTOFHOME;
            return 1;
        }

        if (file_remove(path) != 0) {
            strappfmt(self->what, sizeof self->what, "failed to remove \"%s\".", path);
            self->errno_ = RMCMD_ERR_REMOVE_FILE;
            return 1;
        }
    }

    return 0;
}

int
rmcmd_run(rmcmd_t *self) {
    if (self->argc < self->optind+1) {
        rmcmd_show_usage(self);
        return 0;
    }

    if (self->opts.is_help) {
        rmcmd_show_usage(self);
        return 0;        
    }

    if (!file_readline(self->parpath, sizeof self->parpath, self->config->var_cd_path)) {
        strappfmt(self->what, sizeof self->what, "failed to read line from cd of variable.");
        self->errno_ = RMCMD_ERR_READ_CD;
        return 1;
    }

    if (self->opts.is_recursive) {
        return rmcmd_rmr(self);
    } else {
        return rmcmd_rm(self);
    }

    return 0; // impossible
}
