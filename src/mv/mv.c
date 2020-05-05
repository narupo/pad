#include <mv/mv.h>

struct opts {
    bool is_help;
};

struct mvcmd {
    const config_t *config;
    int argc;
    char **argv;
    int optind;
    struct opts opts;
};

static void
mvcmd_parse_opts(mvcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case '?':
        default: err_die("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        err_die("Failed to parse option");
    }

    self->optind = optind;
}

void
mvcmd_del(mvcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

mvcmd_t *
mvcmd_new(config_t *config, int argc, char **argv) {
    mvcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    mvcmd_parse_opts(self);

    return self;
}

static void
mvcmd_show_usage(mvcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap mv [old file] [new file] [options...]\n"
        "    cap mv [file] [dst dir] [options...]\n"
        "    cap mv [file] [file] [dst dir] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
}

static bool
mvcmd_mv_file_to_dir(mvcmd_t *self, const char *cap_path, const char *dirname) {
    char srcpath[FILE_NPATH];
    char dstpath[FILE_NPATH];
    char tmppath[FILE_NPATH*3];

    if (!solve_cmdline_arg_path(self->config, srcpath, sizeof srcpath, cap_path)) {
        err_error("failed to solve path for source file name");
        return false;
    }

    if (!file_exists(srcpath)) {
        err_error("\"%s\" is not exists", cap_path);
        return false;
    }

    char basename[FILE_NPATH];
    if (!file_basename(basename, sizeof basename, cap_path)) {
        err_error("failed to get basename from file name");
        return false;
    }

    snprintf(tmppath, sizeof tmppath, "%s/%s", dirname, basename);
    if (!solve_cmdline_arg_path(self->config, dstpath, sizeof dstpath, tmppath)) {
        err_error("failed to solve path for destination file name");
        return false;
    }

    if (file_rename(srcpath, dstpath) != 0) {
        err_error("failed to rename file \"%s\" to \"%s\" directory", srcpath, dstpath);
        return false;
    }

    return true;
}

static int
mvcmd_mv_files_to_dir(mvcmd_t *self) {
    const char *lastfname = self->argv[self->argc-1];

    for (int i = self->optind; i < self->argc-1; ++i) {
        const char *fname = self->argv[i];
        if (!mvcmd_mv_file_to_dir(self, fname, lastfname)) {
            err_error("failed to move file %s to %s", fname, lastfname);
            return 1;
        }
    }

    return 0;
}

static int
mvcmd_mv_file_to_other(mvcmd_t *self) {
    const char *src_cap_path = self->argv[self->optind];
    const char *dst_cap_path = self->argv[self->optind+1];

    char srcpath[FILE_NPATH];
    char tmppath[FILE_NPATH*2];

    if (!solve_cmdline_arg_path(self->config, srcpath, sizeof srcpath, src_cap_path)) {
        err_error("failed to follow path for source file name");
        return 1;
    }

    if (!file_exists(srcpath)) {
        err_error("\"%s\" is not exists. can not move to other", src_cap_path);
        return 1;
    }

    char dstpath[FILE_NPATH*2];

    if (!solve_cmdline_arg_path(self->config, dstpath, sizeof dstpath, dst_cap_path)) {
        err_error("failed to solve path for destination file name");
        return 1;
    }

    // remove last separate for stat
    const int dstpathlen = strlen(dstpath);
    if (dstpath[dstpathlen-1] == FILE_SEP) {
        dstpath[dstpathlen-1] = '\0';
    }

    // if dst path is directory then switch to process of directory
    if (file_isdir(dstpath)) {
        char basename[FILE_NPATH];

        if (src_cap_path[0] == ':') {
            src_cap_path += 1;
        }
        if (!file_basename(basename, sizeof basename, src_cap_path)) {
            err_error("failed to get basename in file to other");
            return 1;
        }

        char dstpath2[FILE_NPATH];
         snprintf(tmppath, sizeof tmppath, "%s/%s", dstpath, basename);
        if (!file_solve(dstpath2, sizeof dstpath2, tmppath)) {
            err_error("failed to follow path for second destination path in file to other");
            return 1;
        }

        if (file_rename(srcpath, dstpath2) != 0) {
            err_error("failed to rename \"%s\" to \"%s\"", srcpath, dstpath2);
            return 1;
        }
    } else {
        if (file_rename(srcpath, dstpath) != 0) {
            err_error("failed to rename \"%s\" to \"%s\" (2)", srcpath, dstpath);
            return 1;
        }
    }

    return 0;
}

static int
mvcmd_mv(mvcmd_t *self) {
    const int nargs = self->argc-self->optind;
    if (nargs >= 3) {
        return mvcmd_mv_files_to_dir(self);
    } else if (nargs == 2) {
        return mvcmd_mv_file_to_other(self);
    } else {
        err_error("not found destination");
        return 1;
    }
}

int
mvcmd_run(mvcmd_t *self) {
    if (self->argc < self->optind+2) {
        mvcmd_show_usage(self);
        return 0;
    }

    if (self->opts.is_help) {
        mvcmd_show_usage(self);
        return 0;
    }

    return mvcmd_mv(self);
}
