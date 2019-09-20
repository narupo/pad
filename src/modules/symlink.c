#include "modules/symlink.h"

static const char *symlink_header = "cap symlink:";

static const char *
skip_drive_letter(const char *path) {
    const char *found = strchr(path, ':');
    return ++found;
}

static bool
is_contain_header(char *data, uint32_t datasz) {
    return memcmp(data, symlink_header, strlen(symlink_header)) == 0;
}

static const char *
read_sympath(char *sympath, uint32_t sympathsz, const char *path) {
    FILE *fin = fopen(path, "rb");
    if (!fin) {
        return NULL;
    }

    char data[FILE_NPATH + 100 + 1] = {0};
    uint32_t datasz = fread(data, sizeof(char), sizeof(data)-1, fin);
    if (fclose(fin) < 0) {
        return NULL;
    }
    data[sizeof(data)-1] = '\0'; // for safety

    if (!is_contain_header(data, datasz)) {
        return NULL;
    }

    const char *p = strchr(data, ':');
    if (!p) {
        return NULL;
    }
    ++p;
    datasz -= strlen(symlink_header);
    for (; *p == ' '; ++p) {
        --datasz;
    }

    for (int i = 0; i < sympathsz-1 && *p; ++i, ++p) {
        sympath[i] = *p;
        sympath[i+1] = '\0';
    }

    return sympath;
}

void
fix_path_seps(char *dst, uint32_t dstsz, const char *src) {
#ifdef _CAP_WINDOWS
    char replace_sep = '/';
#else
    char replace_sep = '\\';
#endif
    char *dp = dst;
    char *dpend = dst + dstsz;
    const char *p = src;
    for (; *p && dp < dpend; ++p, ++dp) {
        char ch = *p;
        if (ch == replace_sep) {
            ch = FILE_SEP;
        }
        *dp = ch;
    }
    *dp = '\0';
}

static char *
__symlink_follow_path(char *dst, uint32_t dstsz, const char *abspath, int dep) {
    if (dep >= 8) {
        return NULL;
    }

    char normpath[FILE_NPATH];
    fix_path_seps(normpath, sizeof normpath, abspath);

#ifdef _CAP_WINDOWS
    const char *p = skip_drive_letter(normpath);
    if (!p) {
        return NULL;
    }
#else
    const char *p = normpath;
#endif

    char **toks = cstr_split_ignore_empty(p, FILE_SEP);
    if (!toks) {
        return NULL;
    }

    char **save_toks = NULL;
    char sympath[FILE_NPATH];

    string_t *path = str_new();

    for (char **toksp = toks; *toksp; ++toksp) {
        str_pushb(path, FILE_SEP);
        str_app(path, *toksp);
        // printf("path[%s] toksp[%s]\n", str_getc(path), *toksp);
        if (file_isdir(str_getc(path))) {
            continue;
        }

        sympath[0] = '\0';
        if (!read_sympath(sympath, sizeof sympath, str_getc(path))) {
            continue;
        }
        // printf("sympath[%s]\n", sympath);

        ++toksp;
        save_toks = toksp;
        break;
    }

    if (!save_toks) {
        if (!file_solve(dst, dstsz, normpath)) {
            return NULL;
        }
        goto done;
    }

    str_set(path, sympath);
    for (char **toksp = save_toks; *toksp; ++toksp) {
        str_pushb(path, FILE_SEP);
        str_app(path, *toksp);
    }

    if (!__symlink_follow_path(dst, dstsz, str_getc(path), dep+1)) {
        goto fail;
    }

done:
    for (char **toksp = toks; *toksp; ++toksp) {
        free(*toksp);
    }
    free(toks);
    str_del(path);
    return dst;
fail:
    for (char **toksp = toks; *toksp; ++toksp) {
        free(*toksp);
    }
    free(toks);
    str_del(path);
    return NULL;
}

char *
symlink_follow_path(char *dst, uint32_t dstsz, const char *abspath) {
    if (!dst || !dstsz || !abspath) {
        return NULL;
    }
    dst[0] = '\0';
    return __symlink_follow_path(dst, dstsz, abspath, 0);
}

