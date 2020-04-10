#include "find/find.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
    bool is_normalize;
};

/**
 * Structure of command
 */
struct find {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    arguments_manager_t *argsmgr;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to findcmd_t
 */
static void
findcmd_show_usage(findcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap find [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help         Show usage\n"
        "    -n, --normalize    Normalize path\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to findcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
findcmd_parse_opts(findcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {0},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hn", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'n': self->opts.is_normalize = true; break;
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
findcmd_del(findcmd_t *self) {
    if (!self) {
        return;
    }

    argsmgr_del(self->argsmgr);
    free(self);
}

findcmd_t *
findcmd_new(const config_t *config, int argc, char **argv) {
    findcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!findcmd_parse_opts(self)) {
        findcmd_del(self);
        return NULL;
    }

    self->argsmgr = argsmgr_new(self->argv + self->optind);

    return self;
}

static int
find_files_r(const findcmd_t *self, const char *dirpath, const char *cap_dirpath) {
    file_dir_t *dir = file_diropen(dirpath);
    if (!dir) {
        err_error("failed to open directory \"%s\"", dirpath);
        return 1;
    }

    int ret = 0;

    for (;;) {
        file_dirnode_t *node = file_dirread(dir);
        if (!node) {
            break;
        }

        const char *name = file_dirnodename(node);
        if (cstr_eq(name, ".") || cstr_eq(name, "..")) {
            file_dirnodedel(node);
            continue;
        }

        char cap_path[FILE_NPATH];
        snprintf(cap_path, sizeof cap_path, "%s/%s", cap_dirpath, name);

        char tmp_path[FILE_NPATH];
        snprintf(tmp_path, sizeof tmp_path, "%s/%s", dirpath, name);

        char path[FILE_NPATH];
        if (!symlink_follow_path(self->config, path, sizeof path, tmp_path)) {
            err_error("failed to follow path on find file recursive");
            file_dirnodedel(node);
            continue;
        }

        if (argsmgr_contains_all(self->argsmgr, name)) {
            if (self->opts.is_normalize) {
                puts(path);
            } else {
                puts(cap_path);
            }
        }

        if (file_isdir(path)) {
            ret = find_files_r(self, path, cap_path);
        }

        file_dirnodedel(node);
    }

    file_dirclose(dir);

    return ret;
}

static int
find_files(const findcmd_t *self) {
    return find_files_r(self, self->config->cd_path, ".");
}

int
findcmd_run(findcmd_t *self) {
    int nargs = self->argc - self->optind;

    if (nargs == 0) {
        findcmd_show_usage(self);
        return 0;
    }

    return find_files(self);
}
