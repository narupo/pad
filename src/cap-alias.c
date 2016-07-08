#include "cap-alias.h"

int
main(int argc, char *argv[]) {
	const char *cd = getenv("CAP_VARCD");
	if (!cd) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	printf("%s\n", cd);

	return 0;
}

