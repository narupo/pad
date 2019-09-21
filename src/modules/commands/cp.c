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
    CPCMD_ERR_OPENDIR,
    CPCMD_ERR_MKDIR,
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
        "    cap cp [src] [dst dir] [options...]\n"
        "    cap cp [src] [src] [dst dir] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help         show usage\n"
        "    -r, --recursive    copy directory recursively\n"
        "\n"
        "Examples:\n"
        "\n"
        "    Copy from Cap's environment to user's environment.\n"
        "\n"
        "        $ cap cp :path/to/src.txt /path/to/dst.txt\n"
        "\n"
        "    Copy from user's environment to Cap's environment.\n"
        "\n"
        "        $ cap cp /path/to/src.txt :path/to/dst.txt\n"
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
        {0},
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
    char tmppath[FILE_NPATH];

    if (path[0] == ':') {
        // the path on the user's file system
        const char *pth = path+1;
        if (!file_solve(dst, dstsz, pth)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve path from \"%s\"", pth);
            return NULL;
        }
        return dst;
    }

    // the path on the Cap's environment
    const char *src_path = path;
    const char *org = NULL;

    if (src_path[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        err_die("impossible. invalid state in solve path");
    }

    snprintf(tmppath, sizeof tmppath, "%s/%s", org, src_path);
    if (!symlink_follow_path(self->config, dst, dstsz, tmppath)) {
        cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve path from \"%s\"", path);
        return NULL;
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

    file_close(dstfp);
    file_close(srcfp);
    return true;
}

static bool
cpcmd_copy_r(cpcmd_t *self, const char *dst_path, const char *src_path) {
    file_dir_t *dir = file_diropen(src_path);
    if (!dir) {
        cpcmd_set_err(self, CPCMD_ERR_OPENDIR, "failed to open directory \"%s\"", src_path);
        return false;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *fname = file_dirnodename(node);
        if (!strcmp(fname, ".") || !strcmp(fname, "..")) {
            continue;
        }

        char norm_src_path[FILE_NPATH];
        char tmppath[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", src_path, fname);
        if (!symlink_follow_path(self->config, norm_src_path, sizeof norm_src_path, tmppath)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve source path by \"%s\"", fname);
            goto fail;
        }

        char norm_dst_path[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", dst_path, fname);
        if (!symlink_follow_path(self->config, norm_dst_path, sizeof norm_dst_path, tmppath)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve destination path by \"%s\"", fname);
            goto fail;
        }

        if (file_isdir(norm_src_path)) {
            if (!file_isdir(norm_dst_path) && file_mkdirq(norm_dst_path) != 0) {
                cpcmd_set_err(self, CPCMD_ERR_MKDIR, "failed to make directory \"%s\"", norm_dst_path);
                goto fail;
            }
            if (!cpcmd_copy_r(self, norm_dst_path, norm_src_path)) {
                goto fail;
            }
        } else {
            cpcmd_copy_file(self, norm_dst_path, norm_src_path);
        }
    }

    file_dirclose(dir);
    return true;
fail:
    file_dirclose(dir);
    return false;
}

static bool
cpcmd_cp_src2dst_r(cpcmd_t *self, const char *dst_path, const char *src_path) {
    // dst_path が存在する場合
    //      dst_path がディレクトリの場合
    //          dst_path 以下に src の親ディレクトリごとコピーする
    //      dst_path がファイルの場合
    //          エラーを出す
    // dst_path が存在しない場合
    //      dst_path を mkdir
    //          作成したディレクトリ以下に src の親ディレクトリ以下をコピーする
    if (!file_isdir(src_path)) {
        cpcmd_set_err(self, CPCMD_ERR_COPY, "\"%s\" is not a directory (1)", src_path);
        return false;
    }

    if (file_exists(dst_path)) {
        if (file_isdir(dst_path)) {
            char basename[FILE_NPATH];
            file_basename(basename, sizeof basename, src_path);
            char dstdirpath[FILE_NPATH];
            file_solvefmt(dstdirpath, sizeof dstdirpath, "%s/%s", dst_path, basename);
            if (!file_exists(dstdirpath) && file_mkdirq(dstdirpath) != 0) {
                cpcmd_set_err(self, CPCMD_ERR_MKDIR, "failed to make directory \"%s\"", dstdirpath);
                return false;
            }
            return cpcmd_copy_r(self, dstdirpath, src_path);
        } else {
            cpcmd_set_err(self, CPCMD_ERR_COPY, "\"%s\" is not a directory (2)", dst_path);
            return false;            
        }
    } else {
        if (file_mkdirq(dst_path) != 0) {
            cpcmd_set_err(self, CPCMD_ERR_MKDIR, "failed to make directory \"%s\"", dst_path);
            return false;            
        }
        return cpcmd_copy_r(self, dst_path, src_path);
    }
}

static bool
cpcmd_cp_src2dst(cpcmd_t *self, const char *dst_path, const char *src_path) {
    if (file_isdir(src_path)) {
        if (self->opts.is_recursive) {
            return cpcmd_cp_src2dst_r(self, dst_path, src_path);
        } else {
            cpcmd_set_err(self, CPCMD_ERR_COPY, "omitting directory \"%s\"", src_path);
            return false;            
        }
    }

    if (file_isdir(dst_path)) {
        char basename[FILE_NPATH];
        if (!file_basename(basename, sizeof basename, src_path)) {
            cpcmd_set_err(self, CPCMD_ERR_BASENAME, "failed to get basename from \"%s\"", src_path);
            return false;
        }

        char newdstpath[FILE_NPATH];
        char tmppath[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", dst_path, basename);
        if (!symlink_follow_path(self->config, newdstpath, sizeof newdstpath, tmppath)) {
            cpcmd_set_err(self, CPCMD_ERR_SOLVEPATH, "failed to solve path from \"%s\"", basename);
            return false;            
        }

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
cpcmd_cp2(cpcmd_t *self, const char *to, const char *from) {
    char src_path[FILE_NPATH];
    if (!cpcmd_solve_path(self, src_path, sizeof src_path, from)) {
        return false;
    }
    if (from[0] != ':' && is_out_of_home(self->config->home_path, src_path)) {
        cpcmd_set_err(self, CPCMD_ERR_OUTOFHOME, "\"%s\" is out of home", from);
        return false;
    }

    char dst_path[FILE_NPATH];
    if (!cpcmd_solve_path(self, dst_path, sizeof dst_path, to)) {
        return false;
    }

    if (to[0] != ':' && is_out_of_home(self->config->home_path, dst_path)) {
        cpcmd_set_err(self, CPCMD_ERR_OUTOFHOME, "\"%s\" is out of home (2)", to);
        return false;
    }

    if (!cpcmd_cp_src2dst(self, dst_path, src_path)) {
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
        if (!cpcmd_cp2(self, to, from)) {
            return 1;
        }
    } else {
        const char *to = self->argv[self->argc-1];

        for (int i = self->optind; i < self->argc-1; ++i) {
            const char *from = self->argv[i];
            if (!cpcmd_cp2(self, to, from)) {
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

    int ret = cpcmd_cp(self);
    if (ret != 0) {
        err_error(self->what);
    }
    return ret;
}
