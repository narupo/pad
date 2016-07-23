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

enum {
	ALF_BUFCAPA = 1024,
	ALF_CNAME = 128,
	ALF_CCMD = 256,
	ALF_RLEN = ALF_CNAME + ALF_CCMD,
};

struct alfile {
	FILE *fp;
	char buf[ALF_BUFCAPA];
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

static const char *
alfreadstr(struct alfile *alf, int nread) {
	if (nread >= ALF_BUFCAPA) {
		return NULL;
	}
	int n = fread(alf->buf, 1, nread, alf->fp);
	if (n < nread) {
		if (ferror(alf->fp)) {
			return NULL;
		}
	}
	alf->buf[n] = 0;
	return alf->buf;
}

static size_t
alfwrite(struct alfile *alf, const void *ptr, size_t size, size_t nmemb) {
	return fwrite(ptr, size, nmemb, alf->fp);
}

static int
alfseek(struct alfile *alf, long ofs, int whence) {
	return fseek(alf->fp, ofs, whence);
}

static void
alshowls(void) {
	struct alfile *alf = alfopen("r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}
		printf("%s ", buf);

		buf = alfreadstr(alf, ALF_CCMD);
		if (!buf) {
			break;
		}

		printf("%s\n", buf);
	}

	alfclose(alf);
}

char *
alcmd(const char *name) {
	struct alfile *alf = alfopen("r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		if (strcmp(buf, name) != 0) {
			alfreadstr(alf, ALF_CCMD);
			continue;
		}

		buf = alfreadstr(alf, ALF_CCMD);
		if (!buf) {
			break;
		}

		alfclose(alf);
		return strdup(buf);
	}

	alfclose(alf);
	return NULL;
}

static void
alshowcmd(const char *name) {
	char *cmd = alcmd(name);
	if (!cmd) {
		cap_die("not found alias name of '%s'", name);
	}

	printf("%s\n", cmd);
	free(cmd);
}

static void
aladd(const char *name, const char *cmd) {
	struct alfile *alf = alfopen("r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	char cname[ALF_CNAME];
	char ccmd[ALF_CCMD];
	snprintf(cname, sizeof cname, "%s", name);
	snprintf(ccmd, sizeof ccmd, "%s", cmd);

	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		if (!strcmp(buf, cname)) {
			// Update command column
			alfwrite(alf, ccmd, 1, sizeof ccmd);
			goto done;
		}
	}

	// Insert alias record
	alfwrite(alf, cname, 1, sizeof cname);
	alfwrite(alf, ccmd, 1, sizeof ccmd);

done:
	alfclose(alf);
}

struct alopts {
	bool isdel;
	bool isimport;
	bool isexport;
};

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		alshowls();
	} else if (argc == 2) {
		alshowcmd(argv[1]);
	} else if (argc == 3) {
		aladd(argv[1], argv[2]);
	}
	return 0;
}
