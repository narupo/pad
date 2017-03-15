#include "cap-hub.h"

/*******
* args *
*******/

enum {
    ARGS_NHOST = 256,
    ARGS_NBUF = 256,
};

struct args {
    int32_t argc;
    const uint8_t *const *argv;
    bool ishelp;
    const uint8_t *hostport;
    int32_t port;
    uint8_t host[ARGS_NHOST];
} args;

static bool
argsparsehostport(struct args *self, const uint8_t *hostport) {
    uint8_t buf[ARGS_NBUF] = {0};
    uint32_t bi = 0, m = 0;

    self->host[0] = '\0';

    for (int32_t i = 0; i < uint8len(hostport)+1; ++i) {
        uint8_t c = hostport[i]; // allow final nil

        switch (m) {
        case 0:
            switch (c) {
            case ':':
                buf[bi] = '\0';
                memmove(self->host, buf, bi);
                bi = 0;
                m = 1;
                break;
            case '\0':
                buf[bi] = '\0';
                memmove(self->host, buf, bi);
                break;
            default:
                buf[bi++] = c;
                break;
            }  
            break;
        case 1:
            switch (c) {
            case '\0':
                buf[bi] = '\0';
                self->port = uint8toint32(buf);
                break;
            default:
                buf[bi++] = c;
                break;
            }
            break;
        default:
            assert(0 && "impossible");
            break;
        }
    }

    if (bi && m == 1) {
    }

    return true;
}

static bool
argsparse(struct args *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
    };

    *self = (struct args){};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int32_t optsindex;
        int32_t cur = getopt_long(argc, argv, "h", longopts, &optsindex);
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
        cap_error("failed to parse option");
        return false;
    }

    self->argc = argc-optind+1;
    self->argv = (const uint8_t *const *) argv;

    if (self->argc >= 2) {
        self->hostport = (const uint8_t *) self->argv[1];
        if (!argsparsehostport(self, self->hostport)) {
            cap_error("failed to parse host and port");
        }
    }

    return true;
}

static void
argsshow(const struct args *self) {
    printf("argc %d\n", self->argc);
    for (int32_t i = 0; i < self->argc; ++i) {
        printf("argv[%d] = \"%s\"\n", i, self->argv[i]);
    }
    printf("ishelp %d\n", self->ishelp);
    printf("hostport \"%s\"\n", self->hostport);
    printf("host \"%s\"\n", self->host);
    printf("port %d\n", self->port);
}

/******
* app *
******/

struct app {
    struct args args;
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

    if (!argsparse(&self->args, argc, argv)) {
        cap_error("failed to parse options");
        free(self);
        return NULL;
    }

    return self;
}

static void
appusage(const struct app *self) {
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

static int32_t
apprun(struct app *self) {
    if (self->args.ishelp) {
        appusage(self);
        return 0;
    }

    argsshow(&self->args);
return 0;

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
main(int argc, char *argv[]) {
    cap_envsetf("CAP_PROCNAME", "cap hub");

    struct app *app = appnew(argc, argv);
    if (!app) {
        cap_die("failed to create application");
    }
    int ret = apprun(app);
    appdel(app);

    return ret;
}
