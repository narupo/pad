#include "file.h"

enum {
    NFILE_PATH = 256,
};

char*
file_make_solve_path(char const* path) {
    char tmp[NFILE_PATH];
    char* dst = (char*) malloc(sizeof(char) * NFILE_PATH);
    if (!dst)
        die("malloc");

    // Solve '~'
    if (path[0] == '~') {
        snprintf(tmp, NFILE_PATH, "%s%s", getenv("HOME"), path+1);
    } else {
        snprintf(tmp, NFILE_PATH, "%s", path);
    }

    // Solve real path
    errno = 0;
    if (!realpath(tmp, dst)) {
        if (errno == ENOENT) {
            // Path is not exists
            strcpy(dst, tmp);
        } else {
            die("realpath");
        }
    }

    return dst;
}

FILE*
file_open(char const* path, char const* mode) {
    char* spath = file_make_solve_path(path);
    if (!spath) {
        WARNF("Failed to make solve path \"%s\"", path);
        return NULL;
    }

    FILE* stream = fopen(spath, mode);
    free(spath);

    return stream;
}

DIR*
file_opendir(char const* path) {
    char* spath = file_make_solve_path(path);
    if (!spath) {
        WARNF("Failed to make solve path \"%s\"", path);
        return NULL;
    }

    DIR* dir = opendir(spath);
    free(spath);

    return dir;
}

int
file_close(FILE* fp) {
    return fclose(fp);
}

bool
file_is_exists(char const* path) {
    char* spath = file_make_solve_path(path);
    if (!spath) {
        WARNF("Failed to make solve path \"%s\"", path);
    }
    struct stat s;
    int res = stat(spath, &s);
    free(spath);

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
    char* spath = file_make_solve_path(path);
    if (!spath) {
        WARN("Failed to make solve path \"%s\"", spath);
        return false;
    }
    struct stat s;
    int res = stat(spath, &s);
    free(spath);

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
    char* path = file_make_solve_path(dirpath);
    int res = mkdir(path, mode);
    free(path);
    return res;
}

#if defined(TEST)
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

int
main(int argc, char* argv[]) {
    test_mkdir(argc, argv);
    return 0;
}
#endif

