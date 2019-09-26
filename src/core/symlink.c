#include "core/symlink.h"

static const char *
skip_drive_letter(const char *path) {
    const char *found = strchr(path, ':');
    if (!found) {
        return NULL;
    }
    return ++found;
}

static bool
is_contain_header(char *data, uint32_t datasz) {
    return memcmp(data, SYMLINK_HEADER, strlen(SYMLINK_HEADER)) == 0;
}

static const char *
read_sympath(config_t *config, char *sympath, uint32_t sympathsz, const char *path) {
    FILE *fin = fopen(path, "r");
    if (!fin) {
        return NULL;
    }

    uint32_t symheaderlen = strlen(SYMLINK_HEADER);
    char line[FILE_NPATH + symheaderlen + 1];
    int32_t linelen = file_getline(line, sizeof line, fin);
    if (linelen == EOF) {
        return NULL;
    }
    if (fclose(fin) < 0) {
        return NULL;
    }

    if (!is_contain_header(line, linelen)) {
        return NULL;
    }

    const char *p = strchr(line, ':');
    if (!p) {
        return NULL;
    }
    ++p;
    for (; *p == ' '; ++p) ;

    char cappath[FILE_NPATH];
    for (int i = 0; i < sizeof(cappath)-1 && *p; ++i, ++p) {
        cappath[i] = *p;
        cappath[i+1] = '\0';
    }

    // origin is home path
    const char *org = config->home_path;
    if (!file_solvefmt(sympath, sympathsz, "%s/%s", org, cappath)) {
        return NULL;
    }

    return sympath;
}

static void
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

static const char *
find_path_head(const char *path) {
#ifdef _CAP_WINDOWS
    const char *p = skip_drive_letter(path);
    if (!p) {
        return path;
    }
    return p;
#else
    return path;
#endif
}

static char *
__symlink_follow_path(config_t *config, char *dst, uint32_t dstsz, const char *abspath, int dep) {
    if (dep >= 8) {
        return NULL;
    }

    char normpath[FILE_NPATH];
    fix_path_seps(normpath, sizeof normpath, abspath);

    const char *p = find_path_head(normpath);
    if (!p) {
        return NULL;
    }

    char **toks = cstr_split_ignore_empty(p, FILE_SEP);
    if (!toks) {
        return NULL;
    }

    char **save_toks = NULL;
    char sympath[FILE_NPATH];

    string_t *path = str_new();
#ifdef _CAP_WINDOWS
    // append drive letter
    str_pushb(path, normpath[0]);
    str_app(path, ":\\");
#endif
    for (char **toksp = toks; *toksp; ++toksp) {
        str_pushb(path, FILE_SEP);
        str_app(path, *toksp);
        // printf("path[%s] toksp[%s]\n", str_getc(path), *toksp);
        if (file_isdir(str_getc(path))) {
            continue;
        }

        sympath[0] = '\0';
        if (!read_sympath(config, sympath, sizeof sympath, str_getc(path))) {
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

    if (!__symlink_follow_path(config, dst, dstsz, str_getc(path), dep+1)) {
        goto fail;
    }

#define cleanup() { \
    for (char **toksp = toks; *toksp; ++toksp) { \
        free(*toksp); \
    } \
    free(toks); \
    str_del(path); \
}
done:
    cleanup();
    return dst;
fail:
    cleanup();
    return NULL;
}

char *
symlink_follow_path(config_t *config, char *dst, uint32_t dstsz, const char *abspath) {
    if (!dst || !dstsz || !abspath) {
        return NULL;
    }
    dst[0] = '\0';
    return __symlink_follow_path(config, dst, dstsz, abspath, 0);
}

static cstring_array_t *
split_ignore_empty(const char *p, char sep) {
    char **toks = cstr_split_ignore_empty(p, FILE_SEP);
    if (!toks) {
        return NULL;
    }

    cstring_array_t *arr = cstrarr_new();

    for (char **toksp = toks; *toksp; ++toksp) {
        cstrarr_move(arr, *toksp);
    }

    free(toks);
    return arr;
}

char *
symlink_norm_path(config_t *config, char *dst, uint32_t dstsz, const char *drtpath) {
    if (!config || !dst || !dstsz || !drtpath) {
        return NULL;
    }

    char cleanpath[FILE_NPATH];
    fix_path_seps(cleanpath, sizeof cleanpath, drtpath);
    const char *pathhead = find_path_head(cleanpath);
    if (!pathhead) {
        return NULL;
    }

#ifdef _CAP_WINDOWS
    const char *hasdriveletter = strchr(drtpath, ':');
#endif

    // save tokens from srctoks to dsttoks by ".." token
    cstring_array_t *srctoks = split_ignore_empty(pathhead, FILE_SEP);
    cstring_array_t *dsttoks = cstrarr_new();

    for (int32_t i = 0; i < cstrarr_len(srctoks); ++i) {
        const char *tok = cstrarr_getc(srctoks, i);
        if (cstr_eq(tok, "..")) {
            char *el = cstrarr_pop_move(dsttoks);
            free(el);
        } else {
            cstrarr_push(dsttoks, tok);
        }
    }

    // save normalized path by tokens
    dst[0] = '\0';

#ifdef _CAP_WINDOWS
    // append drive letter of Windows
    if (hasdriveletter) {
        cstr_appfmt(dst, dstsz, "%c:", drtpath[0]);
    }
#endif

    if (pathhead[0] == FILE_SEP) {
        cstr_appfmt(dst, dstsz, "%c", FILE_SEP);
    }

    for (int32_t i = 0; i < cstrarr_len(dsttoks)-1; ++i) {
        const char *tok = cstrarr_getc(dsttoks, i);
        cstr_app(dst, dstsz, tok);
        cstr_appfmt(dst, dstsz, "%c", FILE_SEP);
    }
    if (cstrarr_len(dsttoks)) {
        const char *tok = cstrarr_getc(dsttoks, cstrarr_len(dsttoks)-1);
        cstr_app(dst, dstsz, tok);        
    }

    cstrarr_del(srctoks);
    cstrarr_del(dsttoks);

    return dst;
}

bool
symlink_is_link_file(const char *path) {
    char line[FILE_NPATH + 100];
    if (!file_readline(line, sizeof line, path)) {
        return false;
    }

    return is_contain_header(line, strlen(line));
}

