#include "cap-make.h"

static inline int
strncmph(const char *lh, const char *rh) {
	return strncmp(lh, rh, strlen(rh));
}

static bool
make(FILE *fout, FILE *fin) {
	char *src = cap_freadcp(fin);
	if (!src) {
		return false;
	}
	
	struct cap_string *buf = cap_strnew();
	struct cap_string *tmp = cap_strnew();
	int m = 0;

	for (char *p = src; *p; ++p) {
		// fputc(*p, dst);
		switch (m) {
		case 0: // First
			if (strncmph(p, "{{") == 0) {
				p += strlen("{{")-1;
				cap_strclear(tmp);
				m = 10;
			} else if (strncmph(p, "{%") == 0) {
				p += strlen("{%")-1;
				cap_strclear(tmp);
				m = 20;
			} else if (strncmph(p, "{#") == 0) {
				p += strlen("{#")-1;
				m = 30;
			} else {
				cap_strpushb(buf, *p);
			}
			break;
		case 10: // {{ 
			if (strncmph(p, "}}") == 0) {
				p += strlen("}}")-1;
				m = 0;
				fprintf(fout, "[reference:%s]\n", cap_strgetc(tmp));
				cap_strappother(buf, tmp);
				cap_strclear(tmp);
			} else {
				cap_strpushb(tmp, *p);
			}
			break;
		case 20: // {%
			if (strncmph(p, "%}") == 0) {
				p += strlen("%}")-1;
				m = 0;
				fprintf(fout, "[formula:%s]\n", cap_strgetc(tmp));
				cap_strappother(buf, tmp);
				cap_strclear(tmp);
			} else {
				cap_strpushb(tmp, *p);
			}
			break;
		case 30: // {#
			if (strncmph(p, "#}") == 0) {
				p += strlen("#}")-1;
				m = 0;
			}
			break;
		}
	}

	fwrite(cap_strgetc(buf), 1, cap_strlen(buf), fout);	
	cap_strdel(buf);
	cap_strdel(tmp);
	free(src);
	return true;
}

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		make(stdout, stdin);
		return 0;
	}

	const char *cd = getenv("CAP_VARCD");

	for (int i = 1; i < argc; ++i) {
		const char *fname = argv[i];

		char path[100];
		snprintf(path, sizeof path, "%s/%s", cd, fname);
		// printf("path[%s]\n", path);
		
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			cap_log("error", "failed to open file %s", path);
			continue;
		}

		make(stdout, fin);
		
		if (fclose(fin) < 0) {
			cap_log("error", "failed to close file %s", path);
		}
	}

	fflush(stderr);
	fflush(stdout);
	return 0;
}

