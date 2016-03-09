#include "file.h"

char*
file_realpath(char* dst, size_t dstsize, char const* src) {
#if defined(_CAP_WINDOWS)
	char* fpart;

	if (!GetFullPathName(src, dstsize, dst, &fpart)) {
		WARN("Failed to solve path");
		return NULL;
	}

#else
	errno = 0;

	if (!realpath(src, dst)) {
		if (errno == ENOENT) {
			// Path is not exists
			snprintf(dst, dstsize, "%s", src);
		} else {
			WARN("Failed to realpath \"%s\"", src);
			return NULL;
		}
	}

#endif
	return dst;
}

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
		snprintf(tmp, FILE_NPATH, "%s/%s", environ_get("HOME"), path+1);
	} else {
		snprintf(tmp, FILE_NPATH, "%s", path);
	}

	if (!file_realpath(dst, dstsize, tmp)) {
		WARN("Failed to realpath \"%s\"", tmp);
		return NULL;
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
	return fopen(path, mode);
}

DIR*
file_opendir(char const* path) {
	return opendir(path);
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
	struct stat s;
	int res = stat(path, &s);

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
	struct stat s;
	int res = stat(path, &s);

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
file_mkdir_mode(char const* dirpath, mode_t mode) {
#if defined(_CAP_WINDOWS)
	return mkdir(dirpath);
#else
	return mkdir(dirpath, mode);
#endif
}

int
file_mkdir(char const* dirpath, char const* mode) {
#if defined(_CAP_WINDOWS)
	return mkdir(dirpath);
#else
	DIE("TODO");
	return -1;
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

	size_t size = file_size(fin);
	char* dst = (char*) malloc(sizeof(char)*size+1); // +1 for final nul
	if (!dst) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	if (fread(dst, sizeof(char), size, fin) < size) {
		WARN("Failed to read from stream");
		return NULL;
	}
	dst[size] = '\0';

	return dst;
}

char*
file_escape_blanks(char* dst, size_t dstsize, char const* src) {
	char const* dstend = dst + dstsize - 1; // -1 for final nul

	for (; dst < dstend && *src; ++dst, ++src) {
		if (isblank(*src)) {
			*dst = '\\';
			++dst;
		}
		*dst = *src;
	}

	*dst = '\0';
	return dst;
}

char*
file_getline(char* dst, size_t dstsize, FILE* fin) {
	// Check arguments
	if (!dst || !dstsize || !fin || feof(fin) || ferror(fin)) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Read from stream
	char* cur = dst;
	char const* end = dst + dstsize - 1; // +1 for final nul

	for (; cur < end; ) {
		int ch = fgetc(fin);
		if (ch == EOF || ferror(fin)) {
			break;
		}

		// Check newline
		if (ch == '\n') {
			// UNIX
			break;
		} else if (ch == '\r') {
			int next = fgetc(fin);
			if (next == '\n') {
				// MS Windows
				break;
			} else {
				// Mac
				ungetc(next, fin);
				break;
			}
		}

		*cur++ = ch;
	}

	// Done
	*cur = '\0';
	return dst;
}

long
file_size(FILE* stream) {
	// ("TODO: https://www.securecoding.cert.org/confluence/display/c/FIO19-C.+Do+not+use+fseek%28%29+and+ftell%28%29+to+compute+the+size+of+a+regular+file");
	DIE("TODO");
	return 0L;
}

char const*
file_suffix(char const* path) {
	if (!path) {
		return "";
	}
	
	char const* suf = strrchr(path, '.');
	if (!suf) {
		return path;
	}
	return suf + 1;
}

char*
file_read_script_line(char* dst, size_t dstsize, FILE* stream) {
	if (!dst || feof(stream) || ferror(stream)) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Read line for parse and runtime script name
	char line[dstsize];
	int tell = ftell(stream);

	if (!file_getline(line, dstsize, stream)) {
		WARN("Failed to read line");
		return NULL;
	}

	// Check prefix
	char const* pref = "#!";
	size_t preflen = strlen(pref);

	if (strncmp(line, pref, preflen) != 0) {
		fseek(stream, tell, SEEK_SET);
		return NULL; // Not found
	}

	// Parse script name
	char* src = line + preflen;

#if defined(_CAP_WINDOWS)
	char* p = strrchr(src, '/');
	if (p) {
		p += 1; // +1 for '/'
	} else {
		p = src;
	}

	snprintf(dst, dstsize, "%s", p);
	
#else
	snprintf(dst, dstsize, "%s", src);

#endif

	return dst;
}

/*********************
* file DirectoryNode *
*********************/

struct DirectoryNode {
#if defined(_CAP_WINDOWS)
	WIN32_FIND_DATA finddata;
#else
	struct dirent* node;
#endif	
};

/*******************************
* DirectoryNode delete and new *
*******************************/

void
dirnode_delete(DirectoryNode* self) {
	if (self) {
		free(self);
	}
}

DirectoryNode*
dirnode_new(void) {
	DirectoryNode* self = (DirectoryNode*) calloc(1, sizeof(DirectoryNode));
	if (!self) {
		WARN("Failed to construct DirectoryNode");
		return NULL;
	}
	return self;
}

/***********************
* DirectoryNode getter *
***********************/

char const*
dirnode_name(DirectoryNode const* self) {
#if defined(_CAP_WINDOWS)
	return self->finddata.cFileName;
#else
	return self->node->d_name;
#endif
}

/*****************
* file Directory *
*****************/

struct Directory {
#if defined(_CAP_WINDOWS)
	HANDLE handle;
	char dirpath[FILE_NPATH];
#else
	DIR* directory;
#endif
};

/***************************
* Directory close and open *
***************************/

void
dir_close(Directory* self) {
	if (self) {
#if defined(_CAP_WINDOWS)
		if (self->handle) {
			if (!FindClose(self->handle)) {
				WARN("Failed to close directory");
			}
			self->handle = NULL;
		}

#else
		if (closedir(self->directory) != 0) {
			WARN("Failed to close directory");
		}
#endif	
		
		free(self);
	}
}

Directory*
dir_open(char const* path) {
	Directory* self = (Directory*) calloc(1, sizeof(Directory));
	if (!self) {
		WARN("Failed to construct Directory");
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!file_is_exists(path)) {
		WARN("Not found path \"%s\"", path);
		return NULL;
	}
	self->handle = NULL;
	snprintf(self->dirpath, sizeof(self->dirpath), "%s/*", path);

#else
	if (!(self->directory = opendir(path))) {
		WARN("Failed to open directory \"%s\"", path);
		free(self);
		return NULL;
	}

#endif
	return self;
}

/*******************
* Directory getter *
*******************/

DirectoryNode*
dir_read_node(Directory* self) {
	DirectoryNode* node = dirnode_new();
	if (!node) {
		WARN("Failed to construct DirectoryNode");
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!self->handle) {
		if ((self->handle = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
			WARN("Failed to open directory \"%s\"", self->dirpath);
			dirnode_delete(node);
			return NULL;

		}

	} else {
		if (!FindNextFile(self->handle, &node->finddata)) {
			dirnode_delete(node);
			return NULL; // Done to find
		}
	}

#else
	errno = 0;
	if (!(node->node = readdir(self->directory))) {
		if (errno != 0) {
			dirnode_delete(node);
			WARN("Failed to readdir");
			return NULL;
		} else {
			// Done to readdir
			dirnode_delete(node);
			return NULL;
		}
	}
#endif

	return node;
}

/************
* file test *
************/

#if defined(TEST_FILE)
static int
test_mkdir(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	char const* path = argv[1];

	if (file_is_exists(path)) {
		printf("is exists [%s]\n", path);
	} else {
		printf("is not exists [%s]\n", path);
		file_mkdir_mode(path, S_IRUSR | S_IWUSR | S_IXUSR);
	}

	return 0;
}

static char*
solve_path(char* dst, size_t dstsize, char const* path) {
#if defined(_CAP_WINDOWS)
	char* fpart;

	if (!GetFullPathName(path, dstsize, dst, &fpart)) {
		WARN("Failed to solve path");
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
	printf("[%s] -> \n[%s]\n", argv[1], file_solve_path(dst, sizeof dst, argv[1]));

	return 0;
}

static int
test_directory(int argc, char* argv[]) {
#if defined(_CAP_WINDOWS)
	char const* dirpath = "C:/Windows/Temp";
	
	if (argc >= 2) {
		dirpath = argv[1];
	}

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		WARN("Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		fprintf(stderr, "name[%s]\n", dirnode_name(node));
		dirnode_delete(node);
	} 

	fflush(stderr);
	dir_close(dir);
	return 0;

#else
	char const* dirpath = "/tmp";
	
	if (argc >= 2) {
		dirpath = argv[1];
	}

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		WARN("Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		printf("name[%s]\n", dirnode_name(node));
		dirnode_delete(node);
	} 

	dir_close(dir);
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
