#include "cap-make.h"

static bool
fcopy(FILE *dst, FILE *src) {
	char *p = cap_freadcp(src);
	struct cap_string *buf = cap_strnew();

	int m = 0;
	for (; *p; ++p) {
		// fputc(*p, dst);
		switch (m) {
		case 0: // First
			if (strncmp(p, "@cap", strlen("@cap")) == 0) {
				p += strlen("@cap")-1;
				m = 10;
			} else {
				cap_strpushb(buf, *p);
			}
			break;
		case 10: // Found @cap
			if (strncmp(p, "{{", strlen("{{")) == 0) {
				p += strlen("{{")-1;
				m = 20;
			} else {
				cap_strpushb(buf, *p);
			}
			break;
		case 20: // {{ 
			if (strncmp(p, "}}", strlen("}}")) == 0) {
				p += strlen("}}")-1;
				m = 0;
			} else {
				cap_strpushb(buf, *p);
			}
			break;
		}
	}

	fwrite(cap_strgetc(buf), 1, cap_strlen(buf), dst);	
	cap_strdel(buf);
	return true;
}

int
main(int argc, char* argv[]) {
	FILE *fin = stdin;

	if (argc >= 2) {
		const char *fname = argv[1];
		struct cap_config *conf = cap_confnewload();
		char *cd = cap_confgetcp(conf, "cd");
		cap_confdel(conf);

		char path[100];
		snprintf(path, sizeof path, "%s/%s", cd, fname);
		// printf("path[%s]\n", path);
		free(cd);		
		
		fin = fopen(path, "rb");
		if (!fin) {
			cap_die("fopen %s", path);
		}
	}

	fcopy(stderr, fin);
	fclose(fin);

	fflush(stderr);
	fflush(stdout);
	return 0;
}
