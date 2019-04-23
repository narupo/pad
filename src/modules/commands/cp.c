#include "modules/commands/cp.h"

/**
 * Numbers
 */
typedef enum {
    CPCMD_ERR_NOERR = 0,
    CPCMD_ERR_SOLVEPATH,
    CPCMD_ERR_OUTOFHOME,
    CPCMD_ERR_COPY,
    CPCMD_ERR_OPENFILE,
    CPCMD_ERR_BASENAME,
} cpcmd_errno_t;

/**
 * Structure of options
 */
struct opts {
    bool is_help;
    bool is_recursive;
};

/**
 * Structure of command
 */
struct cpcmd {
    config_t *config;
    int argc;
    int optind;
    cpcmd_errno_t errno_;
    char **argv;
    struct opts opts;
    char what[2048];
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to cpcmd_t
 */
static void
cpcmd_show_usage(cpcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Copy files.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap cp [src] [dst] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help         show usage\n"
        "    -r, --recursive    copy directory recursively\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to cpcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
cpcmd_parse_opts(cpcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"recusive", required_argument, 0, 'r'},
        {},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
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
            err_die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
cpcmd_del(cpcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

cpcmd_t *
cpcmd_new(config_t *move_config, int argc, char **move_argv) {
    cpcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!cpcmd_parse_opts(self)) {
        cpcmd_del(self);
        return NULL;
    }

    return self;
}

static void
cpcmd_set_err(cpcmd_t *self, cpcmd_errno_t errno_, const char *fmt, ...) {
    self->errno_ = errno_;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->what, sizeof self->what, fmt, ap);
    va_end(ap);
}

static char *
cpcmd_solve_path(cpcmd_t *self, char *dst, size_t dstsz, const char *path) {
    if (path[0] == ':') {
        // the path on the Cap's environment
        const char *src_path = path+1;
        const char *pardir = self->config->cd_path;
        if (!file_solvefmt(dst, dstsz, "%s/%s", pardir, src_path)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve path from \"%s\"", path);
            return NULL;
        }
    } else {
        // the path on the user's file system
        if (!file_solve(dst, dstsz, path)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve path from \"%s\"", path);
            return NULL;
        }
    }

    return dst;
}

static bool
cpcmd_copy_file(cpcmd_t *self, const char *dst_path, const char *src_path) {
    FILE *dstfp = file_open(dst_path, "wb");
    if (!dstfp) {
        cpcmd_set_err(self, CPCMD_ERR_OPENFILE, "failed to open destination file \"%s\"", dst_path);
        return false;
    }
    FILE *srcfp = file_open(src_path, "rb");
    if (!srcfp) {
        cpcmd_set_err(self, CPCMD_ERR_OPENFILE, "failed to open source file \"%s\"", src_path);
        return false;
    }
    if (!file_copy(dstfp, srcfp)) {
        cpcmd_set_err(self, CPCMD_ERR_COPY, "failed to copy file from \"%s\" to \"%s\"", src_path, dst_path);
        return false;
    }
    file_close(srcfp);
    file_close(dstfp);
    return true;
}

static bool
cpcmd_cp_src2dst(cpcmd_t *self, const char *src_path, const char *dst_path) {
    if (file_isdir(src_path)) {
        cpcmd_set_err(self, CPCMD_ERR_COPY, "omitting directory \"%s\"", src_path);
        return false;
    }

    if (file_isdir(dst_path)) {
        char basename[FILE_NPATH];
        if (!file_basename(basename, sizeof basename, src_path)) {
            cpcmd_set_err(self, CPCMD_ERR_BASENAME, "failed to get basename from \"%s\"", src_path);
            return false;
        }

        char newdstpath[FILE_NPATH];
        file_solvefmt(newdstpath, sizeof newdstpath, "%s/%s", dst_path, basename);

        if (!cpcmd_copy_file(self, newdstpath, src_path)) {
            return false;
        }
    } else {
        if (!cpcmd_copy_file(self, dst_path, src_path)) {
            return false;
        }
    }

    return true;
}

static bool
cpcmd_cp2(cpcmd_t *self, const char *from, const char *to) {
    char src_path[FILE_NPATH];
    if (!cpcmd_solve_path(self, src_path, sizeof src_path, from)) {
        return false;
    }

    char dst_path[FILE_NPATH];
    if (!cpcmd_solve_path(self, dst_path, sizeof dst_path, to)) {
        return false;
    }

    if (!cpcmd_cp_src2dst(self, src_path, dst_path)) {
        switch (self->errno_) {
        case CPCMD_ERR_NOERR: break;
        default: err_error(self->what); break;
        }
        return false;
    }

    return true;
}

static int
cpcmd_cp(cpcmd_t *self) {
    int nargs = self->argc - self->optind;
    if (nargs == 2) {
        const char *from = self->argv[self->optind];
        const char *to = self->argv[self->optind+1];
        if (!cpcmd_cp2(self, from, to)) {
            return 1;
        }
    } else {
        const char *to = self->argv[self->argc-1];
        for (int i = self->optind; i < self->argc-1; ++i) {
            const char *from = self->argv[i];
            if (!cpcmd_cp2(self, from, to)) {
                return 1;
            }
        }
    }
    return 0;
}

int
cpcmd_run(cpcmd_t *self) {
    if (self->argc < self->optind+2) {
        cpcmd_show_usage(self);
        return 1;
    }

    if (self->opts.is_help) {
        cpcmd_show_usage(self);
        return 1;
    }

    return cpcmd_cp(self);
}
