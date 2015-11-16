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
    if (path[0] == '~')
        snprintf(tmp, NFILE_PATH, "%s%s", getenv("HOME"), path+1);
    else
        snprintf(tmp, NFILE_PATH, "%s", path);

    // Solve real path
    errno = 0;
    if (!realpath(tmp, dst)) {
        if (errno == ENOENT)
            // Path is not exists
            strcpy(dst, tmp);
        else
            die("realpath");
    }

    return dst;
}

FILE*
file_open(char const* name, char const* mode) {
    char* path = file_make_solve_path(name);
    FILE* stream = fopen(path, mode);
    free(path);
    return stream;
}

int
file_close(FILE* fp) {
    return fclose(fp);
}

bool
file_is_exists(char const* path) {
    struct stat s;
    char* spath = file_make_solve_path(path);

    int res = stat(spath, &s);
    if (res == -1) {
        if (errno == ENOENT)
            goto not_found;
        else
            die("stat");
    }
    free(spath);
    return true;

not_found:
    free(spath);
    return false;  // Does not exists
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
    char const* path;

    if (argc < 2)
        die("need path");

    path = argv[1];

    if (file_is_exists(path))
        printf("is exists [%s]\n", path);
    else {
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

