#include "program.h"

void
program_init(int argc, char* argv[]) {
    program.name = argv[0];
    program.argc = argc;
    program.argv = argv;
}
