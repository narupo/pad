#include "cap-alias.h"

static char *
pathtofname(char *dst, size_t dstsz, const char *path) {
	char tmp[dstsz+1];
	size_t ti = 0;

	// Remove 環境依存の文字
	for (const char *p = path; *p; ++p) {
		if (strchr("/\\:", *p)) {
			continue;
		}
		if (ti >= dstsz) {
			return NULL;
		}
		tmp[ti++] = *p;
	}
	tmp[ti] = '\0';

	// Hash value to file name string
	long hsh = cap_hashl(tmp);
	snprintf(dst, dstsz, "%ld", hsh);

	return dst;
}

static char *
makealpath(char *dst, size_t dstsz) {
	const char *hmdir = getenv("CAP_HOMEDIR");
	const char *varhm = getenv("CAP_VARHOME");
	if (!hmdir || !varhm) {
		cap_die("invalid environ variables");
	}

	char fname[100];
	if (!pathtofname(fname, sizeof fname, varhm)) {
		cap_die("internal error");
	}

	char path[100];
	snprintf(path, sizeof path, "%s/%s-alias", hmdir, fname);

	return cap_fsolve(dst, dstsz, path);
}

struct alfile {
	FILE *fp;
	char buf[1024];
};

void
alfclose(struct alfile *alf) {
	if (alf) {
		if (fclose(alf->fp) < 0) {
			cap_log("error", "failed to close file");
		}
		free(alf);
	}
}

struct alfile *
alfopen(const char *mode) {
	struct alfile *alf = calloc(1, sizeof(*alf));

	char path[100];
	if (!makealpath(path, sizeof path)) {
		return NULL;
	}

	alf->fp = fopen(path, mode);
	if (!alf->fp) {
		free(alf);
		return NULL;
	}

	return alf;
}

static int
alfeof(const struct alfile *alf) {
	return feof(alf->fp);
}

enum {
	ALF_CID = 32,
};

static const char *
alfread(struct alfile *alf, int nread) {
	int n = fread(alf->buf, 1, nread, alf->fp);
	if (n < nread) {
		if (ferror(alf->fp)) {
			return NULL;
		}
	}
	alf->buf[n] = 0;
	return alf->buf;
}

static void
alshow(void) {
	struct alfile *alf = alfopen("rb");
	if (!alf) {
		cap_die("internal error");
	}

	for (; !alfeof(alf); ) {
		const char *buf = alfread(alf, ALF_CID);
		if (!buf) {
			break;
		}
		printf("[%s]\n", buf);
	}

	alfclose(alf);
}

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		alshow();
	}

	return 0;
}
