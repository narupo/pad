#include <pad/lib/path.h>

char *
PadPath_PopBackOf(char *path, int32_t ch) {
    if (!path) {
        return NULL;
    }

    size_t len = strlen(path);
    if (!len) {
        return path;
    }

    if (path[len-1] == ch) {
        path[len-1] = '\0';
    }

    return path;
}

char *
PadPath_PopTailSlash(char *path) {
    if (!path) {
        return NULL;
    }

    size_t pathlen = strlen(path);
    if (pathlen <= 0) {
        return path;
    }

#if PAD_PATH__WINDOWS
    int32_t sep = '\\';
#else
    int32_t sep = '/';
#endif

    bool has_slash = path[pathlen-1] == sep;
    if (has_slash) {
        path[pathlen-1] = '\0';
    }

    return path;
}
