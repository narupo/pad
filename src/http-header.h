#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct HttpHeader HttpHeader;

/**
 * @brief 
 *
 * @param self 
*/
void 
httpheader_delete(HttpHeader* self);

/**
 * @brief 
 *
 * @param void 
 *
 * @return 
*/
HttpHeader* 
httpheader_new(void);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
char const* 
httpheader_method_name(HttpHeader const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
char const* 
httpheader_method_value(HttpHeader const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
char const* 
httpheader_http_name(HttpHeader const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
double 
httpheader_http_version(HttpHeader const* self);

/**
 * @brief 
 *
 * @param self 
*/
void 
httpheader_display(HttpHeader const* self);

/**
 * @brief 
 *
 * @param self 
 * @param src  
 *
 * @return 
*/
HttpHeader* 
httpheader_parse_string(HttpHeader* self, char const* src);

#endif