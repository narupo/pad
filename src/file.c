#include "file.h"

bool
cap_fcopy(FILE *dst, FILE *src) {
	long tell = ftell(src);

	for (int c; (c = fgetc(src)) != EOF; fputc(c, dst)) {
	}

	fflush(dst);
	return fseek(src, tell, SEEK_SET) == 0;
}

char*
cap_frealpath(char* dst, size_t dstsize, const char* src) {
#if defined(_CAP_WINDOWS)
	char* fpart;

	if (!GetFullPathName(src, dstsize, dst, &fpart)) {
		fprintf(stderr, "Failed to solve path");
		return NULL;
	}

#else
	errno = 0;

	if (!realpath(src, dst)) {
		if (errno == ENOENT) {
			// Path is not exists
			snprintf(dst, dstsize, "%s", src);
		} else {
			fprintf(stderr, "Failed to realpath \"%s\"", src);
			return NULL;
		}
	}

#endif
	return dst;
}

char*
cap_fsolve(char* dst, size_t dstsize, const char* path) {
	char tmp[FILE_NPATH];

	// Check arugments
	if (!dst || !path) {
		fprintf(stderr, "Invalid arguments");
		return NULL;
	}

	// Solve '~'
	if (path[0] == '~') {
		snprintf(tmp, sizeof tmp, "%s/%s", getenv("HOME"), path+1);
	} else {
		snprintf(tmp, sizeof tmp, "%s", path);
	}

	if (!cap_frealpath(dst, dstsize, tmp)) {
		fprintf(stderr, "Failed to realpath \"%s\"", tmp);
		return NULL;
	}

	return dst;
}

char*
cap_fsolvecp(const char* path) {
	// Check arguments
	if (!path) {
		fprintf(stderr, "Invalid arguments");
		return NULL;
	}

	// Ready
	char* dst = (char*) malloc(sizeof(char) * FILE_NPATH);
	if (!dst) {
		fprintf(stderr, "Failed to malloc");
		return NULL;
	}

	// Solve
	char* res = cap_fsolve(dst, FILE_NPATH, path);
	if (!res) {
		fprintf(stderr, "Failed to solve path \"%s\"", path);
		free(dst);
		return NULL;
	}

	return res;
}

FILE*
cap_fopen(const char* path, const char* mode) {
	return fopen(path, mode);
}

int
cap_fclose(FILE* fp) {
	return fclose(fp);
}

DIR*
cap_fopendir(const char* path) {
	return opendir(path);
}

int
cap_fclosedir(DIR* dir) {
	return closedir(dir);
}

bool
cap_fexists(const char* path) {
	struct stat s;
	int res = stat(path, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			goto notfound;
		} else {
			perror("stat");
			exit(1);
		}
	}
	return true;

notfound:
	return false;  // Does not exists
}

bool
cap_fisdir(const char* path) {
	struct stat s;
	int res = stat(path, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			;
		} else {
			fprintf(stderr, "Failed to stat");
		}
		goto notfound;
	} else {
		if (S_ISDIR(s.st_mode)) {
			return true;
		} else {
			return false;
		}
	}

notfound:
	return false;
}

int
cap_fmkdirmode(const char* dirpath, mode_t mode) {
#if defined(_CAP_WINDOWS)
	return mkdir(dirpath);
#else
	return mkdir(dirpath, mode);
#endif
}

bool
cap_ftrunc(const char* path) {
	FILE* fout = cap_fopen(path, "wb");
	if (!fout) {
		return false;
	}
	cap_fclose(fout);
	return true;
}

char*
cap_freadcp(FILE* fin) {
	// Check arguments
	if (!fin || feof(fin)) {
		fprintf(stderr, "Invalid stream");
		return NULL;
	}

	size_t size = cap_fsize(fin);
	char* dst = malloc(sizeof(char)*size+1); // +1 for final nul
	if (!dst) {
		fprintf(stderr, "Failed to allocate memory");
		return NULL;
	}

	if (fread(dst, sizeof(char), size, fin) < size) {
		fprintf(stderr, "Failed to read from stream");
		return NULL;
	}
	dst[size] = '\0';

	return dst;
}

long
cap_fsize(FILE* fp) {
	long curtel = ftell(fp);
	long size = 0;

	fseek(fp, 0L, SEEK_SET);

	for (; fgetc(fp) != EOF; ) {
		++size;
	}

	fseek(fp, curtel, SEEK_SET);
	return size;
}

const char*
cap_fsuffix(const char* path) {
	if (!path) {
		return "";
	}

	const char* suf = strrchr(path, '.');
	if (!suf) {
		return path;
	}
	return suf + 1;
}

int
cap_frename(const char* oldpath, const char* newpath) {
	return rename(oldpath, newpath);
}

char*
cap_fdirname(char* dst, size_t dstsz, const char* path) {
	snprintf(dst, dstsz, "%s", path);
	return dirname(dst);
}

char*
cap_fbasename(char* dst, size_t dstsz, const char* path) {
	snprintf(dst, dstsz, "%s", path);
	return basename(dst);
}

/*********************
* file cap_dirnode *
*********************/

struct cap_dirnode {
#if defined(_CAP_WINDOWS)
	WIN32_FIND_DATA finddata;
#else
	struct dirent* node;
#endif
};

/*******************************
* cap_dirnode delete and new *
*******************************/

void
cap_dirnodedel(struct cap_dirnode* self) {
	if (self) {
		free(self);
	}
}

struct cap_dirnode*
cap_dirnodenew(void) {
	struct cap_dirnode* self = (struct cap_dirnode*) calloc(1, sizeof(struct cap_dirnode));
	if (!self) {
		fprintf(stderr, "Failed to construct cap_dirnode");
		return NULL;
	}
	return self;
}

/***********************
* cap_dirnode getter *
***********************/

const char*
cap_dirnodename(const struct cap_dirnode* self) {
#if defined(_CAP_WINDOWS)
	return self->finddata.cFileName;
#else
	return self->node->d_name;
#endif
}

/*****************
* file struct cap_dir *
*****************/

struct cap_dir {
#if defined(_CAP_WINDOWS)
	HANDLE handle;
	char dirpath[FILE_NPATH];
#else
	DIR* directory;
#endif
};

/***************************
* struct cap_dir close and open *
***************************/

int
cap_dirclose(struct cap_dir* self) {
	if (self) {
		int ret = 0;
#if defined(_CAP_WINDOWS)
		if (self->handle) {
			ret = FindClose(self->handle);
			if (ret == 0) {
				fprintf(stderr, "Failed to close directory");
			}
			self->handle = NULL;
			ret = !ret;
		} else {
			ret = -1;
		}

#else
		ret = closedir(self->directory);
		if (ret != 0) {
			fprintf(stderr, "Failed to close directory");
		}
#endif
		free(self);

		return ret;
	}

	return -1;
}

struct cap_dir*
cap_diropen(const char* path) {
	struct cap_dir* self = (struct cap_dir*) calloc(1, sizeof(struct cap_dir));
	if (!self) {
		fprintf(stderr, "Failed to construct struct cap_dir");
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!cap_fexists(path)) {
		fprintf(stderr, "Not found path \"%s\"", path);
		return NULL;
	}
	self->handle = NULL;
	snprintf(self->dirpath, sizeof(self->dirpath), "%s/*", path);

#else
	if (!(self->directory = opendir(path))) {
		fprintf(stderr, "Failed to open directory \"%s\"", path);
		free(self);
		return NULL;
	}

#endif
	return self;
}

/*******************
* struct cap_dir getter *
*******************/

struct cap_dirnode*
cap_dirread(struct cap_dir* self) {
	struct cap_dirnode* node = cap_dirnodenew();
	if (!node) {
		fprintf(stderr, "Failed to construct cap_dirnode");
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!self->handle) {
		if ((self->handle = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Failed to open directory \"%s\"", self->dirpath);
			cap_dirnodedel(node);
			return NULL;

		}

	} else {
		if (!FindNextFile(self->handle, &node->finddata)) {
			cap_dirnodedel(node);
			return NULL; // Done to find
		}
	}

#else
	errno = 0;
	if (!(node->node = readdir(self->directory))) {
		if (errno != 0) {
			cap_dirnodedel(node);
			fprintf(stderr, "Failed to readdir");
			return NULL;
		} else {
			// Done to readdir
			cap_dirnodedel(node);
			return NULL;
		}
	}
#endif

	return node;
}

/************
* file test *
************/

#if defined(_TEST_FILE)
static int
test_mkdir(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	const char* path = argv[1];

	if (cap_fexists(path)) {
		printf("is exists [%s]\n", path);
	} else {
		printf("is not exists [%s]\n", path);
		cap_fmkdirmode(path, S_IRUSR | S_IWUSR | S_IXUSR);
	}

	return 0;
}

static char*
solve_path(char* dst, size_t dstsize, const char* path) {
#if defined(_CAP_WINDOWS)
	char* fpart;

	if (!GetFullPathName(path, dstsize, dst, &fpart)) {
		fprintf(stderr, "Failed to solve path");
		return NULL;
	}
#endif
	return dst;
}

static int
test_solve_path(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	char dst[FILE_NPATH];
	printf("[%s] -> \n[%s]\n", argv[1], cap_fsolve(dst, sizeof dst, argv[1]));

	return 0;
}

static int
test_directory(int argc, char* argv[]) {
#if defined(_CAP_WINDOWS)
	const char* dirpath = "C:/Windows/Temp";

	if (argc >= 2) {
		dirpath = argv[1];
	}

	struct cap_dir* dir = cap_diropen(dirpath);
	if (!dir) {
		fprintf(stderr, "Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (struct cap_dirnode* node; (node = cap_dirread(dir)); ) {
		fprintf(stderr, "name[%s]\n", cap_dirnodename(node));
		cap_dirnodedel(node);
	}

	fflush(stderr);
	cap_dirclose(dir);
	return 0;

#else
	const char* dirpath = "/tmp";

	if (argc >= 2) {
		dirpath = argv[1];
	}

	struct cap_dir* dir = cap_diropen(dirpath);
	if (!dir) {
		fprintf(stderr, "Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (struct cap_dirnode* node; (node = cap_dirread(dir)); ) {
		printf("name[%s]\n", cap_dirnodename(node));
		cap_dirnodedel(node);
	}

	cap_dirclose(dir);
#endif
	return 0;
}

int
main(int argc, char* argv[]) {
	// return test_solve_path(argc, argv);
	int ret = test_directory(argc, argv);
	if (ret != 0) {
		caperr_display(stderr);
	}

	return ret;
}
#endif
