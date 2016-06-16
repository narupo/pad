#include "cap-cd.h"

int
main(int argc, char* argv[]) {
puts("4");	
	if (argc < 2) {
		const char *cd = getenv("CAP_CD");
		printf("%s\n", (cd ? cd : "null"));
		return 0;
	}
puts("3");	
	
	char newcd[100];
	cap_fsolve(newcd, sizeof newcd, argv[1]);
	
puts("2");	
	if (!cap_fisdir(newcd)) {
		cap_log("error", "can't move to %s", newcd);
		return 1;
	}

puts("1");	
	struct cap_config *conf = cap_confnew();
	if (!conf) {
		cap_log("error", "cap_confnew");
		return 1;
	}

	if (!cap_confsaverow(conf, "cd", newcd)) {
		cap_log("error", "cap_confsaverow");
		return 1;
	}	

	cap_confdel(conf);
	return 0;
}
