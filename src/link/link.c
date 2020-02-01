#include "link/link.h"

/**
 * Structure of options
 */
struct opts {
    bool is_help;
    bool is_unlink;
};

/**
 * Structure of command
 */
struct linkcmd {
    const config_t *config;
    int argc;
    char **argv;
    struct opts opts;
    int optind;
};

/**
 * Parse options
 * 
 * @param[in] *self   
 * @param[in] argc    
 * @param[in] *argv[] 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
linkcmd_t *
linkcmd_parse_opts(linkcmd_t *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"unlink", no_argument, 0, 'u'},
        {0},
    };

    self->opts = (struct opts){
        .is_help = false,
        .is_unlink = false,
    };
    opterr = 0;
    optind = 0;

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hu", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'u': self->opts.is_unlink = true; break;
        case '?':
        default:
            err_die("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        return NULL;
    }

    self->optind = optind;

    return self;
}

void
linkcmd_del(linkcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

linkcmd_t *
linkcmd_new(const config_t *config, int argc, char **argv) {
    linkcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!linkcmd_parse_opts(self)) {
        linkcmd_del(self);
        return NULL;
    }

    return self;
}

void
linkcmd_usage(const linkcmd_t *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap link [options] [link-name] [cap-path]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -u, --unlink     unlink link.\n"
        "\n"
        "Examples:\n"
        "\n"
        "    $ cap link mylink /path/to/file\n"
        "    $ cap link mylink /path/to/dir\n"
        "    $ cap link -u mylink\n"
        "\n"
    );
}

int
linkcmd_unlink(linkcmd_t *self) {
    if (self->argc-self->optind < 1) {
        linkcmd_usage(self);
        return 1;
    }

    const char *linkname = self->argv[self->optind];
    const char *org = get_origin(self->config, linkname);

    char path[FILE_NPATH];
    if (!file_solvefmt(path, sizeof path, "%s/%s", org, linkname)) {
        err_error("failed to solve path");
        return 1;
    }

    if (is_out_of_home(self->config->home_path, path)) {
        err_error("\"%s\" is out of home", linkname);
        return 1;
    }

    if (!symlink_is_link_file(path)) {
        err_error("\"%s\" is not Cap's symbolic link", linkname);
        return 1;
    }

    if (file_remove(path) != 0) {
        err_error("failed to unlink");
        return 1;
    }

    return 0;
}

int
linkcmd_link(linkcmd_t *self) {
    if (self->argc-self->optind < 2) {
        linkcmd_usage(self);
        return 1;
    }

    const char *linkname = self->argv[self->optind];
    if (strstr(linkname, "..")) {
        err_error("Cap's symbolic link is not allow relative path");
        return 1;
    }

    const char *cappath = self->argv[self->optind+1];
    const char *org = get_origin(self->config, linkname);

    char dstpath[FILE_NPATH];
    if (!file_solvefmt(dstpath, sizeof dstpath, "%s/%s", org, linkname)) {
        err_error("failed to solve path");
        return 1;
    }

    if (is_out_of_home_no_exists(self->config->home_path, dstpath)) {
        err_error("\"%s\" is out of home", linkname);
        return 1;
    }

    char line[FILE_NPATH + 100];
    snprintf(line, sizeof line, "%s %s", SYMLINK_HEADER, cappath);
    if (!file_writeline(line, dstpath)) {
        err_error("failed to create link");
        return 1;        
    }

    return 0;
}

int
linkcmd_run(linkcmd_t *self) {
    if (self->opts.is_help) {
        linkcmd_usage(self);
        return 0;
    }

    if (self->opts.is_unlink) {
        return linkcmd_unlink(self);
    }

    return linkcmd_link(self);
}
