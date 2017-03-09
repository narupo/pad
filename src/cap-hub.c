#include "cap-hub.h"

struct opts {
    bool ishelp;
};

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        // {"fname", required_argument, 0, 'f'},
        {},
    };

    *self = (struct opts){};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "hf:", longopts, &optsindex);
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

int
main(int argc, char* argv[]) {
    printf("Hello, World!\n");
    return 0;
}
