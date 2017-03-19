/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "file.h"

int32_t
cap_fclose(FILE *fp) {
	if (!fp) {
		return -1;
	}

	return fclose(fp);
}

FILE *
cap_fopen(const char *path, const char *mode) {
	if (!path || !mode) {
		return NULL;
	}

	return fopen(path, mode);
}

bool
cap_fcopy(FILE *dst, FILE *src) {
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
cap_frealpath(char *dst, uint32_t dstsz, const char *src) {
	if (!dst || dstsz == 0 || !src) {
		return NULL;
	}

#if defined(_CAP_WINDOWS)
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
cap_fsolve(char *dst, uint32_t dstsz, const char *path) {
	// Check arugments
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	char tmp[FILE_NPATH];

	// Solve '~'
	if (path[0] == '~') {
		snprintf(tmp, sizeof tmp, "%s/%s", getenv("HOME"), path+1);
	} else {
		snprintf(tmp, sizeof tmp, "%s", path);
	}

	if (!cap_frealpath(dst, dstsz, tmp)) {
		return NULL;
	}

	return dst;
}

char *
cap_fsolvecp(const char *path) {
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
	char *res = cap_fsolve(dst, FILE_NPATH, path);
	if (!res) {
		free(dst);
		return NULL;
	}

	return res;
}

char *
cap_fsolvefmt(char *dst, uint32_t dstsz, const char *fmt, ...) {
	if (!dst || dstsz == 0 || !fmt) {
		return NULL;
	}

	va_list ap;
	va_start(ap, fmt);
	char tmp[FILE_NPATH];
	vsnprintf(tmp, sizeof tmp, fmt, ap);
	va_end(ap);
	return cap_fsolve(dst, dstsz, tmp);
}

DIR*
cap_fopendir(const char *path) {
	if (!path) {
		return NULL;
	}

	return opendir(path);
}

int32_t
cap_fclosedir(DIR* dir) {
	if (!dir) {
		return -1;
	}

	return closedir(dir);
}

bool
cap_fexists(const char *path) {
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
cap_fisdir(const char *path) {
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
cap_fmkdirmode(const char *dirpath, mode_t mode) {
	if (!dirpath) {
		return -1;
	}

#if defined(_CAP_WINDOWS)
	return mkdir(dirpath);
#else
	return mkdir(dirpath, mode);
#endif
}

int32_t
cap_fmkdirq(const char *path) {
	if (!path) {
		return -1;
	}

	return cap_fmkdirmode(path, S_IRUSR | S_IWUSR | S_IXUSR);
}

bool
cap_ftrunc(const char *path) {
	if (!path) {
		return false;
	}

	FILE* fout = cap_fopen(path, "wb");
	if (!fout) {
		return false;
	}

	cap_fclose(fout);
	
	return true;
}

char *
cap_freadcp(FILE* fin) {
	if (!fin || feof(fin)) {
		return NULL;
	}

	uint32_t size = cap_fsize(fin);
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
cap_fsize(FILE* fp) {
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
cap_fsuffix(const char *path) {
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
cap_fdirname(char *dst, uint32_t dstsz, const char *path) {
	if (!dst || dstsz == 0 || !path) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s", path);

	return dirname(dst);
}

char *
cap_fbasename(char *dst, uint32_t dstsz, const char *path) {
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
cap_fgetline(char *dst, uint32_t dstsz, FILE *fin) {
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
cap_freadline(char *dst, uint32_t dstsz, const char *path) {
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

	if (cap_fgetline(dst, dstsz, fin) == EOF) {
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
cap_fwriteline(const char *line, const char *path) {
	if (!line || !path) {
		return NULL;
	}

	FILE *fout = fopen(path, "w");
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
cap_dirnodedel(struct cap_dirnode *self) {
	if (self) {
		free(self);
	}
}

struct cap_dirnode *
cap_dirnodenew(void) {
	struct cap_dirnode *self = (struct cap_dirnode *) calloc(1, sizeof(struct cap_dirnode));
	if (!self) {
		return NULL;
	}
	return self;
}

/***********************
* cap_dirnode getter *
***********************/

const char *
cap_dirnodename(const struct cap_dirnode *self) {
	if (!self) {
		return NULL;
	}

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

int32_t
cap_dirclose(struct cap_dir *self) {
	if (self) {
		int32_t ret = 0;
#if defined(_CAP_WINDOWS)
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

struct cap_dir *
cap_diropen(const char *path) {
	if (!path) {
		return NULL;
	}

	struct cap_dir *self = calloc(1, sizeof(struct cap_dir));
	if (!self) {
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!cap_fexists(path)) {
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
* struct cap_dir getter *
************************/

struct cap_dirnode *
cap_dirread(struct cap_dir *self) {
	if (!self) {
		return NULL;
	}
	
	struct cap_dirnode * node = cap_dirnodenew();
	if (!node) {
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (!self->handle) {
		if ((self->handle = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
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
