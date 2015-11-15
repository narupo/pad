#include "types.h"
#include "util.h"
#include "program.h"
#include "help.h"
#include "cat.h"

static void _Noreturn
usage(Program* program) {
    fprintf(stderr, "Usage: %s\n"
        "\n"
        "  -h, --help Display usage.\n"
        "\n"
        , program_name(program)
    );
    fflush(stderr);
    program_delete(program);
    exit(EXIT_FAILURE);
}


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
    Program* program = program_new(argc, argv);
    if (!program)
        goto fail_0;

    if (argc < 2)
        usage(program);

    --argc;
    ++argv;

    char const* cmdname = argv[0];
    Command command = find_command(cmdname);
    if (!command)
        usage(program);
    int ret = command(argc, argv);

    program_delete(program);
    return ret;

fail_0:
    fprintf(stderr, "Failed to create program.\n");
    return 1;
}

