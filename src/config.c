/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include "config.h"

struct cap_config {
	struct cap_map *map;
};

static bool
iskey(int c) {
	return isalpha(c) || c == '_';
}

static void
parse(struct cap_config *self, FILE *fin) {
	int m = 0;
	struct cap_string *key = cap_strnew();
	struct cap_string *val = cap_strnew();

	for (int c; (c = fgetc(fin)) != EOF; ) {
		switch (m) {
		case 0: // First
			if (c == '#') {
				m = 100;
			} else if (iskey(c)) {
				m = 10;
				cap_strpushb(key, c);
			}
			break;
		case 10: // Read name
			if (isblank(c) || c == '=') {
				m = 20;
			} else {
				cap_strpushb(key, c);
			}
			break;
		case 20: // Ignore blanks and equals
			if (isblank(c) || c == '=') {
				;
			} else if (c == '"') {
				// printf("key[%s]\n", cap_strgetc(key));
				m = 30;
			} else {
				// printf("key[%s]\n", cap_strgetc(key));
				cap_strpushb(val, c);
				m = 40;
			}
			break;
		case 30: // Read string of value
			if (c == '"') {
				// printf("val[%s]\n", cap_strgetc(val));
				cap_mapset(self->map, cap_strgetc(key), cap_strgetc(val));
				cap_strclear(key);
				cap_strclear(val);
				m = 100;
			} else {
				cap_strpushb(val, c);
			}
			break;
		case 40: // Read number of value
			break;
		case 100: // Waiting newline
			if (c == '\n') {
				m = 0;
			}
			break;
		}
	}

	cap_strdel(key);
	cap_strdel(val);
}

/***********************
* cap config structure *
***********************/

void
cap_confdel(struct cap_config *self) {
	if (self) {
		cap_mapdel(self->map);
		free(self);
	}
}

struct cap_config *
cap_confnew(void) {
	struct cap_config *self = calloc(1, sizeof(struct cap_config));
	if (!self) {
		return NULL;
	}
	return self;
}

struct cap_config *
cap_confnewfile(const char *fname) {
	struct cap_config *self = cap_confnew();
	if (!cap_confloadfile(self, fname)) {
		cap_confdel(self);
		return NULL;
	}
	return self;
}

struct cap_config *
cap_confnewload(void) {
	const char *confpath = getenv("CAP_CONFPATH");
	if (!confpath) {
		fprintf(stderr, "Not found environment variable of 'CAP_CONFPATH'\n");
		fflush(stderr);
		exit(1);
		return NULL;
	}
	return cap_confnewfile(confpath);
}

bool
cap_confloadfile(struct cap_config *self, const char *fname) {
	FILE *fin = fopen(fname, "rb");
	if (!fin) {
		return false;
	}

	struct cap_map *newmap = cap_mapnew();
	if (!newmap) {
		return false;
	}

	cap_mapdel(self->map);
	self->map = newmap;

	parse(self, fin);
	fclose(fin);
	return true;
}

const char *
cap_confgetc(const struct cap_config *self, const char *key) {
	return cap_mapgetc(self->map, key);
}

char *
cap_confgetcp(const struct cap_config *self, const char *key) {
	return cap_mapgetcp(self->map, key);
}

#if 0
int main(int argc, char *argv[]) {
	struct cap_config *conf = cap_confnew();
	cap_confloadfile(conf, argv[1]);
	cap_confdump(conf, stderr);
	const char *fnd = cap_confgetc(conf, "editor");
	fprintf(stderr, "Found[%s]\n", fnd);
	cap_confdel(conf);
	return 0;
}
#endif
