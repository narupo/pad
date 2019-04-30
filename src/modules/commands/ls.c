#include "modules/commands/ls.h"

struct opts {
    bool ishelp;
    bool isall;
};

struct lscmd {
    config_t *config;
    int argc;
    char **argv;
    struct opts opts;
};

void
lscmd_del(lscmd_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        free(self);
    }
}

static bool
lscmd_parse_opts(lscmd_t *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"all", no_argument, 0, 'a'},
        {},
    };
    
    self->opts = (struct opts){0};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->opts.ishelp = true; break;
        case 'a': self->opts.isall = true; break;
        case '?':
        default:
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        return false;
    }

    return true;
}

static void
lscmd_usage(const lscmd_t *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap ls [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage.\n"
        "    -a, --all     show all files.\n"
        "\n"
    );
}

lscmd_t *
lscmd_new(config_t *move_config, int argc, char **move_argv) {
    lscmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!lscmd_parse_opts(self)) {
        lscmd_del(self);
        return NULL;
    }

    return self;
}

static void
lscmd_arrdump(const lscmd_t *_, const cstring_array_t *arr, FILE *fout) {
    for (int i = 0; i < cstrarr_len(arr); ++i) {
        fprintf(fout, "%s\n", cstrarr_getc(arr, i));
    }
    fflush(fout);
}

static bool
lscmd_isdotfile(const lscmd_t *_, const char *fname) {
    if (strcmp(fname, "..") == 0 ||
        fname[0] == '.') {
        return true;
    }

    return false;
}

static cstring_array_t *
lscmd_dir2arr(const lscmd_t *self, file_dir_t *dir) {
    cstring_array_t *arr = cstrarr_new();
    if (!arr) {
        return NULL;
    }

    for (file_dirnode_t *nd; (nd = file_dirread(dir)); ) {
        const char *name = file_dirnodename(nd);
        if (lscmd_isdotfile(self, name) && !self->opts.isall) {
            continue;            
        }
        cstrarr_push(arr, name);
        file_dirnodedel(nd);
    }

    return arr;
}

static int
lscmd_ls(const lscmd_t *self, const char *path) {
    if (isoutofhome(self->config->var_home_path, path)) {
        err_error("\"%s\" is out of home", path);
        return 1;
    }
    
    file_dir_t *dir = file_diropen(path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", path);
        return 2;
    }

    cstring_array_t *arr = lscmd_dir2arr(self, dir);
    if (!arr) {
        err_error("failed to read directory \"%s\"", path);
        return 3;
    }

    cstrarr_sort(arr);
    lscmd_arrdump(self, arr, stdout);
    cstrarr_del(arr);

    if (file_dirclose(dir) < 0) {
        err_error("failed to close directory \"%s\"", path);
        return 4;
    }

    return 0;
}

int
lscmd_run(lscmd_t *self) {
    if (self->opts.ishelp) {
        lscmd_usage(self);
        return 0;
    }

    char cd[FILE_NPATH];
    if (!file_readline(cd, sizeof cd, self->config->var_cd_path)) {
        err_error("failed to read line from cd of variable");
        return 1;
    }

    char home[FILE_NPATH];
    if (!file_readline(home, sizeof home, self->config->var_home_path)) {
        err_error("failed to read line from home of variable");
        return 2;
    }

    if (optind - self->argc == 0) {
        return lscmd_ls(self, cd);
    } else {
        char path[FILE_NPATH];

        for (int i = optind; i < self->argc; ++i) {
            const char *arg = self->argv[i];
            const char *org = (arg[0] == '/' ? home : cd);
            file_solvefmt(path, sizeof path, "%s/%s", org, arg);
            lscmd_ls(self, path);
        }
    }

    return 0;
}
