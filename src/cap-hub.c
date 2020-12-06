#include "cap-hub.h"

/*******
* opts *
*******/

struct opts {
    bool ishelp;
} opts;

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
    };

    *self = (struct opts){};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->ishelp = true; break;
        case '?':
        default: return false; break;
        }
    }

    if (argc < optind) {
        perror("Failed to parse option");
        return false;
    }

    return true;
}

/******
* app *
******/

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
        cap_error("failed to allocate memory");
        return NULL;
    }

    if (!optsparse(&self->opts, argc, argv)) {
        cap_error("failed to parse options");
        free(self);
        return NULL;
    }

    return self;
}

static void
appusage(struct app *self) {
    fflush(stdout);
    fprintf(stderr,
        "Cap's hub.\n"
        "\n"
        "Usage:\n"
        "\n"
        "   cap hub [command] [options]\n"
        "\n"
        "The commands are:\n"
        "\n"
        "   ls    list nodes.\n"
        "\n"
        "The options are:\n"
        "\n"
        "   -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
}

static int
apprun(struct app *self) {
    if (self->opts.ishelp) {
        appusage(self);
        return 0;
    }
    
    struct cap_socket *serv = cap_sockopen("localhost:12345", "tcp-server");
    if (!serv) {
        cap_error("failed create socket");
        return 1;
    }

    for (;;) {
        cap_log("server", "accept...");
        struct cap_socket *clie = cap_sockaccept(serv);
        if (!clie) {
            cap_error("failed to accept");
            continue;
        }

        cap_sockclose(clie);
    }

    cap_sockclose(serv);
    return 0;
}

int
main(int argc, char* argv[]) {
    cap_envsetf("CAP_PROCNAME", "cap hub");

    struct app *app = appnew(argc, argv);
    if (!app) {
        cap_die("failed to create application");
    }
    int ret = apprun(app);
    appdel(app);

    return ret;
}
