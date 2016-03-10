#ifndef JSON_H
#define JSON_H

#include "define.h"
#include "memory.h"
#include "string.h"
#include "strarray.h"
#include <stdbool.h>

typedef struct JsonObject JsonObject;
typedef struct Json Json;
typedef struct JsonIter JsonIter;

/*************
* JsonObject *
************ */

typedef enum {
	JOTDict = 0,
	JOTList,
	JOTValue,
} JsonObjectType;

/**
 * Get type of JsonObject
 *
 * @param[in] self 
 *
 * @return type of JsonObject
 */
JsonObjectType 
jsonobj_type_const(JsonObject const* self);

/**
 * Get parent of pointer to memory of JsonObject
 *
 * @param[in] self 
 *
 * @return pointer to memory of JsonObject
 */
JsonObject const* 
jsonobj_parent_const(JsonObject const* self);

/**
 * Get list of JsonObject
 *
 * @param[in] self 
 *
 * @return pointer to memory of list as StringArray
 */
StringArray* 
jsonobj_list(JsonObject* self);

/**
 * Get value of JsonObject
 *
 * @param[in] self 
 *
 * @return pointer to memory of value as String
 */
String* 
jsonobj_value(JsonObject* self);

/**
 * Get name of JsonObject
 *
 * @param[in] self 
 *
 * @return pointer to memory of name as String
 */
String const* 
jsonobj_name_const(JsonObject const* self);

/**
 * Destruct JsonObject
 *
 * @param[in] self 
 */
void 
jsonobj_delete(JsonObject* self);

/**
 * Construct JsonObject with parameters
 * If this object is root parent parameter is NULL
 *
 * @param[in] type   type of JsonObject
 * @param[in] parent pointer to memory of parent of JsonObject
 * @param[in] name   pointer to memory of name of C string
 *
 * @return pointer to dynamic allocate memory of JsonObject
 */
JsonObject* 
jsonobj_new_with(JsonObjectType type, JsonObject const* parent, char const* name);

/**
 * Resize array of child objects in JsonObject
 *
 * @param[in] self    
 * @param[in] newcapa resize capacity
 */
void 
jsonobj_resize(JsonObject* self, size_t newcapa);

/**
 * Move other object to back of objects in JsonObject
 *
 * @param[in] self  
 * @param[in] other pointer to memory of other JsonObject
 */
void 
jsonobj_move_back(JsonObject* self, JsonObject* other);

/**
 * Display JsonObject parameters
 *
 * @param[in] self  
 * @param[in] depth current recursive depth
 */
void 
jsonobj_display(JsonObject const* self, int depth);

/**
 * Find list of JsonObject by name of object
 *
 * @param[in] self 
 * @param[in] name pointer to memory of find name of C string
 *
 * @return found to pointer to memory of found list
 * @return not found to NULL
 */
StringArray* 
jsonobj_find_list(JsonObject* self, char const* name);

/**
 * Find value of JsonObject by name of object
 *
 * @param[in] self 
 * @param[in] name pointer to memory of C string
 *
 * @return found to pointer to memory of found value
 * @return not found to NULL
 */
String* 
jsonobj_find_value(JsonObject* self, char const* name);

/**
 * Find read-only value of JsonObject by name of object
 *
 * @param[in] self 
 * @param[in] name pointer to memory of C string
 *
 * @return found to pointer to memory of value of String
 * @return not found to NULL
 */
String const* 
jsonobj_find_value_const(JsonObject const* self, char const* name);

/**
 * Find dictionary of JsonObject by name of object
 *
 * @param[in] self 
 * @param[in] name pointer to memory of name of C string
 *
 * @return found to pointer to memory of name of C string
 * @return not found to NULL
 */
JsonObject* 
jsonobj_find_dict(JsonObject* self, char const* name);

/**
 * Find read-only dictionary of JsonObject by name of object
 *
 * @param[in] self 
 * @param[in] name pointer to memory of name of C string
 *
 * @return found to pointer to memory of name of C string
 * @return not found to NULL
 */
JsonObject const* 
jsonobj_find_dict_const(JsonObject const* self, char const* name);

/**
 * Write JsonObject to stream
 *
 * @param[in] self 
 * @param[in] fout output stream
 *
 * @return success to true
 * @return failed to false
 */
bool 
jsonobj_write_to_stream(JsonObject const* self, FILE* fout);

/*******
* Json *
*******/

/**
 * Destruct Json
 *
 * @param[in] self 
 */
void 
json_delete(Json* self);

/**
 * Construct Json
 *
 * @param[in] void 
 *
 * @return pointer to dynamic allocate memory of Json
 */
Json* 
json_new(void);

/**
 * Parse string for build objects
 *
 * @param[in] self 
 * @param[in] src  source string
 *
 * @return success to true
 * @return failed to false
 */
bool 
json_parse_string(Json* self, char const* src);

/**
 * Get root object of Json
 * 
 * @param self 
 *
 * @return found to pointer to memory of root JsonObject
 * @return not found to NULL
 */
JsonObject*
json_root(Json* self);

/**
 * Get root object of Json
 *
 * @param[in] self 
 *
 * @return pointer to read-only memory of JsonObject
 */
JsonObject const* 
json_root_const(Json const* self);

/**
 * Display Json objects to stderr
 *
 * @param[in] self 
 */
void 
json_display(Json* self);

/**
 * Read from stream for build objects
 *
 * @param[in] self 
 * @param[in] fin  input stream
 *
 * @return success to true
 * @return failed to false
 */
bool 
json_read_from_stream(Json* self, FILE* fin);

/**
 * Read from file for build objects by file name
 *
 * @param[in] self  
 * @param[in] fname input file name
 *
 * @return success to true
 * @return failed to false
 */
bool 
json_read_from_file(Json* self, char const* fname);

/**
 * Write Json to stream
 *
 * @param[in] self 
 * @param[in] fout output stream
 *
 * @return success to true
 * @return failed to false
 */
bool 
json_write_to_stream(Json const* self, FILE* fout);

/**
 * Write Json to file
 *
 * @param[in] self  
 * @param[in] fname output file name
 *
 * @return success to true
 * @return failed to false
 */
bool 
json_write_to_file(Json const* self, char const* fname);

/***********
* JsonIter *
***********/

struct JsonIter {
	JsonObject** beg;
	JsonObject** end;
	JsonObject** cur;
};

/**
 * Get begin of iterator from JsonObject
 *
 * @param[in] self 
 *
 * @return JsonIter
 */
JsonIter 
jsonobj_begin(JsonObject* self);

/**
 * Get end of iterator from JsonObject
 *
 * @param[in] self 
 *
 * @return JsonIter
 */
JsonIter 
jsonobj_end(JsonObject* self);

/**
 * Compare for equals like a "lh == rh"
 *
 * @param[in] lh left-hand iterator
 * @param[in] rh right-hand iterator
 *
 * @return equals to true
 * @return not equals to false
 */
bool 
jsoniter_equals(JsonIter const* lh, JsonIter const* rh);

/**
 * Get value of current from iterator
 *
 * @param[in] self 
 *
 * @return pointer to memory of JsonObject
 */
JsonObject* 
jsoniter_value(JsonIter* self);

/**
 * Get next of iterator from current iterator
 * 
 * @param self 
 *             
 * @return pointer to memory of next iterator
 */
JsonIter*
jsoniter_next(JsonIter* self);

#endif
