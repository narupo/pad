#ifndef STRING_H
#define STRING_H

#include "util.h"
#include "memory.h"
#include <limits.h>

#define String_type char
typedef struct String String;

/**
 * @brief 
 *
 * @param self 
*/
void 
str_delete(String* self);

/**
 * @brief 
 *
 * @param void 
 *
 * @return 
*/
String* 
str_new(void);

/**
 * @brief 
 *
 * @param str 
 *
 * @return 
*/
String* 
str_new_from_string(String_type const* str);

/**
 * @brief 
 *
 * @param capacity 
 *
 * @return 
*/
String* 
str_new_from_capacity(int capacity);

/**
 * @brief 
 *
 * @param other 
 *
 * @return 
*/
String* 
str_new_from_other(String const* other);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
int 
str_length(String const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
int 
str_capacity(String const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
String_type const* 
str_get_const(String const* self);

/**
 * @brief      
 *
 * @param self
 *
 * @return 
 */
int
str_empty(String const* self);

/**
 * @brief 
 *
 * @param self 
*/
void 
str_clear(String* self);

void
str_set_string(String* self, char const* src);

/**
 * @brief 
 *
 * @param self   
 * @param newlen 
*/
void 
str_resize(String* self, int newlen);

/**
 * @brief 
 *
 * @param self 
 * @param ch   
*/
void 
str_push_back(String* self, String_type ch);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
String_type 
str_pop_back(String* self);

/**
 * @brief 
 *
 * @param self 
 * @param ch   
*/
void 
str_push_front(String* self, String_type ch);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
String_type 
str_pop_front(String* self);

/**
 * @brief 
 *
 * @param self 
 * @param src  
*/
int 
str_append_string(String* self, char const* src);

/**
 * @brief      
 *
 * @param      self
 * @param      fin
 *
 * @return     
 */
int
str_append_stream(String* self, FILE* fin);

/**
 * @brief 
 *
 * @param self  
 * @param other 
*/
int 
str_append_other(String* self, String const* other);

/**
 * @brief 
 *
 * @param self 
 * @param rems 
*/
void 
str_rstrip(String* self, char const* rems);

/**
 * @brief 
 *
 * @param self 
 * @param rems 
*/
void 
str_lstrip(String* self, char const* rems);

/**
 * @brief 
 *
 * @param self 
 * @param rems 
*/
void 
str_strip(String* self, char const* rems);

/**
 * @brief 
 *
 * @param self 
*/
void 
str_pop_newline(String* self);

/**
 * @brief 
 *
 * @param self   
 * @param target 
 *
 * @return 
*/
char const* 
str_find_const(String const* self, char const* target);

/**
 * @brief 
 *
 * @param self 
*/
void 
str_shuffle(String* self);

/**
 * @brief 
 *
 * @param self 
 * @param fin  
 *
 * @return 
*/
String* 
str_getline(String* self, FILE* fin);

int
str_read_stream(String* self, FILE* fin);

#endif
