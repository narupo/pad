/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2017, 2018
 */
#include "file.h"

int32_t
file_close(FILE *fp) {
	if (!fp) {
		return -1;
	}

	return fclose(fp);
}

FILE *
file_open(const char *path, const char *mode) {
	if (!path || !mode) {
		return NULL;
	}

	return fopen(path, mode);
}

bool
file_copy(FILE *dst, FILE *src) {
	if (!dst || !src) {
		return false;
	}

	int64_t tell = ftell(src);
	for (int c; (c = fgetc(src)) != EOF; fputc(c, dst)) {
	}

	fflush(dst);
	return fseek(src, tell, SEEK_SET) == 0;
}

char *
file_realpath(char *dst, uint32_t dstsz, const char *src) {
	if (!dst || dstsz == 0 || !src) {
		return NULL;
	}

#if defined(_FILE_WINDOWS)
	char *fpart;
	if (!GetFullPathName(src, dstsz, dst, &fpart)) {
		return NULL;
	}
#else
	errno = 0;

	if (!realpath(src, dst)) {
		if (errno == ENOENT) {
			// Path is not exists
			snprintf(dst, dstsz, "%s", src);
		} else {
			return NULL;
		}
	}
#endif
	return dst;
}

char *
file_get_user_home(char *dst, uint32_t dstsz) {
#ifdef _FILE_WINDOWS	
	const char *drive = getenv("HOMEDRIVE");
	if (!drive) {
		return NULL;
	}

	const char *userhome = getenv("HOMEPATH");
	if (!userhome) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s%s", drive, userhome);	
#else
	const char *userhome = getenv("HOME");
	if (!userhome) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s", userhome);	
#endif
	return dst;
}

char *
file_solve(char *dst, uint32_t dstsz, const char *path) {
	// Check arugments
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	char tmp[FILE_NPATH*2];

	// Solve '~'
	if (path[0] == '~') {
		char home[FILE_NPATH];
		if (!file_get_user_home(home, sizeof home)) {
			return NULL;
		}
		const char *p = path;
		for (;*p == '~' || *p == '/'; ++p);
		snprintf(tmp, sizeof tmp, "%s/%s", home, p);
	} else {
		snprintf(tmp, sizeof tmp, "%s", path);
	}

	// Get real path
	if (!file_realpath(dst, dstsz, tmp)) {
		return NULL;
	}

	return dst;
}

char *
file_solvecp(const char *path) {
	// Check arguments
	if (!path) {
		return NULL;
	}

	// Ready
	char *dst = malloc(sizeof(char) * FILE_NPATH);
	if (!dst) {
		return NULL;
	}

	// Solve
	char *res = file_solve(dst, FILE_NPATH, path);
	if (!res) {
		free(dst);
		return NULL;
	}

	return res;
}

char *
file_solvefmt(char *dst, uint32_t dstsz, const char *fmt, ...) {
	if (!dst || dstsz == 0 || !fmt) {
		return NULL;
	}

	va_list ap;
	va_start(ap, fmt);
	char tmp[FILE_NPATH];
	vsnprintf(tmp, sizeof tmp, fmt, ap);
	va_end(ap);
	return file_solve(dst, dstsz, tmp);
}

DIR*
file_opendir(const char *path) {
	if (!path) {
		return NULL;
	}

	return opendir(path);
}

int32_t
file_closedir(DIR* dir) {
	if (!dir) {
		return -1;
	}

	return closedir(dir);
}

bool
file_exists(const char *path) {
	if (!path) {
		return false;
	}

	struct stat s;
	int res = stat(path, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			return false; // Does not exists
		} else {
			perror("stat");
			exit(1);
		}
	}

	return true;
}

bool
file_isdir(const char *path) {
	if (!path) {
		return false;
	}

	struct stat s;
	int res = stat(path, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			;
		} else {
			// Error. Break through
		}
		return false; // Doe's not exists 
	} else {
		if (S_ISDIR(s.st_mode)) {
			return true;
		} else {
			return false;
		}
	}

	return false; // Impossible
}

int32_t
file_mkdirmode(const char *dirpath, mode_t mode) {
	if (!dirpath) {
		return -1;
	}

#if defined(_FILE_WINDOWS)
	return mkdir(dirpath);
#else
	return mkdir(dirpath, mode);
#endif
}

int32_t
file_mkdirq(const char *path) {
	if (!path) {
		return -1;
	}

	return file_mkdirmode(path, S_IRUSR | S_IWUSR | S_IXUSR);
}

bool
file_trunc(const char *path) {
	if (!path) {
		return false;
	}

	FILE* fout = file_open(path, "wb");
	if (!fout) {
		return false;
	}

	file_close(fout);
	
	return true;
}

char *
file_readcp(FILE* fin) {
	if (!fin || feof(fin)) {
		return NULL;
	}

	uint32_t size = file_size(fin);
	char *dst = malloc(sizeof(char)*size+1); // +1 for final nul
	if (!dst) {
		return NULL;
	}

	fread(dst, sizeof(char), size, fin);
	if (ferror(fin)) {
		return NULL;
	}

	if (feof(fin)) {
		return NULL;
	}

	dst[size] = '\0';

	return dst;
}

int64_t
file_size(FILE* fp) {
	if (!fp) {
		return -1;
	}

	int64_t curtel = ftell(fp);
	int64_t size = 0;

	fseek(fp, 0L, SEEK_SET);

	for (; fgetc(fp) != EOF; ) {
		++size;
	}

	fseek(fp, curtel, SEEK_SET);

	return size;
}

const char *
file_suffix(const char *path) {
	if (!path) {
		return NULL;
	}

	const char *suf = strrchr(path, '.');
	if (!suf) {
		return path;
	}

	return suf + 1;
}

char *
file_dirname(char *dst, uint32_t dstsz, const char *path) {
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s", path);

	return dirname(dst);
}

char *
file_basename(char *dst, uint32_t dstsz, const char *path) {
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	char tmp[dstsz];
	snprintf(tmp, sizeof tmp, "%s", path);
	char *p = basename(tmp);
	if (!p) {
		return p;
	}

	snprintf(dst, dstsz, "%s", p);
	
	return dst;
}

int32_t
file_getline(char *dst, uint32_t dstsz, FILE *fin) {
	if (!dst || dstsz == 0 || !fin) {
		return EOF;
	}

	if (!fgets(dst, dstsz, fin)) {
		return EOF;
	}

	int32_t dstlen = strlen(dst);
	if (dst[dstlen-1] == '\n') {
		dst[--dstlen] = '\0';
	}

	return dstlen;
}

char *
file_readline(char *dst, uint32_t dstsz, const char *path) {
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	FILE *fin = fopen(path, "rb");
	if (!fin) {
		if (dstsz) {
			*dst = '\0';
		}
		return NULL;
	}

	if (file_getline(dst, dstsz, fin) == EOF) {
		fclose(fin);
		if (dstsz) {
			*dst = '\0';
		}
		return NULL;
	}

	if (fclose(fin) < 0) {
		if (dstsz) {
			*dst = '\0';
		}
		return NULL;
	}

	return dst;
}

const char *
file_writeline(const char *line, const char *path) {
	if (!line || !path) {
		return NULL;
	}

	FILE *fout = fopen(path, "wb");
	if (!fout) {
		return NULL;
	}

	fprintf(fout, "%s\n", line);
	fflush(fout);

	if (fclose(fout) < 0) {
		return NULL;
	}

	return line;
}

/*********************
* file file_dirnode *
*********************/

struct file_dirnode {
#if defined(_FILE_WINDOWS)
	WIN32_FIND_DATA finddata;
#else
	struct dirent* node;
#endif
};

/*******************************
* file_dirnode delete and new *
*******************************/

void
file_dirnodedel(struct file_dirnode *self) {
	if (self) {
		free(self);
	}
}

struct file_dirnode *
file_dirnodenew(void) {
	struct file_dirnode *self = calloc(1, sizeof(struct file_dirnode));
	if (!self) {
		return NULL;
	}
	return self;
}

/*********************
* file_dirnode getter *
*********************/

const char *
file_dirnodename(const struct file_dirnode *self) {
	if (!self) {
		return NULL;
	}

#if defined(_FILE_WINDOWS)
	return self->finddata.cFileName;
#else
	return self->node->d_name;
#endif
}

/**********************
* file struct file_dir *
**********************/

struct file_dir {
#if defined(_FILE_WINDOWS)
	HANDLE handle;
	char dirpath[FILE_NPATH];
#else
	DIR* directory;
#endif
};

/********************************
* struct file_dir close and open *
********************************/

int32_t
file_dirclose(struct file_dir *self) {
	if (self) {
		int32_t ret = 0;
#if defined(_FILE_WINDOWS)
		if (self->handle) {
			ret = FindClose(self->handle);
			if (ret == 0) {
				// Error
			}
			self->handle = NULL;
			ret = !ret;
		} else {
			ret = -1;
		}

#else
		ret = closedir(self->directory);
		if (ret != 0) {
			// Error
		}
#endif
		free(self);

		return ret;
	}

	return -1;
}

struct file_dir *
file_diropen(const char *path) {
	if (!path) {
		return NULL;
	}

	struct file_dir *self = calloc(1, sizeof(struct file_dir));
	if (!self) {
		return NULL;
	}

#if defined(_FILE_WINDOWS)
	if (!file_exists(path)) {
		return NULL;
	}
	self->handle = NULL;
	snprintf(self->dirpath, sizeof(self->dirpath), "%s/*", path);

#else
	if (!(self->directory = opendir(path))) {
		free(self);
		return NULL;
	}

#endif
	return self;
}

/************************
* struct file_dir getter *
************************/

struct file_dirnode *
file_dirread(struct file_dir *self) {
	if (!self) {
		return NULL;
	}
	
	struct file_dirnode * node = file_dirnodenew();
	if (!node) {
		return NULL;
	}

#if defined(_FILE_WINDOWS)
	if (!self->handle) {
		if ((self->handle = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
			file_dirnodedel(node);
			return NULL;

		}

	} else {
		if (!FindNextFile(self->handle, &node->finddata)) {
			file_dirnodedel(node);
			return NULL; // Done to find
		}
	}

#else
	errno = 0;
	if (!(node->node = readdir(self->directory))) {
		if (errno != 0) {
			file_dirnodedel(node);
			return NULL;
		} else {
			// Done to readdir
			file_dirnodedel(node);
			return NULL;
		}
	}
#endif

	return node;
}
