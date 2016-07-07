#include "cap-ls.h"

struct array {
	char **arr;
	size_t len;
	size_t capa;
};

static void
arrdel(struct array *arr) {
	if (arr) {
		free(arr->arr);
		free(arr);
	}
}

static struct array *
arrnew(void) {
	struct array *arr = calloc(1, sizeof(struct array));
	if (!arr) {
		return NULL;
	}

	arr->capa = 4;
	arr->arr = calloc(arr->capa+1, sizeof(struct array *));
	if (!arr->arr) {
		free(arr);
		return NULL;
	}

	return arr;
}

static struct array *
arrpush(struct array *arr, const char *str) {
	if (arr->len >= arr->capa) {
		size_t capa = arr->capa*2;
		size_t size = sizeof(arr->arr[0]);
		char **tmp = realloc(arr->arr, size*capa + size);
		if (!tmp) {
			return NULL;
		}

		arr->arr = tmp;
		arr->capa = capa;
	}

	arr->arr[arr->len++] = strdup(str);
	arr->arr[arr->len] = NULL;

	return arr;
}

static int
arrcmp(const void *lh, const void *rh) {
	const char *ls = *(const char **)lh;
	const char *rs = *(const char **)rh;
	return strcmp(ls, rs);
}

static struct array *
arrsort(struct array *arr) {
	qsort(arr->arr, arr->len, sizeof(arr->arr[0]), arrcmp);
	return arr;
}

static void
arrdump(const struct array *arr, FILE *fout) {
	for (size_t i = 0; i < arr->len; ++i) {
		fprintf(fout, "%s\n", arr->arr[i]);
	}
	fflush(fout);
}

static struct array *
dir2array(struct cap_dir *dir) {
	struct array *arr = arrnew();
	if (!arr) {
		return NULL;
	}

	for (struct cap_dirnode *nd; (nd = cap_dirread(dir)); ) {
		const char *name = cap_dirnodename(nd);
		arrpush(arr, name);
		cap_dirnodedel(nd);
	}

	return arr;
}

static int
cap_ls(const char *path) {
	struct cap_dir *dir = cap_diropen(path);
	if (!dir) {
		cap_log("error", "failed to open directory %s", path);
		return 1;
	}

	struct array *arr = dir2array(dir);
	if (!arr) {
		cap_log("error", "failed to read directory %s", path);
		return 1;
	}

	arrsort(arr);
	arrdump(arr, stdout);
	arrdel(arr);

	if (cap_dirclose(dir) < 0) {
		cap_log("error", "failed to close directory %s", path);
		return 1;
	}

	return 0;
}

int
main(int argc, char *argv[]) {
	const char *cd = getenv("CAP_CD");
	if (!cd) {
		cap_die("need environment variable of cd");
	}

	return cap_ls(cd);
}

