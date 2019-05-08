#include "modules/commands/mv.h"

struct opts {
    bool is_help;
};

struct mvcmd {
    config_t *config;
    int argc;
    char **argv;
    int optind;
    struct opts opts;
};

void
mvcmd_parse_opts(mvcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
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
    freeargv(self->argc, self->argv);
    config_del(self->config);
    free(self);
}

mvcmd_t *
mvcmd_new(config_t *move_config, int argc, char **move_argv) {
    mvcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    mvcmd_parse_opts(self);

    return self;
}

void
mvcmd_show_usage(mvcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap mv [old] [new] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
}

int 
mvcmd_mv(mvcmd_t *self) {
    const char *old = self->argv[self->optind];
    const char *new = self->argv[self->optind+1];
    const char *org;
    char oldpath[FILE_NPATH];
    char newpath[FILE_NPATH];

    if (old[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        err_die("impossible. invalid state in mv");
    }

    if (!file_solvefmt(oldpath, sizeof oldpath, "%s/%s", org, old)) {
        err_error("failed to solve path for old");
        return 2;
    }

    if (new[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        err_die("impossible. invalid state in mv (2)");
    }

    if (!file_solvefmt(newpath, sizeof newpath, "%s/%s", org, new)) {
        err_error("failed to solve path for new");
        return 3;
    }

    if (!file_exists(oldpath)) {
        err_error("\"%s\" is not exists", oldpath);
        return 4;
    }

    if (file_exists(newpath)) {
        if (file_remove(newpath) != 0) {
            err_error("failed to remove file \"%s\"", newpath);
            return 5;
        }
    }

    if (file_rename(oldpath, newpath) != 0) {
        err_error("failed to rename \"%s\" to \"%s\"", oldpath, newpath);
        return 6;
    }

    return 0;
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
