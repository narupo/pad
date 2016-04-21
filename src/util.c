#include "util.h"

void
free_argv(int argc, char** argv) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

#if defined(_TEST_UTIL)
int
main(int argc, char* argv[]) {
    return 0;
}
#endif
