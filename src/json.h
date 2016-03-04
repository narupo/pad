#ifndef JSON_H
#define JSON_H

#include "memory.h"
#include "string.h"
#include "strarray.h"
#include <stdbool.h>

typedef struct JsonObject JsonObject;
typedef struct Json Json;
typedef struct JsonIter JsonIter;

/*************
* JsonObject *
*************/

typedef enum {
	JOTDict = 0,
	JOTList,
	JOTValue,
} JsonObjectType;

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonObjectType 
jsonobj_type_const(JsonObject const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonObject const* 
jsonobj_parent_const(JsonObject const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
StringArray* 
jsonobj_list(JsonObject* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
String* 
jsonobj_value(JsonObject* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
String const* 
jsonobj_name_const(JsonObject const* self);

/**
 * @brief 
 *
 * @param self 
*/
void 
jsonobj_delete(JsonObject* self);

/**
 * @brief 
 *
 * @param type   
 * @param parent 
 * @param name   
 *
 * @return 
*/
JsonObject* 
jsonobj_new_with(JsonObjectType type, JsonObject const* parent, char const* name);

/**
 * @brief 
 *
 * @param self 
 * @param list 
*/
void 
jsonobj_move_list(JsonObject* self, StringArray* list);

/**
 * @brief 
 *
 * @param self  
 * @param value 
*/
void 
jsonobj_set_value(JsonObject* self, char const* value);

/**
 * @brief 
 *
 * @param self    
 * @param newcapa 
*/
void 
jsonobj_resize(JsonObject* self, size_t newcapa);

/**
 * @brief 
 *
 * @param self  
 * @param other 
*/
void 
jsonobj_move_back(JsonObject* self, JsonObject* other);

/**
 * @brief 
 *
 * @param self  
 * @param depth 
*/
void 
jsonobj_display(JsonObject const* self, int depth);

/**
 * @brief 
 *
 * @param self 
 * @param name 
 *
 * @return 
*/
StringArray* 
jsonobj_find_list(JsonObject* self, char const* name);

/**
 * @brief 
 *
 * @param self 
 * @param name 
 *
 * @return 
*/
String* 
jsonobj_find_value(JsonObject* self, char const* name);

/**
 * @brief 
 *
 * @param self 
 * @param name 
 *
 * @return 
*/
String const* 
jsonobj_find_value_const(JsonObject const* self, char const* name);

/**
 * @brief 
 *
 * @param self 
 * @param name 
 *
 * @return 
*/
JsonObject* 
jsonobj_find_dict(JsonObject* self, char const* name);

/**
 * @brief 
 *
 * @param self 
 * @param name 
 *
 * @return 
*/
JsonObject const* 
jsonobj_find_dict_const(JsonObject const* self, char const* name);

/**
 * @brief 
 *
 * @param self 
*/
void 
json_delete(Json* self);

/**
 * @brief 
 *
 * @param void 
 *
 * @return 
*/
Json* 
json_new(void);

/**
 * @brief 
 *
 * @param self 
 * @param src  
 *
 * @return 
*/
bool 
json_parse_string(Json* self, char const* src);

JsonObject*
json_root(Json* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonObject const* 
json_root_const(Json const* self);

/**
 * @brief 
 *
 * @param self 
*/
void 
json_display(Json* self);

/**
 * @brief 
 *
 * @param self 
 * @param fout 
 *
 * @return 
*/
bool 
jsonobj_write_to_stream(JsonObject const* self, FILE* fout);

/**
 * @brief 
 *
 * @param self 
 * @param fin  
 *
 * @return 
*/
bool 
json_read_from_stream(Json* self, FILE* fin);

/**
 * @brief 
 *
 * @param self  
 * @param fname 
 *
 * @return 
*/
bool 
json_read_from_file(Json* self, char const* fname);

/**
 * @brief 
 *
 * @param self 
 * @param fout 
 *
 * @return 
*/
bool 
json_write_to_stream(Json const* self, FILE* fout);

/**
 * @brief 
 *
 * @param self  
 * @param fname 
 *
 * @return 
*/
bool 
json_write_to_file(Json const* self, char const* fname);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonIter 
jsonobj_begin(JsonObject* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonIter 
jsonobj_end(JsonObject* self);

struct JsonIter {
	JsonObject** beg;
	JsonObject** end;
	JsonObject** cur;
};

/**
 * @brief 
 *
 * @param lh 
 * @param rh 
 *
 * @return 
*/
bool 
jsoniter_equals(JsonIter const* lh, JsonIter const* rh);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
JsonObject* 
jsoniter_value(JsonIter* self);

JsonIter*
jsoniter_next(JsonIter* self);

#endif
