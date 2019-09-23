#include "snippet/snippet.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
    bool is_add;
    bool is_clear;
};

/**
 * Structure of command
 */
struct snptcmd {
    config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to snptcmd_t
 */
static void
snptcmd_show_usage(snptcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Save or show snippet codes.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap snippet [name] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "    -a, --add     add snippet code\n"
        "    -c, --clear   clear all snippet codes\n"
        "\n"
    );
    fflush(stderr);
}

/**
 * Parse options
 *
 * @param[in] self pointer to snptcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
snptcmd_parse_opts(snptcmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"add", no_argument, 0, 'a'},
        {"clear", no_argument, 0, 'c'},
        {0},
    };

    self->opts = (struct opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hac", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'a': self->opts.is_add = true; break;
        case 'c': self->opts.is_clear = true; break;
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
snptcmd_del(snptcmd_t *self) {
    if (!self) {
        return;
    }
    config_del(self->config);
    freeargv(self->argc, self->argv);
    free(self);
}

snptcmd_t *
snptcmd_new(config_t *move_config, int argc, char **move_argv) {
    snptcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    if (!snptcmd_parse_opts(self)) {
        snptcmd_del(self);
        return NULL;
    }

    return self;
}

static int
snptcmd_show_files(snptcmd_t *self) {
    file_dir_t *dir = file_diropen(self->config->codes_dir_path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *name = file_dirnodename(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        puts(name);
        file_dirnodedel(node);
    }

    file_dirclose(dir);
    return 0;
}

static int
snptcmd_add(snptcmd_t *self) {
    const char *name = self->argv[self->optind];
    char path[FILE_NPATH];

    if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        err_error("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fout = file_open(path, "wb");
    if (!fout) {
        err_error("failed to open snippet \"%s\"", name);
        return 1;
    }

    for (int c; (c = fgetc(stdin)) != EOF; ) {
        fputc(c, fout);
    }

    fflush(fout);
    file_close(fout);
    return 0;
}

static int 
snptcmd_show(snptcmd_t *self) {
    const char *name = self->argv[self->optind];
    char path[FILE_NPATH];

    if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        err_error("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fin = file_open(path, "rb");
    if (!fin) {
        err_error("failed to open snippet \"%s\"", name);
        return 1;
    }

    char *content = file_readcp(fin);
    if (!content) {
        err_error("failed to read snippet code from \"%s\"", path);
        return 1;
    }

    printf("%s", content);
    free(content);
    return 0;
}

static int
snptcmd_clear(snptcmd_t *self) {
    file_dir_t *dir = file_diropen(self->config->codes_dir_path);
    if (!dir) {
        err_error("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (file_dirnode_t *node; (node = file_dirread(dir)); ) {
        const char *name = file_dirnodename(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        char path[FILE_NPATH];
        if (!file_solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
            err_error("failed to solve path for \"%s\"", name);
            goto fail;
        }

        if (file_remove(path) != 0) {
            err_error("failed to remove file \"%s\"", path);
            goto fail;
        }

        file_dirnodedel(node);
    }

fail:
    file_dirclose(dir);
    return 0;
}

int
snptcmd_run(snptcmd_t *self) {
    if (self->opts.is_help) {
        snptcmd_show_usage(self);
        return 0;
    } else if (self->opts.is_clear) {
        return snptcmd_clear(self);
    } else if (self->argc < self->optind+1) {
        return snptcmd_show_files(self);
    } else if (self->opts.is_add) {
        return snptcmd_add(self);
    }
    return snptcmd_show(self);
}
