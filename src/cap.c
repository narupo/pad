#include "cap.h"

typedef int (*Command)(int, char**);

static Command
find_command(char const* name) {
    struct CommandRecord {
        char const* name;
        Command command;
    };
    static struct CommandRecord table[] = {
        {"help", help_main},
        {"cat", cat_main},
        {0},
    };
    for (int i = 0; ; ++i) {
        struct CommandRecord* rec = &table[i];
        if (!rec->name) {
            goto fail_0;
        }
        if (strcmp(rec->name, name) == 0) {
            return rec->command;
        }
    }

fail_0:
    return NULL;
}

int
main(int argc, char* argv[]) {
    if (argc < 2)
        help_usage();

    --argc;
    ++argv;

    char const* cmdname = argv[0];
    Command command = find_command(cmdname);
    if (!command)
        help_usage();
    return command(argc, argv);
}

