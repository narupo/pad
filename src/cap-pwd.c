#include "cap-pwd.h"

int
main(void) {
	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	printf("%s\n", cd);

	return 0;
}
