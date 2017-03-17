/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-run.h"

enum {
	NSCRIPTNAME = 100,
	NCMDLINE = 256
};

static char *
readscriptline(char *dst, size_t dstsz, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return NULL;
	}

	char tmp[dstsz];
	cap_fgetline(tmp, sizeof tmp, fin);

	const char *needle = "!";
	char *at = strstr(tmp, needle);
	if (!at) {
		fclose(fin);
		return NULL;
	}

	snprintf(dst, dstsz, "%s", at + strlen(needle));

	if (fclose(fin) < 0) {
		return NULL;
	}

	return dst;
}

struct opts {
    int argc;
    char **argv;
    bool ishelp;
};

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
    };

    extern int opterr;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->ishelp = true; break;
        case '?':
        default: cap_error("Unknown option \"%s\"", cur); break;
        }
    }

    if (argc < optind) {
        cap_error("Failed to parse option");
        return false;
    }

    self->argc = argc;
    self->argv = argv;

    return true;
}

struct app {
    struct opts opts;
};

static void
appdel(struct app *self) {
    if (self) {
        free(self);
    }
}

static struct app *
appnew(int argc, char *argv[]) {
    struct app *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    if (!optsparse(&self->opts, argc, argv)) {
        free(self);
        return NULL;
    }

    return self;
}

static int32_t
appmain(struct app *self) {
    if (self->opts.argc < 2) {
        cap_error("need script name");
        return 1;
    }

    const char *scope = getenv("CAP_SCOPE");
    if (!scope) {
        cap_error("not found scope in environment");
        return 1;
    }

    char cdorhome[FILE_NPATH];
    if (strcasecmp(scope, "local") == 0) {
        if (!cap_envget(cdorhome, sizeof cdorhome, "CAP_VARCD")) {
            cap_error("need environment variable of cd");
            return 1;
        }
    } else if (strcasecmp(scope, "global") == 0) {
        if (!cap_envget(cdorhome, sizeof cdorhome, "CAP_VARHOME")) {
            cap_error("need environment variable of home");
            return 1;
        }
    } else {
        cap_error("invalid scope \"%s\"", scope);
        return 1;
    }

    char spath[FILE_NPATH]; // Script path
    cap_fsolvefmt(spath, sizeof spath, "%s/%s", cdorhome, self->opts.argv[1]);
    if (isoutofhome(spath)) {
        cap_die("invalid script. '%s' is out of home.", spath);
    }

    char exesname[NSCRIPTNAME]; // Execute script name in file
    readscriptline(exesname, sizeof exesname, spath);

    struct cap_string *cmdline = cap_strnew();
    cap_strapp(cmdline, exesname);
    cap_strapp(cmdline, " ");
    cap_strapp(cmdline, spath);
    cap_strapp(cmdline, " ");
    for (int i = 2; i < self->opts.argc; ++i) {
        cap_strapp(cmdline, self->opts.argv[i]);
        cap_strapp(cmdline, " ");
    }

    // Start process communication
    safesystem(cap_strgetc(cmdline));
    
    // Done
    cap_strdel(cmdline);
    return 0;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap run");

    struct app *app = appnew(argc, argv);
    if (!app) {
        cap_error("failed to create application");
        return 1;
    }

    int32_t ret = appmain(app);
    appdel(app);

    return ret;
}
