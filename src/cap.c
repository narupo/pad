#include "cap.h"

int
main(int argc, char *ap[]) {
	const char *pname = ap[1];
	char *const *argv = (char *const *)ap+1;

	char ppath[100];
	snprintf(ppath, sizeof ppath, "%s/%s", getenv("CAP_BINDIR"), pname);

	pid_t pid = fork();
	if (pid == -1) {
		cap_die("fork");
	}

	if (pid == 0) {
		// Child
		if (execv(ppath, argv) == -1) {
			cap_die("execv");
		}
	} else {
		// Parent
		wait(NULL);
		exit(0);
	}

	return 0;
}
