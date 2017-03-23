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
argsparse(struct args *self, int argc, uint8_t *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {},
    };

    *self = (struct args){
        .argc = 0,
        .argv = NULL,
        .ishelp = false,
        .hostport = (const uint8_t *) "",
        .host = "",
        .port = -1,
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int32_t optsindex;
        int32_t cur = getopt_long(argc, (char **) argv, "h", longopts, &optsindex);
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
        self->hostport = self->argv[1];
        struct cap_url *url = cap_urlnew();
        if (!url) {
            return false;
        }

        if (!cap_urlparse(url, self->hostport)) {
            cap_error("failed to parse host and port");
            cap_urldel(url);
            return false;
        }
        memmove(self->host, cap_urlhost(url), uint8len(cap_urlhost(url)+1));
        self->port = cap_urlport(url);
        cap_urldel(url);
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

/*********
* thread *
*********/

struct worker {
    struct cap_socket *sock;
    uint8_t *buf;
    int32_t bufsz;
};

static void
wkrdel(struct worker *self) {
    if (self) {
        free(self);
    }
}

static struct worker *
wkrnew(void) {
    struct worker *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }
    
    self->bufsz = 5012;
    self->buf = calloc(self->bufsz, sizeof(*self->buf));
    if (!self->buf) {
        free(self);
        return NULL;
    }

    return self;
}

static struct worker *
wkrmvsock(struct worker *self, struct cap_socket *sock) {
    if (!self || !sock) {
        return NULL;
    }
    self->sock = sock;
    return self;
}

static void *
wkrstart(void *ptr) {
    struct worker *self = ptr;

    for (;;) {
        if (!cap_sockrecvstr(self->sock, self->buf, self->bufsz)) {
            break;
        }
        printf("[%s]\n", self->buf);
    }

    wkrdel(self);
    return NULL;
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
appnew(int argc, uint8_t *argv[]) {
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

    struct cap_socket *serv = cap_sockopen((uint8_t *) self->args.hostport, (uint8_t *) "tcp-server");
    if (!serv) {
        cap_error("failed create socket");
        return 1;
    }

    printf("hostport[%s\n", self->args.hostport);

    for (;;) {
        cap_log("server", "accept");
        struct cap_socket *clie = cap_sockaccept(serv);
        if (!clie) {
            cap_error("failed to accept");
            continue;
        }

        struct worker *wkr = wkrnew();
        if (!wkr) {
            cap_error("failed to create worker");
            cap_sockclose(clie);
            continue;
        }
        wkrmvsock(wkr, clie);

        pthread_t th;
        if (pthread_create(&th, NULL, wkrstart, wkr) != 0) {
            cap_error("failed to create thread");
            wkrdel(wkr);
            cap_sockclose(clie);
            continue;
        }
    }

    cap_sockclose(serv);
    return 0;
}

int
main(int argc, char *argv[]) {
    cap_envsetf("CAP_PROCNAME", "cap hub");

    struct app *app = appnew(argc, (uint8_t **) argv);
    if (!app) {
        cap_die("failed to create application");
    }
    int32_t ret = apprun(app);
    appdel(app);

    return ret;
}
