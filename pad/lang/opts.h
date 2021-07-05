#pragma once

#include <assert.h>

#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/dict.h>

struct PadOpts;
typedef struct PadOpts PadOpts;

/**
 * destruct PadOpts_t
 * the self of argument is will be free'd
 *
 * @param[in] *self pointer to PadOpts
 */
void
PadOpts_Del(PadOpts *self);

/**
 * construct PadOpts_t
 * allocate memory and init that memory
 * default constructor
 *
 * @return pointer to PadOpts (dynamic allocated memory)
 */
PadOpts *
PadOpts_New(void);

PadOpts *
PadOpts_DeepCopy(const PadOpts *other);

PadOpts *
PadOpts_ShallowCopy(const PadOpts *other);

/**
 * parse arguments and store values at PadOpts
 *
 * @param[in] *self   pointer to PadOpts
 * @param[in] argc    number of arguments
 * @param[in] *argv[] array of arguments (NULL terminated)
 *
 * @return succes to pointer to the self of argument
 * @return failed to NULL
 */
PadOpts *
PadOpts_Parse(PadOpts *self, int argc, char *argv[]);

/**
 * get element in PadOpts by option name
 * the option name will be without '-' like the 'h' or 'help'
 *
 * @param[in] *self    pointer to PadOpts
 * @param[in] *optname strings of option name
 *
 * @return found to pointer to option value of string in PadOpts
 * @return not found to NULL
 */
const char *
PadOpts_Getc(const PadOpts *self, const char *optname);

/**
 * if PadOpts has option name then return true else return false
 *
 * @param[in] *self    pointer to PadOpts
 * @param[in] *optname striongs of option name
 *
 * @return found option name to return true
 * @return not found option name to return false
 */
bool
PadOpts_Has(const PadOpts *self, const char *optname);

/**
 * get element of arguments
 *
 * @param[in] *self pointer to PadOpts
 * @param[in] idx   number of index
 *
 * @return found to return argument value of strings
 * @return not found to NULL
 */
const char *
PadOpts_GetcArgs(const PadOpts *self, int32_t idx);

/**
 * get arguments length
 *
 * @param[in] *self pointer to PadOpts
 *
 * @return number of arguments length
 */
int32_t
PadOpts_ArgsLen(const PadOpts *self);

/**
 * clear status
 *
 * @param[in] *self
 */
void
PadOpts_Clear(PadOpts *self);
