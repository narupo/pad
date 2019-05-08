#include "modules/commands/run.h"

enum {
    NSCRIPTNAME = 100,
    NCMDLINE = 256
};

struct runcmd {
    config_t *config;
    int argc;
    char **argv;
};

void
runcmd_del(runcmd_t *self) {
    if (self) {
        config_del(self->config);
        freeargv(self->argc, self->argv);
        free(self);
    }
}

runcmd_t *
runcmd_new(config_t *move_config, int argc, char **move_argv) {
    runcmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = move_config;
    self->argc = argc;
    self->argv = move_argv;

    return self;
}

static char *
runcmd_read_script_line(runcmd_t *self, char *dst, size_t dstsz, const char *path) {
    // read first line of file
    FILE *fin = fopen(path, "rb");
    if (!fin) {
        return NULL;
    }

    char tmp[dstsz];
    file_getline(tmp, sizeof tmp, fin);
    cstr_pop_newline(tmp);

    // find !
    const char *needle = "!";
    char *at = strstr(tmp, needle);
    if (!at) {
        fclose(fin);
        return NULL;
    }
    at = at + strlen(needle);

#ifdef _CAP_WINDOWS
    // fix script path for Windows (trim execute file name only, remove directories)
    char *last = at+strlen(at)-1;
    char *beg = at;
    for (char *p = last; p >= at; --p){
        if (*p == '/') {
            beg = p+1;
            break;
        }
    }
    at = beg;
#endif

    // copy path
    snprintf(dst, dstsz, "%s", at);

    // done
    if (fclose(fin) == EOF) {
        return NULL;
    }

    return dst;
}

int
runcmd_run(runcmd_t *self) {
    if (self->argc < 2) {
        err_error("need script file name");
        return 1;
    }

    const char *argpath = self->argv[1];
    const char *org;

    if (argpath[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        err_die("impossible. invalid state in run");
    }

    // Create script path
    char spath[FILE_NPATH];
    file_solvefmt(spath, sizeof spath, "%s/%s", org, argpath);
    if (isoutofhome(self->config->var_home_path, spath)) {
        err_error("invalid script. \"%s\" is out of home.", spath);
        return 6;
    }

    // Read script line in file
    char script[NSCRIPTNAME];
    if (!runcmd_read_script_line(self, script, sizeof script, spath)) {
        script[0] = '\0';
    }

    // Create command line
    string_t *cmdline = str_new();
    if (strlen(script)) {
        str_app(cmdline, script);
        str_app(cmdline, " ");
    }
    str_app(cmdline, spath);
    str_app(cmdline, " ");

    for (int32_t i = 2; i < self->argc; ++i) {
        str_app(cmdline, "\"");
        str_app(cmdline, self->argv[i]);
        str_app(cmdline, "\"");
        str_app(cmdline, " ");
    }
    str_popb(cmdline);

    // Start process communication
    int option = SAFESYSTEM_DEFAULT;
    const char *dodetach = getenv("CAP_RUN_DETACH");
    if (dodetach && dodetach[0] == '1') {
        option |= SAFESYSTEM_DETACH;
    }

    safesystem(str_getc(cmdline), SAFESYSTEM_DEFAULT);

    // Done
    str_del(cmdline);
    return 0;
}

