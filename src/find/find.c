#include "find/find.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
    bool is_normalize;
    bool is_alias;
    char origin[FILE_NPATH];
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
    argsmgr_t *argsmgr;
    almgr_t *almgr;  
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
        "    -h, --help         show usage\n"
        "    -n, --normalize    normalize path\n"
        "    -o, --origin       origin path\n"
        "    -a, --alias        find aliases\n"
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
        {"alias", no_argument, 0, 'a'},
        {"origin", required_argument, 0, 'o'},
        {0},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hnao:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'n': self->opts.is_normalize = true; break;
        case 'a': self->opts.is_alias = true; break;
        case 'o': snprintf(self->opts.origin, sizeof self->opts.origin, "%s", optarg); break;
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
    almgr_del(self->almgr);
    free(self);
}

findcmd_t *
findcmd_new(const config_t *config, int argc, char **argv) {
    findcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    strcpy(self->opts.origin, ".");
    self->almgr = almgr_new(self->config);

    if (!findcmd_parse_opts(self)) {
        findcmd_del(self);
        return NULL;
    }

    self->argsmgr = argsmgr_new(self->argv + self->optind);

    return self;
}

static void
join_cap_path(char *dst, int32_t dstsz, const char *lhs, const char *rhs) {
    for (; *rhs == '/' ; ++rhs);
    bool sep = lhs[strlen(lhs)-1] == '/' ? false : true;
    if (sep) {
        snprintf(dst, dstsz, "%s/%s", lhs, rhs);
    } else {
        snprintf(dst, dstsz, "%s%s", lhs, rhs);
    }
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
        join_cap_path(cap_path, sizeof cap_path, cap_dirpath, name);

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

static bool
has_contents(const findcmd_t *self, const dict_t *kvmap, int32_t *maxkeylen, int32_t *maxvallen) {
    bool has = false;
    for (int32_t i = 0; i < dict_len(kvmap); ++i) {
        const dict_item_t *item = dict_getc_index(kvmap, i);
        if (argsmgr_contains_all(self->argsmgr, item->key)) {
            int32_t keylen = strlen(item->key);
            int32_t vallen = strlen(item->value);
            *maxkeylen = keylen > *maxkeylen ? keylen : *maxkeylen;
            *maxvallen = vallen > *maxvallen ? vallen : *maxvallen;
            has = true;
        }
    }

    return has;
}

static int
find_aliases_r(const findcmd_t *self, const char *dirpath, const char *cap_dirpath) {
    char alpath[FILE_NPATH];
    if (!file_solvefmt(alpath, sizeof alpath, "%s/.caprc", dirpath)) {
        // not error
    }

    almgr_clear(self->almgr);
    if (file_exists(alpath)) {
        if (!almgr_load_path(self->almgr, alpath)) {
            err_error("failed to load resource file \"%s\" for alias", alpath);
            return 1;
        }
    }

    const context_t *ctx = almgr_getc_context(self->almgr);
    const alinfo_t *alinfo = ctx_getc_alinfo(ctx);
    const dict_t *kvmap = alinfo_getc_key_value_map(alinfo);
    int32_t maxkeylen = 0;
    int32_t maxvallen = 0;
    bool hascontents = has_contents(self, kvmap, &maxkeylen, &maxvallen);
    const char *disppath = self->opts.is_normalize ? dirpath : cap_dirpath;
    disppath = strlen(disppath) ? disppath : ".";

    if (dict_len(kvmap)) {
        if (hascontents) {
            printf("%s\n\n", disppath);
        }
    }

    for (int32_t i = 0; i < dict_len(kvmap); ++i) {
        const dict_item_t *item = dict_getc_index(kvmap, i);
        if (argsmgr_contains_all(self->argsmgr, item->key)) {
            printf("    %-*s    %-*s\n", maxkeylen, item->key, maxvallen, item->value);
        }
    }

    if (hascontents) {
        printf("\n");
    }

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
        join_cap_path(cap_path, sizeof cap_path, cap_dirpath, name);

        char tmp_path[FILE_NPATH];
        snprintf(tmp_path, sizeof tmp_path, "%s/%s", dirpath, name);

        char path[FILE_NPATH];
        if (!symlink_follow_path(self->config, path, sizeof path, tmp_path)) {
            err_error("failed to follow path on find file recursive");
            file_dirnodedel(node);
            continue;
        }

        if (file_isdir(path)) {
            ret = find_aliases_r(self, path, cap_path);
        }

        file_dirnodedel(node);
    }

    return ret;
}

static int
find_start(const findcmd_t *self) {
    const char *origin = get_origin(self->config, self->opts.origin);
    char tmppath[FILE_NPATH*2];
    snprintf(tmppath, sizeof tmppath, "%s/%s", origin, self->opts.origin);

    char path[FILE_NPATH];
    if (!symlink_follow_path(self->config, path, sizeof path, tmppath)) {
        err_error("failed to follow path in find files");
        return 1;
    }
    
    if (self->opts.is_alias) {
        return find_aliases_r(self, path, self->opts.origin);
    }

    return find_files_r(self, path, self->opts.origin);
}

int
findcmd_run(findcmd_t *self) {
    int nargs = self->argc - self->optind;

    if (nargs == 0) {
        findcmd_show_usage(self);
        return 0;
    }

    return find_start(self);
}
