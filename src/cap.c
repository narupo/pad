#include "types.h"
#include "util.h"
#include "program.h"
#include "help.h"
#include "cat.h"

Command
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
            goto fail;
        }
        if (strcmp(rec->name, name) == 0) {
            return rec->command;
        }
    }
fail:
    return NULL;
}

int
main(int argc, char* argv[]) {
    program_init(argc, argv);

    if (argc < 2) {
        usage();
    }

    --argc;
    ++argv;

    char const* cmdname = argv[0];
    Command command = find_command(cmdname);
    if (!command)
        usage();
    return command(argc, argv);
}

