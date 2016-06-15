#include "cap-make.h"

static bool
fcopy(FILE *dst, FILE *src) {
	char *p = cap_freadcp(src);
	if (!p) {
		return false;
	}
	
	struct cap_string *buf = cap_strnew();
	struct cap_string *tmp = cap_strnew();

	for (int m = 0; *p; ++p) {
		// fputc(*p, dst);
		switch (m) {
		case 0: // First
			if (strncmp(p, "{{", strlen("{{")) == 0) {
				p += strlen("{{")-1;
				cap_strclear(tmp);
				m = 10;
			} else {
				cap_strpushb(buf, *p);
			}
			break;
		case 10: // {{ 
			if (strncmp(p, "}}", strlen("}}")) == 0) {
				p += strlen("}}")-1;
				m = 0;
				fprintf(dst, "[%s]\n", cap_strgetc(tmp));
				cap_strclear(tmp);
			} else {
				cap_strpushb(tmp, *p);
			}
			break;
		}
	}

	fwrite(cap_strgetc(buf), 1, cap_strlen(buf), dst);	
	cap_strdel(buf);
	cap_strdel(tmp);
	return true;
}

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		fcopy(stdout, stdin);
		return 0;
	}

	const char *fname = argv[1];
	const char *cd = getenv("CAP_CD");

	char path[100];
	snprintf(path, sizeof path, "%s/%s", cd, fname);
	// printf("path[%s]\n", path);
	
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		cap_die("fopen %s", path);
	}

	fcopy(stdout, fin);
	fclose(fin);

	fflush(stderr);
	fflush(stdout);
	return 0;
}
