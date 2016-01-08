#include "file.h"

char*
file_solve_path(char* dst, size_t dstsize, char const* path) {
	char tmp[FILE_NPATH];

	// Check arugments
	if (!dst || !path) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Solve '~'
	if (path[0] == '~') {
		snprintf(tmp, FILE_NPATH, "%s%s", getenv("HOME"), path+1);
	} else {
		snprintf(tmp, FILE_NPATH, "%s", path);
	}

	// Solve real path
	errno = 0;

#if defined(_WIN32) || defined(_WIN64)
# error Need realpath!
#endif
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
	char* dst = (char*) malloc(sizeof(char) * FILE_NPATH);
	if (!dst) {
		WARN("Failed to malloc");
		return NULL;
	}

	// Solve
	char* res = file_solve_path(dst, FILE_NPATH, path);
	if (!res) {
		WARN("Failed to solve path \"%s\"", path);
		free(dst);
		return NULL;
	}

	return res;
}

FILE*
file_open(char const* path, char const* mode) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	return fopen(spath, mode);
}

DIR*
file_opendir(char const* path) {
	char spath[FILE_NPATH];

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
	char spath[FILE_NPATH];

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
	char spath[FILE_NPATH];

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
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, dirpath)) {
		WARN("Failed to solve path \"%s\"", dirpath);
		return -1;
	}

#if defined(_WIN32) || defined(_WIN64)
	return mkdir(spath);
#else
	return mkdir(spath, mode);
#endif
}

bool
file_create(char const* path) {
	FILE* fout = file_open(path, "wb");
	if (!fout) {
		return false;
	}
	file_close(fout);
	return true;
}

char*
file_read_string(FILE* fin) {
	// Check arguments
	if (!fin || feof(fin)) {
		WARN("Invalid stream");
		return NULL;
	}

	// Buffer for read stream
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		return NULL;
	}

	// Read stream
	int ch;
	for (;;) {
		ch = fgetc(fin);
		if (ch == EOF || ferror(fin)) {
			goto done;
		}
		buffer_push(buf, ch);
	}

done:
	// Done
	buffer_push(buf, '\0');
	return buffer_escape_delete(buf);
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
