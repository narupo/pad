/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include "cap-ls.h"

static void
arrdump(const struct cap_array *arr, FILE *fout) {
	for (int i = 0; i < cap_arrlen(arr); ++i) {
		fprintf(fout, "%s\n", cap_arrgetc(arr, i));
	}
	fflush(fout);
}

static struct cap_array *
dir2array(struct cap_dir *dir) {
	struct cap_array *arr = cap_arrnew();
	if (!arr) {
		return NULL;
	}

	for (struct cap_dirnode *nd; (nd = cap_dirread(dir)); ) {
		const char *name = cap_dirnodename(nd);
		cap_arrpush(arr, name);
		cap_dirnodedel(nd);
	}

	return arr;
}

static int
capls(const char *path) {
	if (isoutofhome(path)) {
		cap_error("'%s' is out of home", path);
		return 1;
	}
	
	struct cap_dir *dir = cap_diropen(path);
	if (!dir) {
		cap_error("failed to open directory %s", path);
		return 1;
	}

	struct cap_array *arr = dir2array(dir);
	if (!arr) {
		cap_log("error", "failed to read directory %s", path);
		return 1;
	}

	cap_arrsort(arr);
	arrdump(arr, stdout);
	cap_arrdel(arr);

	if (cap_dirclose(dir) < 0) {
		cap_log("error", "failed to close directory %s", path);
		return 1;
	}

	return 0;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap ls");

	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_die("need environment variable of cd");
	}

	if (argc < 2) {
		capls(cd);
	} else {
		char path[FILE_NPATH];
		for (int i = 1; i < argc; ++i) {
			cap_fsolvefmt(path, sizeof path, "%s/%s", cd, argv[i]);
			capls(path);
		}
	}

	return 0;
}
