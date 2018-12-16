/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
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
	struct cap_array *args;
	bool ishelp;
	bool isdelete;
	bool isimport;
	bool isexport;
	bool isrun;
	const char *opensrc; // "local" or "global"
	char *delname; // delete alias name
	char *runarg; // run alias arg
	char imppath[FILE_NPATH]; // import file path
	char exppath[FILE_NPATH]; // export file path
};

static struct opts *
optsparse(struct opts *opts, int argc, char *argv[]) {
	/* TODO: Success to ...
		$ cap al alg "alias -g"
		$ cap alg
	*/
	static struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{"export", required_argument, 0, 'e'},
		{"import", required_argument, 0, 'i'},
		{"delete", required_argument, 0, 'd'},
		{"run", required_argument, 0, 'r'},
		{"global", no_argument, 0, 'g'},
		{},
	};

	*opts = (struct opts) {
		.opensrc = "local",
	};
	opterr = 0;
	optind = 0;
	
	char cdpath[FILE_NPATH];
	if (!cap_envget(cdpath, sizeof cdpath, "CAP_VARCD")) {
		cap_error("invalid environ variables of CAP_VARCD");
		return NULL;
	}

	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "he:i:d:r:g", longopts, &optsindex);
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
			cap_fsolve(opts->exppath, sizeof opts->exppath, optarg);
		break;
		case 'd':
			opts->isdelete = true;
			opts->delname = strdup(optarg);
		break;
		case 'r':
			opts->isrun = true;
			opts->runarg = strdup(optarg);
		break;
		case 'g':
			opts->opensrc = "global";
		break;
		case '?':
		default: return NULL; break;
		}
	}

	if (argc < optind) {
		return NULL;
	}

	opts->args = argsbyoptind(argc, argv, optind);
	if (!opts->args) {
		return NULL;
	}

	return opts;
}

/*******
* utls *
*******/

static char *
pathtohomename(char *dst, size_t dstsz, const char *path) {
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

/**
 * @param[in] src source "local" or "global"
 */
static char *
makealpath(char *dst, size_t dstsz, const char *src) {
	if (strcmp(src, "local") == 0) {
		char envdir[FILE_NPATH];
		if (!cap_envget(envdir, sizeof envdir, "CAP_ENVDIR")) { 
			cap_error("invalid environ variables CAP_ENVDIR");
			return NULL;
		}

		char varcd[FILE_NPATH];
		if (!cap_envget(varcd, sizeof varcd, "CAP_VARCD")) {
			cap_error("invalid environ variables CAP_VARCD");
			return NULL;
		}

		char homename[FILE_NPATH];
		if (!pathtohomename(homename, sizeof homename, varcd)) {
			cap_error("internal error");
			return NULL;
		}

		char homepath[FILE_NPATH];
		if (!cap_fsolvefmt(homepath, sizeof homepath, "%s/%s", envdir, homename)) {
			cap_error("failed to solve file path");
			return NULL;
		}

		if (!cap_fexists(homepath)) {
			if (cap_fmkdirq(homepath) != 0) {
				cap_error("failed to make directory \"%s\"", homepath);
				return NULL;
			}
		}

		char fpath[FILE_NPATH];
		snprintf(fpath, sizeof fpath, "%s/alias", homepath);
		return cap_fsolve(dst, dstsz, fpath);

	} else if (strcmp(src, "global") == 0) {
		return cap_fsolve(dst, dstsz, "~/.cap/alias");

	} else {
		cap_error("invalid source \"%s\"", src);
		return NULL;
	}
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

/**
 * @param[in] src source "local" or "global"
 */
static struct alfile *
alfopen(const char *src, const char *mode) {
	// Create file
	char path[FILE_NPATH];
	if (!makealpath(path, sizeof path, src)) {
		return NULL;
	}

	if (!cap_fexists(path)) {
		cap_ftrunc(path);
	}

	// Open file
	struct alfile *alf = calloc(1, sizeof(*alf));
	if (!alf) {
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

/******
* app *
******/

struct app {
	struct opts opts;
};

static int
appshowls(const struct app *self) {
	struct records *recs = recsnewcapa(4);
	if (!recs) {
		cap_die("failed to create records");
	}

	struct record *tmprec = recnew();
	if (!tmprec) {
		recsdel(recs);
		cap_die("failed to create record");
	}

	struct alfile *alf = alfopen(self->opts.opensrc, "rb");
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
		recsetname(tmprec, buf);

		buf = alfreadstr(alf, ALF_CCMD);
		if (!buf) {
			break;
		}
		recsetcmd(tmprec, buf);

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
		fprintf(stdout, "    %s\n", cmd);
	}

	// Done
	recdel(tmprec);
	recsdel(recs);

	if (alfclose(alf) < 0) {
		cap_error("failed to close alias file");
		return 1;
	}

	return 0;
}

/**
 * Read command by alias name
 *
 * @param fndsrc "local" or "global"
 * @param string alias name
 * @return string pointer to dynamic allocate memory
 */
static char *
appfindcmdcp(const char *findsrc, const char *fndname) {
	struct alfile *alf = alfopen(findsrc, "r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	for (; !alfeof(alf); ) {
		const char *buf = alfreadstr(alf, ALF_CNAME);
		if (!buf) {
			break;
		}

		if (strcmp(buf, fndname) != 0) {
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
appshowcmd(struct app *self) {
	const char *name = cap_arrgetc(self->opts.args, 1);
	char *cmd = appfindcmdcp(self->opts.opensrc, name);
	if (!cmd) {
		cap_die("not found alias name of '%s'", name);
	}

	printf("%s\n", cmd);
	free(cmd);
	return 0;
}

static int
appaddal(struct app *self) {
	struct alfile *alf = alfopen(self->opts.opensrc, "r+");
	if (!alf) {
		cap_die("failed to open alias file");
	}

	char colname[ALF_CNAME];
	snprintf(colname, sizeof colname, "%s", cap_arrgetc(self->opts.args, 1));
	
	char ccmd[ALF_CCMD] = {0};
	for (int i = 2; i < cap_arrlen(self->opts.args); ++i) {
		capstrncat(ccmd, sizeof ccmd, cap_arrgetc(self->opts.args, i));
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

		if (!strcmp(buf, colname)) {
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

	alfwrite(alf, colname, 1, sizeof colname);
	alfwrite(alf, ccmd, 1, sizeof ccmd);

done:
	if (alfclose(alf) < 0) {
		cap_log("error", "failed to close alias file");
		return 1;
	}

	return 0;
}

static int
appdelal(struct app *self) {
	struct alfile *alf = alfopen(self->opts.opensrc, "r+");

	for (; !alfeof(alf); ) {
		const char *clmname = alfreadstr(alf, ALF_CNAME);
		if (!clmname) {
			break;
		}

		if (!strcmp(self->opts.delname, clmname)) {
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
appimport(struct app *self) {
	char path[FILE_NPATH];

	cap_fsolve(path, sizeof path, self->opts.imppath);
	if (!cap_fexists(path)) {
		cap_die("invalid import alias path '%s'", self->opts.imppath);
		return 1;
	}

	FILE *fsrc = fopen(path, "rb");
	if (!fsrc) {
		cap_die("failed to open file '%s'", path);
		return 1;
	}

	struct alfile *fdst = alfopen(self->opts.opensrc, "wb");
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

	return 0;
}

static int
appexport(struct app *self) {
	char path[FILE_NPATH];
	if (!cap_fsolve(path, sizeof path, self->opts.exppath)) {
		cap_die("invalid export alias path '%s'", self->opts.exppath);
	}

	struct alfile *fsrc = alfopen(self->opts.opensrc, "rb");
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

	return 0;
}

static int
apprun(struct app *self) {
	// Create bin path of commands
	char bindir[FILE_NPATH];
	if (!cap_envget(bindir, sizeof bindir, "CAP_BINDIR")) {
		cap_error("need bin directory on environ");
		return 1;
	}

	// Grep command name from argument of option
	char cmdname[ALF_CNAME];
	const char *parg = self->opts.runarg;
	{
		char *beg = cmdname;
		const char *end = cmdname + sizeof(cmdname) - 1;
		for (; *parg && !isspace(*parg) && beg < end; *beg++ = *parg++) {
		}
		*beg = '\0';
	}

	// First find at local
	const char *scope = "local";
	char *cmdcol = appfindcmdcp(scope, cmdname);
	if (!cmdcol) {
		// Next find at global
		scope = "global";
		cmdcol = appfindcmdcp(scope, cmdname);
		if (!cmdcol) {
			cap_error("not found command name \"%s\"", cmdname);
			return 2;
		}
	}

	// Create command path
	char cmdpath[FILE_NPATH];
	snprintf(cmdpath, sizeof cmdpath, "%s/cap-%s", bindir, cmdcol);
	free(cmdcol);

	// Create command line for child process
	struct cap_string *cmdline = cap_strnew();
	if (!cmdline) {
		cap_error("failed to create cap string");
		return 3;
	}

	cap_strapp(cmdline, cmdpath);
	cap_strapp(cmdline, parg);

	// Execute child process
	cap_envsetf("CAP_SCOPE", scope);
	safesystem(cap_strgetc(cmdline));
	
	// Done
	cap_strdel(cmdline);
	return 0;
}

static int
appusage(const struct app *self) {
	fprintf(stderr,
		"Usage: cap alias [arguments] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help      show usage.\n"
		"    -d, --delete    delete alias.\n"
		"    -i, --import    import alias file.\n"
		"    -e, --export    export alias file.\n"
		"    -r, --run       run command of alias.\n"
		"    -g, --global    on global environment.\n"
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

static void
appdel(struct app *self) {
	if (self) {
		cap_arrdel(self->opts.args);
		free(self->opts.delname);
		free(self->opts.runarg);
		free(self);
	}
}

static struct app *
appnew(int argc, char *argv[]) {
	struct app *self = calloc(1, sizeof(*self));
	if (!self) {
		cap_error("failed to create app");
		return NULL;
	}

	if (!optsparse(&self->opts, argc, argv)) {
		cap_die("failed to parse options");
	}

	return self;
}

static int
appmain(struct app *self) {
	if (self->opts.ishelp) {
		return appusage(self);

	} else if (self->opts.isimport) {
		return appimport(self);

	} else if (self->opts.isexport) {
		return appexport(self);

	} else if (self->opts.isdelete) {
		return appdelal(self);

	} else if (self->opts.isrun) {
		return apprun(self);

	} else if (cap_arrlen(self->opts.args) == 1) {
		return appshowls(self);

	} else if (cap_arrlen(self->opts.args) == 2) {
		return appshowcmd(self);

	} else if (cap_arrlen(self->opts.args) > 2) {
		return appaddal(self);
	}

	return 0;
}

int
main(int argc, char* argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap alias");

	struct app *app = appnew(argc, argv);
	if (!app) {
		cap_die("failed to create app");
		return 1;
	}

	int ret = appmain(app);
	appdel(app);

	return ret;
}

