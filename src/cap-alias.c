/**
 * cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include "cap-alias.h"

/**********
* numbers *
**********/

enum {
	CAPAL_NCMDLINE = 1024
};

enum {
	ALF_BUFCAPA = 1024,
	ALF_CNAME = 128,
	ALF_CCMD = 256,
	ALF_RLEN = ALF_CNAME + ALF_CCMD,
};

/**********
* options *
**********/

struct opts {
	int nargs; // Number of arguments without program name (argc - optind)
	bool ishelp;
	bool isdelete;
	bool isimport;
	bool isexport;
	bool isrun;
	char *delname; // delete alias name
	char *runarg; // run alias arg
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

	char hmpath[FILE_NPATH];
	if (!cap_envget(hmpath, sizeof hmpath, "CAP_VARHOME")) {
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
			opts->delname = strdup(optarg);
		break;
		case 'r':
			opts->isrun = true;
			opts->runarg = strdup(optarg);
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

/*******
* utls *
*******/

static char *
pathtofname(char *dst, size_t dstsz, const char *path) {
	char tmp[dstsz];

	// Remove 環境依存の文字
	if (!capstrcpywithout(tmp, sizeof tmp, path, "/\\:")) {
		return NULL;
	}

	// Hash value to file name string
	long hsh = cap_hashl(tmp);
	snprintf(dst, dstsz, "%ld", hsh);

	return dst;
}

static char *
makealpath(char *dst, size_t dstsz) {
	char hmdir[FILE_NPATH];
	char varhm[FILE_NPATH];
	if (!cap_envget(hmdir, sizeof hmdir, "CAP_HOMEDIR") ||
		!cap_envget(varhm, sizeof varhm, "CAP_VARHOME")) {
		cap_log("error", "invalid environ variables");
		return NULL;
	}

	char fname[FILE_NPATH];
	if (!pathtofname(fname, sizeof fname, varhm)) {
		cap_die("error", "internal error");
		return NULL;
	}

	char path[FILE_NPATH];
	snprintf(path, sizeof path, "%s/%s-alias", hmdir, fname);

	return cap_fsolve(dst, dstsz, path);
}

/*********
* alfile *
*********/

struct alfile {
	FILE *fp;
	char buf[ALF_BUFCAPA];
};

static int
alfclose(struct alfile *alf) {
	int ret = 0;

	if (alf) {
		ret = fclose(alf->fp);
		free(alf);
	}

	return ret;
}

static struct alfile *
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

/*********
* record *
*********/

struct record {
	char name[ALF_CNAME];
	char cmd[ALF_CCMD];
};

static void
recdel(struct record *self) {
	if (self) {
		free(self);
	}
}

static struct record *
recnew(void) {
	struct record *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}

	return self;
}

static struct record *
recnewparam(const char *name, const char *cmd) {
	struct record *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}

	snprintf(self->name, sizeof self->name,"%s", name);
	snprintf(self->cmd, sizeof self->cmd,"%s", cmd);

	return self;
}

static struct record *
recsetname(struct record *self, const char *name) {
	snprintf(self->name, sizeof self->name,"%s", name);
	return self;
}

static struct record *
recsetcmd(struct record *self, const char *cmd) {
	snprintf(self->cmd, sizeof self->cmd,"%s", cmd);
	return self;
}

static void
recshow(const struct record *self, FILE *fout) {
	fprintf(fout, "<record name='%s' cmd='%s'>\n", self->name, self->cmd);
}

static const char *
recnamec(const struct record *self) {
	return self->name;
}

static const char *
reccmdc(const struct record *self) {
	return self->cmd;
}

/**********
* records *
**********/

struct records {
	int len;
	int capa;
	struct record **recs;
};

static void
recsdel(struct records *self) {
	if (self) {
		for (int i = 0; i < self->len; ++i) {
			recdel(self->recs[i]);
		}
		free(self->recs);
		free(self);
	}
}

static struct records *
recsnewcapa(int capa) {
	struct records *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}

	self->capa = (capa > 0 ? capa : 4);
	self->recs = calloc(self->capa+1, sizeof(struct record*)); // +1 for final nul
	if (!self->recs) {
		free(self);
		return NULL;
	}

	return self;
}

static struct records *
recsresize(struct records *self, int newcapa) {
	if (newcapa < self->capa) {
		return NULL;
	}

	struct record **tmp = realloc(self->recs, sizeof(struct record *) * newcapa +1);
	if (!tmp) {
		return NULL;
	}

	self->capa = newcapa;
	self->recs = tmp;
	self->recs[newcapa] = NULL;

	return self;
}

static struct records *
recsmove(struct records *self, struct record *rec) {
	if (self->len >= self->capa) {
		if (!recsresize(self, self->capa*2)) {
			return NULL;
		}
	}

	self->recs[self->len++] = rec;
	return self;
}

static int
recscmp(const void *p1, const void *p2) {
	const struct record *r1 = *(const struct record **)p1;
	const struct record *r2 = *(const struct record **)p2;
	return strcmp(r1->name, r2->name);
}

static void
recssortname(struct records *self) {
	qsort(self->recs, self->len, sizeof(self->recs[0]), recscmp);
}

static int
recslen(const struct records *self) {
	return self->len;
}

static const struct record *
recsgetc(const struct records *self, int idx) {
	if (idx >= self->len) {
		return NULL;
	}
	return self->recs[idx];
}

/*************
* al options *
*************/

static int
alshowls(void) {
	struct records *recs = recsnewcapa(4);
	if (!recs) {
		cap_die("failed to create records");
	}

	struct record *tmprec = recnew();
	if (!tmprec) {
		recsdel(recs);
		cap_die("failed to create record");
	}

	struct alfile *alf = alfopen("rb");
	if (!alf) {
		recdel(tmprec);
		recsdel(recs);
		cap_die("failed to open alias file");
	}

	// Read records
	int maxnamelen = 0;
	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		int namelen = strlen(buf);
		if (!namelen) {
			alfseek(alf, ALF_CCMD, SEEK_CUR);
			continue;
		}
		maxnamelen = (namelen > maxnamelen ? namelen : maxnamelen);
		// printf("%s ", buf);
		recsetname(tmprec, buf);

		buf = alfreadstr(alf, ALF_CCMD);
		if (!buf) {
			break;
		}
		// printf("%s\n", buf);
		recsetcmd(tmprec, buf);
		// recshow(tmprec, stderr);

		recsmove(recs, tmprec);
		tmprec = recnew();
	}

	// Sort records
	recssortname(recs);

	// Draw records
	for (int i = 0; i < recslen(recs); ++i) {
		const struct record *r = recsgetc(recs, i); 
		const char *name = recnamec(r);
		const char *cmd = reccmdc(r);
		int namelen = strlen(name);
		
		fprintf(stdout, "%s", name);
		for (int i = 0; i < maxnamelen-namelen; ++i) {
			fputc(' ', stdout);
		}
		fprintf(stdout, " %s\n", cmd);
	}

	// Done
	recdel(tmprec);
	recsdel(recs);

	if (alfclose(alf) < 0) {
		cap_log("error", "failed to close alias file");
		return 1;
	}

	return 0;
}

/**
 * Read command by alias name
 *
 * @param string alias name
 * @return string pointer to dynamic allocate memory
 */
static char *
alreadcmd(const char *name) {
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
	char *cmd = alreadcmd(name);
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
	char bindir[FILE_NPATH];
	if (!cap_envget(bindir, sizeof bindir, "CAP_BINDIR")) {
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

	const char *cmdcol = alreadcmd(name);
	if (!cmdcol) {
		cap_die("not found alias command of '%s'", name);
		return 1;
	}

	char cmdbase[FILE_NPATH];
	snprintf(cmdbase, sizeof cmdbase, "%s/cap-%s", bindir, cmdcol);

	struct cap_string *cmdline = cap_strnew();
	if (!cmdline) {
		cap_log("error", "failed to create cap string");
		return 1;
	}

	cap_strapp(cmdline, cmdbase);
	cap_strapp(cmdline, parg);

	system(cap_strgetc(cmdline));
	// cap_log("debug", "runarg[%s] name[%s] parg[%s] cmdcol[%s]\n", runarg, name, parg, cmdcol);
	// cap_log("debug", "cmdln[%s]\n", cap_strgetc(cmdline));
	
	cap_strdel(cmdline);
	return 0;
}

static int
alusage(void) {
	fprintf(stderr,
		"Usage: cap alias [arguments] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help   show usage.\n"
		"    -d, --delete delete alias.\n"
		"    -i, --import import alias file.\n"
		"    -e, --export export alias file.\n"
		"    -r, --run    run command of alias.\n"
		"\n"
		"Example:\n"
		"\n"
		"    cap alias myalias \"cat my/path/to/docs/note.md\"\n"
		"    cap myalias\n"
		"\n"
	);
	return 0;
}

/*******
* main *
*******/

int
main(int argc, char* argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap alias");

	struct opts opts;
	if (!parseopts(&opts, argc, argv)) {
		cap_die("failed to parse options");
	}

	int ret = 0;

	if (opts.ishelp) {
		ret = alusage();

	} else if (opts.isimport) {
		ret = alimport(opts.imppath);

	} else if (opts.isexport) {
		ret = alexport(opts.exppath);

	} else if (opts.isdelete) {
		ret = aldelal(opts.delname);
		free(opts.delname);

	} else if (opts.isrun) {
		ret = alrun(opts.runarg);
		free(opts.runarg);

	} else if (opts.nargs == 0) {
		ret = alshowls();

	} else if (opts.nargs == 1) {
		ret = alshowcmd(argv[1]);

	} else if (opts.nargs >= 2) {
		ret = aladdal(argc, argv);
	}

	return ret;
}
