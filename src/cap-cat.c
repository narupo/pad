#include "cap-cat.h"

char *
cap_envwith(char *dst, size_t dstsz, const char *key, const char *with, int delim) {
	const char* val = getenv(key);
	snprintf(dst, dstsz, "%s%c%s", val, delim, with);
	return dst;
}

char *
cap_pathwith(char *dst, size_t dstsz, const char *key, const char *with) {
	return cap_envwith(dst, dstsz, key, with, '/');
}

void
cap_fwriteto(FILE *fin, FILE *fout) {
	for (int c; (c = fgetc(fin)) != EOF; fputc(c, fout)) {
	}
}

int
main(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		char path[100];
		cap_pathwith(path, sizeof path, "CAP_PWD", name);
		//printf("path[%s]\n", path);
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			continue;
		}
		cap_fwriteto(fin, stderr);
		fclose(fin);
	}
	return 0;
}

