#include "cap-alias.h"

enum {
	CAPAL_NCMDLINE = 1024
};

enum {
	ALF_BUFCAPA = 1024,
	ALF_CNAME = 128,
	ALF_CCMD = 256,
	ALF_RLEN = ALF_CNAME + ALF_CCMD,
};

static char *
cap_strcpywithout(char *dst, size_t dstsz, const char *src, const char *without) {
	size_t di = 0;
	for (const char *p = src; *p; ++p) {
		if (strchr(without, *p)) {
			continue;
		}
		if (di >= dstsz-1) {
			dst[di] = '\0';
			return NULL;
		}
		dst[di++] = *p;
	}
	dst[di] = '\0';
	return dst;
}

static char *
pathtofname(char *dst, size_t dstsz, const char *path) {
	char tmp[dstsz];

	// Remove 環境依存の文字
	if (!cap_strcpywithout(tmp, sizeof tmp, path, "/\\:")) {
		return NULL;
	}

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
		cap_log("error", "invalid environ variables");
		return NULL;
	}

	char fname[100];
	if (!pathtofname(fname, sizeof fname, varhm)) {
		cap_die("error", "internal error");
		return NULL;
	}

	char path[100];
	snprintf(path, sizeof path, "%s/%s-alias", hmdir, fname);

	return cap_fsolve(dst, dstsz, path);
}

struct alfile {
	FILE *fp;
	char buf[ALF_BUFCAPA];
};

int
alfclose(struct alfile *alf) {
	int ret = 0;

	if (alf) {
		ret = fclose(alf->fp);
		free(alf);
	}

	return ret;
}

struct alfile *
alfopen(const char *mode) {
	struct alfile *alf = calloc(1, sizeof(*alf));
	if (!alf) {
		return NULL;
	}

	char path[100];
	if (!makealpath(path, sizeof path)) {
		return NULL;
	}

	if (!cap_fexists(path)) {
		cap_ftrunc(path);
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
	if (nread >= ALF_BUFCAPA || nread < 1) {
		cap_log("error", "buffer overflow");
		return NULL;
	}

	int n = fread(alf->buf, 1, nread, alf->fp);
	alf->buf[n] = '\0';

	if (n < nread) {
		return NULL;
	}

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

static int
alfclear(struct alfile *alf, long len) {
	char buf[len];
	memset(buf, 0, sizeof buf);
	return fwrite(buf, 1, len, alf->fp);
}

static int
alshowls(void) {
	struct alfile *alf = alfopen("rb");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		if (!strlen(buf)) {
			alfseek(alf, ALF_CCMD, SEEK_CUR);
			continue;
		}

		printf("%s ", buf);

		buf = alfreadstr(alf, ALF_CCMD);
		if (!buf) {
			break;
		}
		printf("%s\n", buf);
	}

	if (alfclose(alf) < 0) {
		cap_log("error", "failed to close alias file");
		return 1;
	}

	return 0;
}

/**
 *
 * @return string pointer to dynamic allocate memory
 */
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

		char *cmd = strdup(buf);
		alfclose(alf);

		return cmd;
	}

	alfclose(alf);
	return NULL;
}

static int
alshowcmd(const char *name) {
	char *cmd = alcmd(name);
	if (!cmd) {
		cap_die("not found alias name of '%s'", name);
	}

	printf("%s\n", cmd);
	free(cmd);
	return 0;
}

static int
aladdal(int argc, char *argv[]) {
	struct alfile *alf = alfopen("r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	char cname[ALF_CNAME];
	snprintf(cname, sizeof cname, "%s", argv[1]);
	
	char ccmd[ALF_CCMD] = {0};
	for (int i = 2; i < argc; ++i) {
		capstrncat(ccmd, sizeof ccmd, argv[i]);
		capstrncat(ccmd, sizeof ccmd, " ");
	}

	int ern = -1; // Empty record number
	for (int i = 0; !alfeof(alf); ++i) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		if (!strlen(buf)) {
			ern = i;
			alfseek(alf, ALF_CCMD, SEEK_CUR);
			continue;
		}

		if (!strcmp(buf, cname)) {
			// Update command column
			alfwrite(alf, ccmd, 1, sizeof ccmd);
			goto done;
		}

		alfseek(alf, ALF_CCMD, SEEK_CUR);
	}

	if (ern < 0) {
		// Insert alias record
		alfseek(alf, 0L, SEEK_END);
	} else {
		alfseek(alf, ALF_RLEN*ern, SEEK_SET);
	}

	alfwrite(alf, cname, 1, sizeof cname);
	alfwrite(alf, ccmd, 1, sizeof ccmd);
	// printf("ern[%d] cname[%s] ccmd[%s]\n", ern, cname, ccmd);

done:
	if (alfclose(alf) < 0) {
		cap_log("error", "failed to close alias file");
		return 1;
	}

	return 0;
}

static int
aldelal(const char *name) {
	struct alfile *alf = alfopen("r+");

	for (; !alfeof(alf); ) {
		const char *clmname = alfreadstr(alf, ALF_CNAME);
		if (!clmname) {
			break;
		}
		// printf("clmname[%s]\n", clmname);

		if (!strcmp(name, clmname)) {
			alfseek(alf, -ALF_CNAME, SEEK_CUR);
			alfclear(alf, ALF_RLEN);
			break;
		}

		alfseek(alf, ALF_CCMD, SEEK_CUR);
	}

	alfclose(alf);
	return 0;
}

static int
alimport(const char *drtpath) {
	char path[FILE_NPATH];
	cap_fsolve(path, sizeof path, drtpath);
	if (!cap_fexists(path)) {
		cap_die("invalid import alias path '%s'", drtpath);
		return 1;
	}

	FILE *fsrc = fopen(path, "rb");
	if (!fsrc) {
		cap_die("failed to open file '%s'", path);
		return 1;
	}

	struct alfile *fdst = alfopen("wb");
	if (!fdst) {
		fclose(fsrc);
		cap_die("failed to open alias file");
		return 1;
	}

	cap_fcopy(fdst->fp, fsrc);

	if (fclose(fsrc) < 0) {
		cap_log("error", "failed to close file '%s'", path);
		alfclose(fdst);
		return 1;
	}

	if (alfclose(fdst) < 0) {
		cap_log("error", "failed to close alias file");
		return 1;
	}

	// cap_log("debug", "copy alias file from '%s'", path);
	return 0;
}

static int
alexport(const char *drtpath) {
	char path[FILE_NPATH];
	if (!cap_fsolve(path, sizeof path, drtpath)) {
		cap_die("invalid export alias path '%s'", drtpath);
	}

	struct alfile *fsrc = alfopen("rb");
	if (!fsrc) {
		cap_die("failed to open alias file");
	}

	FILE *fdst = fopen(path, "wb");
	if (!fdst) {
		alfclose(fsrc);
		cap_die("failed to open file '%s'", path);
	}

	cap_fcopy(fdst, fsrc->fp);

	if (alfclose(fsrc) < 0) {
		cap_log("error", "failed to close alias file");
		fclose(fdst);
		return 1;
	}

	if (fclose(fdst) < 0) {
		cap_log("error", "failed to close file '%s'", path);
		return 1;
	}

	// cap_log("debug", "export alias file to '%s'", path);
	return 0;
}

static int
alrun(const char *runarg) {
	const char *bindir = getenv("CAP_BINDIR");
	if (!bindir) {
		cap_log("error", "need bin directory on environ");
		return 1;
	}

	char name[ALF_CNAME];
	const char *parg = runarg;
	{
		char *beg = name;
		const char *end = name + sizeof(name) - 1;
		for (; *parg && !isspace(*parg) && beg < end; *beg++ = *parg++) {
		}
		*beg = '\0';
	}

	const char *cmdclm = alcmd(name);
	if (!cmdclm) {
		cap_die("not found alias command of '%s'", name);
		return 1;
	}

	char cmdbase[FILE_NPATH];
	snprintf(cmdbase, sizeof cmdbase, "%s/cap-%s", bindir, cmdclm);

	char cmdln[CAPAL_NCMDLINE];
	snprintf(cmdln, sizeof cmdln, "%s%s", cmdbase, parg);
	system(cmdln);
	// printf("runarg[%s] name[%s] parg[%s] cmdclm[%s]\n", runarg, name, parg, cmdclm);
	// printf("cmdln[%s]\n", cmdln);
	
	return 0;
}

struct opts {
	int nargs; // argc - optind
	bool ishelp;
	bool isdelete;
	bool isimport;
	bool isexport;
	bool isrun;
	char delname[ALF_CNAME]; // delete alias name
	char runname[ALF_CNAME]; // run alias name
	char imppath[FILE_NPATH]; // import file path
	char exppath[FILE_NPATH]; // export file path
};

static struct opts *
parseopts(struct opts *opts, int argc, char *argv[]) {
	static struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{"export", required_argument, 0, 'e'},
		{"import", required_argument, 0, 'i'},
		{"delete", required_argument, 0, 'd'},
		{"run", required_argument, 0, 'r'},
		{},
	};

	const char *hmpath = getenv("CAP_VARHOME");
	if (!hmpath) {
		cap_log("error", "invalid environ variables");
		return NULL;
	}

	*opts = (struct opts){};
	optind = 0;
	
	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "he:i:d:r:", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': opts->ishelp = true; break;
		case 'i':
			opts->isimport = true;
			cap_fsolve(opts->imppath, sizeof opts->imppath, optarg);
			if (!cap_fexists(opts->imppath)) {
				cap_die("Invalid import path %s", opts->imppath);
			}
		break;
		case 'e':
			opts->isexport = true;
			if (!optarg) {
				snprintf(opts->exppath, sizeof opts->exppath, "%s/.capalias", hmpath);
			} else {
				cap_fsolve(opts->exppath, sizeof opts->exppath, optarg);
			}
		break;
		case 'd':
			opts->isdelete = true;
			snprintf(opts->delname, sizeof opts->delname, "%s", optarg);
		break;
		case 'r':
			opts->isrun = true;
			snprintf(opts->runname, sizeof opts->runname, "%s", optarg);
		break;
		case '?':
		default: return NULL; break;
		}
	}

	if (argc < optind) {
		return NULL;
	}

	opts->nargs = argc - optind;
	return opts;
}

static int
alusage(void) {
	fprintf(stderr,
		"Usage: cap alias [arguments] [options]\n"
		"\n"
		"Example:\n"
		"\n"
		"    cap alias myalias \"cat my/path/to/docs/note.md\"\n"
		"    cap myalias\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help   show usage.\n"
		"    -d, --delete delete alias.\n"
		"    -i, --import import alias file.\n"
		"    -e, --export export alias file to home.\n"
		"    -r, --run    run command of alias.\n"
		"\n"
	);
	return 0;
}

int
main(int argc, char* argv[]) {
	setenv("CAP_PROCNAME", "cap alias", 1);

	struct opts opts;
	if (!parseopts(&opts, argc, argv)) {
		cap_die("failed to parse options");
	}

	if (opts.ishelp) {
		return alusage();

	} else if (opts.isdelete) {
		return aldelal(opts.delname);

	} else if (opts.isimport) {
		return alimport(opts.imppath);

	} else if (opts.isexport) {
		return alexport(opts.exppath);

	} else if (opts.isrun) {
		return alrun(opts.runname);

	} else if (opts.nargs == 0) {
		return alshowls();

	} else if (opts.nargs == 1) {
		return alshowcmd(argv[1]);

	} else if (opts.nargs >= 2) {
		return aladdal(argc, argv);
	}

	return 1;
}
