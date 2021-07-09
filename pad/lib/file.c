/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2017, 2018
 */
#include <pad/lib/file.h>

int32_t
PadFile_Close(FILE *fp) {
    if (!fp) {
        return -1;
    }

    return fclose(fp);
}

FILE *
PadFile_Open(const char *path, const char *mode) {
    if (!path || !mode) {
        return NULL;
    }

    return fopen(path, mode);
}

bool
PadFile_Copy(FILE *dst, FILE *src) {
    if (!dst || !src) {
        return false;
    }

    int64_t tell = ftell(src);
    for (int c; (c = fgetc(src)) != EOF; fputc(c, dst)) {
    }

    fflush(dst);
    return fseek(src, tell, SEEK_SET) == 0;
}

bool
PadFile_CopyPath(const char *dst, const char *src) {
    if (!dst || !src) {
        return false;
    }

    FILE *fin = NULL;
    FILE *fout = NULL;

    fin = fopen(src, "rb");
    if (fin == NULL) {
        goto error;
    }

    fout = fopen(dst, "wb");
    if (fout == NULL) {
        goto error;
    }

    bool result = PadFile_Copy(fout, fin);

    fclose(fout);
    fclose(fin);
    return result;
error:
    if (fout) fclose(fout);
    if (fin) fclose(fin);
    return false;
}

char *
PadFile_RealPath(char *dst, uint32_t dstsz, const char *src) {
    if (!dst || dstsz == 0 || !src) {
        return NULL;
    }

#if defined(PAD_FILE__WINDOWS)
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
PadFile_GetUserHome(char *dst, uint32_t dstsz) {
#ifdef PAD_FILE__WINDOWS
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
PadFile_Solve(char *dst, uint32_t dstsz, const char *path) {
    // Check arugments
    if (!dst || dstsz == 0 || !path) {
        return NULL;
    }

    char tmp[PAD_FILE__NPATH*2];

    // Solve '~'
    if (path[0] == '~') {
        char home[PAD_FILE__NPATH];
        if (!PadFile_GetUserHome(home, sizeof home)) {
            return NULL;
        }
        const char *p = path;
        for (;*p == '~' || *p == '/'; ++p);
        snprintf(tmp, sizeof tmp, "%s/%s", home, p);
    } else {
        snprintf(tmp, sizeof tmp, "%s", path);
    }

    // Get real path
    if (!PadFile_RealPath(dst, dstsz, tmp)) {
        return NULL;
    }

    return dst;
}

char *
PadFile_SolveCopy(const char *path) {
    // Check arguments
    if (!path) {
        return NULL;
    }

    // Ready
    char *dst = malloc(sizeof(char) * PAD_FILE__NPATH);
    if (!dst) {
        return NULL;
    }

    // Solve
    char *res = PadFile_Solve(dst, PAD_FILE__NPATH, path);
    if (!res) {
        free(dst);
        return NULL;
    }

    return res;
}

char *
PadFile_SolveFmt(char *dst, uint32_t dstsz, const char *fmt, ...) {
    if (!dst || dstsz == 0 || !fmt) {
        return NULL;
    }

    va_list ap;
    va_start(ap, fmt);
    char tmp[PAD_FILE__NPATH];
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    return PadFile_Solve(dst, dstsz, tmp);
}

int32_t
PadFile_Remove(const char *path) {
#ifdef PAD_FILE__WINDOWS
    if (PadFile_IsDir(path)) {
        if (RemoveDirectory(path)) {
            return 0; // success
        } else {
            return remove(path);
        }
    }
#endif
    return remove(path);
}

int32_t
PadFile_Rename(const char *old, const char *new) {
    return rename(old, new);
}

DIR*
PadFile_OpenDir(const char *path) {
    if (!path) {
        return NULL;
    }

    return opendir(path);
}

int32_t
PadFile_CloseDir(DIR* dir) {
    if (!dir) {
        return -1;
    }

    return closedir(dir);
}

bool
PadFile_IsExists(const char *path) {
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
PadFile_IsDir(const char *path) {
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
PadFile_MkdirMode(const char *dirpath, mode_t mode) {
    if (!dirpath) {
        return -1;
    }

#if defined(PAD_FILE__WINDOWS)
    return mkdir(dirpath);
#else
    return mkdir(dirpath, mode);
#endif
}

int32_t
PadFile_MkdirQ(const char *path) {
    if (!path) {
        return -1;
    }

    return PadFile_MkdirMode(path, S_IRUSR | S_IWUSR | S_IXUSR);
}

int32_t
PadFile_MkdirsQ(const char *path) {
    if (!path) {
        return -1;
    }

    char tmp[strlen(path)+1];
    int i = 0;
    for (const char *p = path; *p && i < PAD_FILE__NPATH; ++p) {
        tmp[i++] = *p;
        tmp[i] = '\0';
        if (*p == PAD_FILE__SEP && !PadFile_IsExists(tmp)) {
            PadFile_MkdirQ(tmp);
        }
    }

    PadFile_MkdirQ(tmp);
    return 0;
}

bool
PadFile_Trunc(const char *path) {
    if (!path) {
        return false;
    }

    FILE* fout = PadFile_Open(path, "wb");
    if (!fout) {
        return false;
    }

    PadFile_Close(fout);

    return true;
}

char *
PadFile_ReadCopy(FILE* fin) {
    if (!fin || feof(fin)) {
        return NULL;
    }

    int32_t size = 4;
    int32_t len = 0;
    char *dst = malloc(sizeof(char)*size+1); // +1 for final nul
    if (!dst) {
        return NULL;
    }

    for (;;) {
        int c = fgetc(fin);
        if (c == EOF) {
            break;
        }
        if (len >= size) {
            size *= 2;
            char *tmpdst = realloc(dst, sizeof(char)*size+1);
            if (!tmpdst) {
                free(dst);
                return NULL;
            }
            dst = tmpdst;
        }

        dst[len++] = c;
    }

    dst[len] = '\0';

    return dst;
}

char *
PadFile_ReadCopyFromPath(const char *path) {
    FILE *fin = fopen(path, "rb");
    if (fin == NULL) {
        return NULL;
    }

    char *content = PadFile_ReadCopy(fin);

    if (fclose(fin) == EOF) {
        return NULL;
    }

    return content;
}

int64_t
PadFile_Size(FILE* fp) {
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
PadFile_Suffix(const char *path) {
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
PadFile_DirName(char *dst, uint32_t dstsz, const char *path) {
    if (!dst || dstsz == 0 || !path) {
        return NULL;
    }

    snprintf(dst, dstsz, "%s", path);

    return dirname(dst);
}

char *
PadFile_BaseName(char *dst, uint32_t dstsz, const char *path) {
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
PadFile_GetLine(char *dst, uint32_t dstsz, FILE *fin) {
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
PadFile_ReadLine(char *dst, uint32_t dstsz, const char *path) {
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

    if (PadFile_GetLine(dst, dstsz, fin) == EOF) {
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
PadFile_WriteLine(const char *line, const char *path) {
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

struct PadDirNode {
#if defined(PAD_FILE__WINDOWS)
    WIN32_FIND_DATA finddata;
#else
    struct dirent* node;
#endif
};

/*******************************
* file_dirnode delete and new *
*******************************/

void
PadDirNode_Del(PadDirNode *self) {
    if (self) {
        free(self);
    }
}

PadDirNode *
file_dirnodenew(void) {
    PadDirNode *self = calloc(1, sizeof(PadDirNode));
    if (!self) {
        return NULL;
    }
    return self;
}

/**********************
* file_dirnode getter *
**********************/

const char *
PadDirNode_Name(const PadDirNode *self) {
    if (!self) {
        return NULL;
    }

#if defined(PAD_FILE__WINDOWS)
    return self->finddata.cFileName;
#else
    return self->node->d_name;
#endif
}

/***********************
* file struct PadDir *
***********************/

struct PadDir {
#if defined(PAD_FILE__WINDOWS)
    HANDLE handle;
    char dirpath[PAD_FILE__NPATH];
#else
    DIR* directory;
#endif
};

/*********************************
* struct PadDir close and open *
*********************************/

int32_t
PadDir_Close(PadDir *self) {
    if (self) {
        int32_t ret = 0;
#if defined(PAD_FILE__WINDOWS)
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

PadDir *
PadDir_Open(const char *path) {
    if (!path) {
        return NULL;
    }

    PadDir *self = calloc(1, sizeof(PadDir));
    if (!self) {
        return NULL;
    }

#if defined(PAD_FILE__WINDOWS)
    if (!PadFile_IsExists(path)) {
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

/********************
* PadDir getter *
********************/

PadDirNode *
PadDir_Read(PadDir *self) {
    if (!self) {
        return NULL;
    }

    PadDirNode * node = file_dirnodenew();
    if (!node) {
        return NULL;
    }

#if defined(PAD_FILE__WINDOWS)
    if (!self->handle) {
        if ((self->handle = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
            PadDirNode_Del(node);
            return NULL;

        }

    } else {
        if (!FindNextFile(self->handle, &node->finddata)) {
            PadDirNode_Del(node);
            return NULL; // Done to find
        }
    }

#else
    errno = 0;
    if (!(node->node = readdir(self->directory))) {
        if (errno != 0) {
            PadDirNode_Del(node);
            return NULL;
        } else {
            // Done to readdir
            PadDirNode_Del(node);
            return NULL;
        }
    }
#endif

    return node;
}

char *
PadFile_ConvLineEnc(const char *encoding, const char *text) {
    if (!encoding || !text) {
        return NULL;
    }

    bool to_crlf = strcasecmp(encoding, "crlf") == 0;
    bool to_cr = strcasecmp(encoding, "cr") == 0;
    bool to_lf = strcasecmp(encoding, "lf") == 0;

    if (!to_crlf && !to_cr && !to_lf) {
        return NULL;
    }

    int32_t di = 0;
    int32_t dcapa = strlen(text);
    int32_t elsize = sizeof(char);
    char *dst = calloc(dcapa+1, elsize); // +1 for final nul
    if (!dst) {
        return NULL;
    }

    // -1 of dcapa-1 for crlf
#define resize() { \
        if (di >= dcapa-1) { \
            dcapa *= 2; \
            int32_t size = dcapa + elsize; \
            char *tmp = realloc(dst, size); \
            if (!tmp) { \
                free(dst); \
                return NULL; \
            } \
            dst = tmp; \
        } \
    }

#define conv() { \
        if (to_crlf) { \
            dst[di++] = '\r'; \
            dst[di++] = '\n'; \
        } else if (to_cr) { \
            dst[di++] = '\r'; \
        } else if (to_lf) { \
            dst[di++] = '\n'; \
        } \
    }

    for (const char *p = text; *p; ++p) {
        if (*p == '\r' && *(p+1) == '\n') {
            resize();
            ++p;
            conv();
        } else if (*p == '\r') {
            resize();
            conv();
        } else if (*p == '\n') {
            resize();
            conv();
        } else {
            resize();
            dst[di++] = *p;
        }
    }

    dst[di] = '\0';
    return dst;
}

int
PadFile_GetNum(FILE *fp) {
    return fileno(fp);
}

char **
PadFile_ReadLines(const char *fname) {
    FILE *fin = fopen(fname, "r");
    if (!fin) {
        return NULL;
    }

    int32_t linessize = 4;
    char **lines = calloc(linessize + 1, sizeof(char *));
    int32_t linesi = 0;

    int32_t bufsize = 4;
    char *buf = calloc(bufsize + 1, sizeof(char));
    int32_t bi = 0;

    bool active = true;

    for (; active ;) {
        int32_t c = fgetc(fin);
        if (c == EOF) {
            active = false;
            c = '\n';
        }

        if (c == '\n') {
            if (!strlen(buf)) {
                continue;
            }
            if (linesi >= linessize) {
                linessize *= 2;
                lines = realloc(lines, linessize * sizeof(char *) + sizeof(char *));
            }
            lines[linesi++] = strdup(buf);
            lines[linesi] = NULL;
            buf[0] = '\0';
            bi = 0;
            continue;
        }

        if (bi >= bufsize) {
            bufsize *= 2;
            buf = realloc(buf, bufsize * sizeof(char) + sizeof(char));
        }
        buf[bi++] = c;
        buf[bi] = '\0';
    }

    free(buf);
    fclose(fin);
    return lines;
}
