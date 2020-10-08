#pragma once

#include <assert.h>

#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/dict.h>

struct opts;
typedef struct opts opts_t;

/**
 * destruct opts_t
 * the self of argument is will be free'd
 *
 * @param[in] *self pointer to opts_t
 */
void
opts_del(opts_t *self);

/**
 * construct opts_t
 * allocate memory and init that memory
 * default constructor
 *
 * @return pointer to opts_t (dynamic allocated memory)
 */
opts_t *
opts_new(void);

opts_t *
opts_deep_copy(const opts_t *other);

/**
 * parse arguments and store values at opts_t
 *
 * @param[in] *self   pointer to opts_t
 * @param[in] argc    number of arguments
 * @param[in] *argv[] array of arguments (NULL terminated)
 *
 * @return succes to pointer to the self of argument
 * @return failed to NULL
 */
opts_t *
opts_parse(opts_t *self, int argc, char *argv[]);

/**
 * get element in opts_t by option name
 * the option name will be without '-' like the 'h' or 'help'
 *
 * @param[in] *self    pointer to opts_t
 * @param[in] *optname strings of option name
 *
 * @return found to pointer to option value of string in opts_t
 * @return not found to NULL
 */
const char *
opts_getc(const opts_t *self, const char *optname);

/**
 * if opts_t has option name then return true else return false
 *
 * @param[in] *self    pointer to opts_t
 * @param[in] *optname striongs of option name
 *
 * @return found option name to return true
 * @return not found option name to return false
 */
bool
opts_has(const opts_t *self, const char *optname);

/**
 * get element of arguments
 *
 * @param[in] *self pointer to opts_t
 * @param[in] idx   number of index
 *
 * @return found to return argument value of strings
 * @return not found to NULL
 */
const char *
opts_getc_args(const opts_t *self, int32_t idx);

/**
 * get arguments length
 *
 * @param[in] *self pointer to opts_t
 *
 * @return number of arguments length
 */
int32_t
opts_args_len(const opts_t *self);

/**
 * clear status
 *
 * @param[in] *self
 */
void
opts_clear(opts_t *self);
