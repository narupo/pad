#include "modules/commands/mkdir.h"

extern int opterr;
extern int optind;

struct opts {
    bool is_help;
    bool is_parents;
};

struct mkdircmd {
    config_t *config;
    int argc;
    char **argv;
    struct opts opts;
    int optind;
};

void
mkdircmd_parse_opts(mkdircmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"parents", no_argument, 0, 'p'},
        {},
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse
    self->opts = (struct opts){0};

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hp", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'p': self->opts.is_parents = true; break;
        case '?':
        default:
            err_die("unsupported option");
            break;
        }
    }

    if (self->argc < optind) {
        err_die("Failed to parse option");
    }

    self->optind = optind;
}

void
mkdircmd_del(mkdircmd_t *self) {
    if (!self) {
        return;
    }
    freeargv(self->argc, self->argv);
    config_del(self->config);
    free(self);
}

mkdircmd_t *
mkdircmd_new(config_t *move_config, int argc, char **move_argv) {
    mkdircmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    mkdircmd_parse_opts(self);

    return self;
}

void
mkdircmd_show_usage(mkdircmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap mkdir [path] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage\n"
        "    -p, --parents    not error if existing, make parent directories as needed\n"
        "\n"
    );
    fflush(stderr);
}

int
mkdircmd_mkdirp(mkdircmd_t *self) {
    const char *argpath = self->argv[self->optind];
    const char* org = get_origin(self->config, argpath);
    char path[FILE_NPATH];

    char tmppath[FILE_NPATH];
    snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

    if (!symlink_follow_path(self->config, path, sizeof path, tmppath)) {
        err_error("failed to follow path");
        return 1;
    }

    if (file_mkdirsq(path) != 0) {
        err_error("failed to create directory \"%s\"", path);
        return 1;
    }

    return 0;
}

int
mkdircmd_mkdir(mkdircmd_t *self) {
    const char *argpath = self->argv[self->optind];
    const char *org = get_origin(self->config, argpath);
    char path[FILE_NPATH];

    char tmppath[FILE_NPATH];
    snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

    if (!symlink_follow_path(self->config, path, sizeof path, tmppath)) {
        err_error("failed to follow path");
        return 1;
    }

    if (file_exists(path)) {
        err_error("failed to create directory. \"%s\" is exists", path);
        return 1;
    }

    if (file_mkdirq(path) != 0) {
        err_error("failed to create directory \"%s\"", path);
        return 1;
    }

    return 0;
}

int
mkdircmd_run(mkdircmd_t *self) {
    if (self->opts.is_help) {
        mkdircmd_show_usage(self);
        return 0;        
    }

    if (self->argc < self->optind+1) {
        mkdircmd_show_usage(self);
        return 0;
    }

    if (self->opts.is_parents) {
        return mkdircmd_mkdirp(self);
    } else {
        return mkdircmd_mkdir(self);
    }

    return 0; // impossible
}
