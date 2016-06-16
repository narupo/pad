#include "cap-cd.h"

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		const char *cd = getenv("CAP_CD");
		if (!cd) {
			cap_log("error", "need environ variable of cd");
			return 1;
		}
		printf("%s\n", cd);
		return 0;
	}
	
	char newcd[100];
	cap_fsolve(newcd, sizeof newcd, argv[1]);
	
	if (!cap_fisdir(newcd)) {
		cap_log("error", "can't move to %s", newcd);
		return 1;
	}

	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir) {
		cap_log("error", "need environ variable of vardir");
		return 1;
	}
	
	char cdpath[100];
	snprintf(cdpath, sizeof cdpath, "%s/cd", vardir);
	// printf("vardir[%s]\n", cdpath);

	FILE *fout = fopen(cdpath, "w");
	if (!fout) {
		cap_log("error", "fopen %s", cdpath);
		return 1;
	}

	fprintf(fout, "%s", newcd);

	fclose(fout);
	return 0;
}
