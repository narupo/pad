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
    struct cap_array *args;
};

static void
optsdel(struct opts *self) {
    if (self) {
        cap_arrdel(self->args);
        free(self);
    }
}

static struct opts *
optsnew(int argc, char *argv[]) {
    struct opts *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->args = cap_arrnew();
    if (!self->args) {
        free(self);
        return NULL;
    }

    for (int32_t i = 0; i < argc; ++i) {
        cap_arrpush(self->args, argv[i]);
    }

    return self;
}

struct app {
    struct opts *opts;
};

static void
appdel(struct app *self) {
    if (self) {
        optsdel(self->opts);
        free(self);
    }
}

static struct app *
appnew(int argc, char *argv[]) {
    struct app *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = optsnew(argc, argv);
    if (!self->opts) {
        free(self);
        return NULL;
    }

    return self;
}

static int32_t
appmain(struct app *self) {
    if (cap_arrlen(self->opts->args) < 1) {
        cap_error("need script name");
        return 1;
    }

    const char *scope = getenv("CAP_SCOPE");
    if (!scope) {
        cap_error("not found scope in environment");
        return 2;
    }

    char cdorhome[FILE_NPATH];
    if (strcasecmp(scope, "local") == 0) {
        if (!cap_envget(cdorhome, sizeof cdorhome, "CAP_VARCD")) {
            cap_error("need environment variable of cd");
            return 3;
        }
    } else if (strcasecmp(scope, "global") == 0) {
        if (!cap_envget(cdorhome, sizeof cdorhome, "CAP_VARHOME")) {
            cap_error("need environment variable of home");
            return 4;
        }
    } else {
        cap_error("invalid scope \"%s\"", scope);
        return 5;
    }

    // Create script path
    char spath[FILE_NPATH];
    cap_fsolvefmt(spath, sizeof spath, "%s/%s", cdorhome, cap_arrgetc(self->opts->args, 1));
    if (isoutofhome(spath)) {
        cap_die("invalid script. '%s' is out of home.", spath);
    }

    // Read script line in file
    char exesname[NSCRIPTNAME];
    readscriptline(exesname, sizeof exesname, spath);

    // Create command line
    struct cap_string *cmdline = cap_strnew();
    cap_strapp(cmdline, exesname);
    cap_strapp(cmdline, " ");
    cap_strapp(cmdline, spath);
    cap_strapp(cmdline, " ");

    for (int32_t i = 2; i < cap_arrlen(self->opts->args); ++i) {
        cap_strapp(cmdline, cap_arrgetc(self->opts->args, i));
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
