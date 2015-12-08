#include "file.h"

char*
file_solve_path(char* dst, size_t dstsize, char const* path) {
	char tmp[NFILE_PATH];

	//! Check arugments
	if (!dst || !path) {
		WARN("Invalid arguments");
		return NULL;
	}

	//! Solve '~'
	if (path[0] == '~') {
		snprintf(tmp, NFILE_PATH, "%s%s", getenv("HOME"), path+1);
	} else {
		snprintf(tmp, NFILE_PATH, "%s", path);
	}

	//! Solve real path
	errno = 0;
	if (!realpath(tmp, dst)) {
		if (errno == ENOENT) {
			// Path is not exists
			snprintf(dst, dstsize, "%s", tmp);
		} else {
			WARN("Failed to realpath \"%s\"", tmp);
			return NULL;
		}
	}

	return dst;
}

char*
file_make_solve_path(char const* path) {
	// Check arguments
	if (!path) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Ready
	char* dst = (char*) malloc(sizeof(char) * NFILE_PATH);
	if (!dst) {
		WARN("Failed to malloc");
		return NULL;
	}

	// Solve
	char* res = file_solve_path(dst, NFILE_PATH, path);
	if (!res) {
		WARN("Failed to solve path \"%s\"", path);
		free(dst);
		return NULL;
	}

	return res;
}

FILE*
file_open(char const* path, char const* mode) {
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	return fopen(spath, mode);
}

DIR*
file_opendir(char const* path) {
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	return opendir(spath);
}

int
file_closedir(DIR* dir) {
	return closedir(dir);
}

int
file_close(FILE* fp) {
	return fclose(fp);
}

bool
file_is_exists(char const* path) {
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return false;
	}

	struct stat s;
	int res = stat(spath, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			goto notfound;
		} else {
			die("stat");
		}
	}
	return true;

notfound:
	return false;  // Does not exists
}

bool
file_is_dir(char const* path) {
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	struct stat s;
	int res = stat(spath, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			;
		} else {
			WARN("Failed to stat");
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
file_mkdir(char const* dirpath, mode_t mode) {
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, dirpath)) {
		WARN("Failed to solve path \"%s\"", dirpath);
		return -1;
	}

	return mkdir(spath, mode);
}

char*
file_read_string(FILE* fin) {
	if (!fin || feof(fin)) {
		WARN("Invalid stream");
		return NULL;
	}

	fseek(fin, 0L, SEEK_END);
	int len = ftell(fin);
	fseek(fin, 0L, SEEK_SET);

	char* dst = (char*) malloc(sizeof(char) * len + 1);  // +1 for final nul
	if (!dst) {
		WARN("Failed to allocate memory");
		file_close(fin);
		return NULL;
	}

	if (fread(dst, sizeof(char), len, fin) < len) {
		WARN("Failed to fread");
		file_close(fin);
		free(dst);
		return NULL;
	}

	dst[len] = '\0';

	return dst;
}

#if defined(TEST_FILE)
void
test_mkdir(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	char const* path = argv[1];

	if (file_is_exists(path)) {
		printf("is exists [%s]\n", path);
	} else {
		printf("is not exists [%s]\n", path);
		file_mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR);
	}
}

void
test_readall(int argc, char* argv[]) {
	char* str = file_read_string("util.h");
	printf("str[%s]\n", str);
	free(str);
}

int
main(int argc, char* argv[]) {
	test_readall(argc, argv);
	return 0;
}
#endif
