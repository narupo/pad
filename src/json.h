#ifndef JSON_H
#define JSON_H

#include "memory.h"
#include "string.h"
#include "strarray.h"
#include <stdbool.h>

/*************
* JsonObject *
*************/

typedef struct JsonObject JsonObject;

typedef enum {
	JOTDict = 0,
	JOTList,
	JOTValue,
} JsonObjectType;

void
jsonobj_delete(JsonObject* self);

JsonObject*
jsonobj_new_with(JsonObjectType type, JsonObject const* parent, char const* name);

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
JsonObject* 
jsonobj_find_dict(JsonObject* self, char const* name);

void
jsonobj_move_back(JsonObject* self, JsonObject* other);

/*******
* Json *
*******/

typedef struct Json Json;

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

JsonObject*
json_root(Json* self);

bool
json_read_from_stream(Json* self, FILE* fin);

bool
json_read_from_file(Json* self, char const* fname);

bool
json_write_to_stream(Json const* self, FILE* fout);

bool
json_write_to_file(Json const* self, char const* fname);

#endif
