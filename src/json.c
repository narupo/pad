#include "json.h"

typedef struct {
	char const* cur;
	char const* beg;
	char const* end;
} Stream;

static void
stream_delete(Stream* self) {
	if (self) {
		free(self);
	}
}

static Stream*
stream_new(void) {
	Stream* self = (Stream*) mem_ecalloc(1, sizeof(Stream));
	return self;
}

static void
stream_init(Stream* self, char const* src) {
	self->cur = src;
	self->beg = src;
	self->end = src + strlen(src) + 1; // +1 for final nul
}

static int
stream_eof(Stream const* self) {
	return self->cur < self->beg || self->cur >= self->end;
}

static int
stream_current(Stream const* self) {
	return (int) *self->cur;
}

static int
stream_current_at(Stream const* self, int ofs) {
	char const* at = self->cur + ofs;
	if (at < self->end && at >= self->beg) {
		return *at;
	} else {
		return EOF;
	}
}

static int
stream_get(Stream* self) {
	if (self->cur < self->end) {
		int ret = (int) *self->cur;
		++self->cur;
		return ret;
	} else {
		return EOF;
	}
}

static void
stream_next(Stream* self) {
	if (self->cur < self->end) {
		++self->cur;
	}
}

static void
stream_prev(Stream* self) {
	if (self->cur > self->beg) {
		--self->cur;
	}
}

/*************
* JsonObject *
*************/

enum {
	NBRACE_NAME = 32,
	NINIT_BRACES_CAPACITY = 4,
};

typedef struct JsonObject JsonObject;

struct JsonObject {
	JsonObjectType type;
	JsonObject const* parent;
	StringArray* list;
	String* value;
	JsonObject** jsonobjs;
	int jsonobjs_length;
	int jsonobjs_capacity;
	char name[NBRACE_NAME];
};

void
jsonobj_delete(JsonObject* self) {
	if (self) {
		for (int i = 0; i < self->jsonobjs_length; ++i) {
			jsonobj_delete(self->jsonobjs[i]);
		}
		strarray_delete(self->list);
		str_delete(self->value);
		free(self);
	}
}

JsonObject*
jsonobj_new_with(
	JsonObjectType type
	, JsonObject const* parent
	, char const* name) {

	JsonObject* self = (JsonObject*) mem_ecalloc(1, sizeof(JsonObject));

	self->type = type;
	self->parent = parent;
	snprintf(self->name, sizeof self->name, "%s", name);

	self->jsonobjs_length = 0;
	self->jsonobjs_capacity = NINIT_BRACES_CAPACITY;
	self->jsonobjs = (JsonObject**) mem_ecalloc(NINIT_BRACES_CAPACITY+1, sizeof(JsonObject*)); // +1 final nul

	return self;
}

void
jsonobj_move_list(JsonObject* self, StringArray* list) {
	if (self->list) {
		strarray_delete(self->list);
	}
	
	self->list = list;
}

void
jsonobj_set_value(JsonObject* self, char const* value) {
	if (self->value) {
		str_delete(self->value);
	}

	self->value = str_new_from_string(value);
}

void
jsonobj_resize(JsonObject* self, size_t newcapa) {
	JsonObject** tmp = (JsonObject**) mem_erealloc(self->jsonobjs, newcapa * sizeof(JsonObject*) + sizeof(JsonObject*));
	self->jsonobjs = tmp;
	self->jsonobjs_capacity = newcapa;
}

void
jsonobj_move_back(JsonObject* self, JsonObject* other) {
	if (self->jsonobjs_length >= self->jsonobjs_capacity) {
		jsonobj_resize(self, self->jsonobjs_capacity*2);
	}

	self->jsonobjs[self->jsonobjs_length++] = other;
	self->jsonobjs[self->jsonobjs_length] = NULL;
}

static int
is_quote(int ch) {
	return ch == '\'' || ch == '"';
}

void
jsonobj_prase_list(JsonObject* self, Stream* s) {
	if (self->list) {
		strarray_clear(self->list);
	} else {
		self->list = strarray_new();
	}

	String* tok = str_new();
	int m = 0;

	for (; !stream_eof(s); ) {
		int ch = stream_get(s);

		switch (m) {
		case -1:
			return;
			break;
		case 0: // Out of list
			if (ch == '[') {
				m = 1;
			}
			break;
		case 1: // In of list
			if (is_quote(ch)) {
				m = 2;
			} else if (isdigit(ch)) {
				str_push_back(tok, ch);
				m = 3;
			}
			break;
		case 2: // Token with quote
			if (is_quote(ch)) {
				strarray_push_copy(self->list, str_get_const(tok));
				m = 4;
			} else {
				str_push_back(tok, ch);
			}
			break;
		case 3: // Token with numbers
			if (!isdigit(ch)) {
				strarray_push_copy(self->list, str_get_const(tok));
				m = 4;
			} else {
				str_push_back(tok, ch);
			}
			break;
		case 4: // Waiting
			if (ch == ',') {
				str_clear(tok);
				m = 1;
			} else if (ch == ']') {
				goto done;
			} else if (is_quote(ch)) {
				str_clear(tok);
				m = 2;
			} else if (isdigit(ch)) {
				str_clear(tok);
				str_push_back(tok, ch);
				m = 3;
			} else if (isspace(ch)) {
				;
			} else {
				WARN("Syntax error");
				m = -1;
			}
			break;
		}
	}

done:
	str_delete(tok);
}

void
jsonobj_prase_value(JsonObject* self, Stream* s) {
	if (self->value) {
		str_clear(self->value);
	} else {
		self->value = str_new();
	}

	int m = 0;

	for (; !stream_eof(s); ) {
		int ch = stream_get(s);

		switch (m) {
		case 0:
			if (is_quote(ch)) {
				m = 1;
			} else if (isdigit(ch)) {
				str_push_back(self->value, ch);
				m = 2;
			}
			break;
		case 1:
			if (is_quote(ch)) {
				goto done;
			} else {
				str_push_back(self->value, ch);
			}
			break;
		case 2:
			if (!isdigit(ch)) {
				stream_prev(s);
				goto done;
			} else {
				str_push_back(self->value, ch);
			}
			break;
		}
	}

done:
	;
}

void
jsonobj_parse_dict(JsonObject* self, Stream* s) {
	String* tmpname = str_new();
	int m = 0;

	for (; !stream_eof(s); ) {
		int ch = stream_get(s);

		// fprintf(stderr, "mode[%d] ch[%c]\n", m, ch);

		switch (m) {
		case -1: // Syntax error
			return;
			break;
		case 0: // Out of dict
			if (ch == '{') {
				m = 1;
			}
			break;
		case 1: // In of dict
			if (ch == '}') {
				goto done; // End of parse
			} else if (is_quote(ch)) {
				m = 2;
			}
			break;
		case 2: // Begin name
			if (is_quote(ch)) {
				m = 3;
			} else {
				str_push_back(tmpname, ch);
			}
			break;
		case 3: // End name
			if (ch == ':') {
				m = 4;
			} else {
				WARN("Syntax error");
				m = -1;
			}
			break;
		case 4: // :
			if (ch == '{') {
				// Parse dict
				if (str_empty(tmpname)) {
					WARN("Syntax error: need name");
					m = -1;
				} else {
					stream_prev(s);
					JsonObject* obj = jsonobj_new_with(JOTDict, self, str_get_const(tmpname));
					jsonobj_parse_dict(obj, s);
					jsonobj_move_back(self, obj);
					m = 5;
				}
			} else if (ch == '[') {
				// Parse list
				if (str_empty(tmpname)) {
					WARN("Syntax error: need name");
					m = -1;
				} else {
					stream_prev(s);
					JsonObject* obj = jsonobj_new_with(JOTList, self, str_get_const(tmpname));
					jsonobj_prase_list(obj, s);
					jsonobj_move_back(self, obj);
					m = 5;
				}
			} else if (is_quote(ch) || isdigit(ch)) {
				// Parse value with quotes
				if (str_empty(tmpname)) {
					WARN("Syntax error: need name");
					m = -1;
				} else {
					stream_prev(s);
					JsonObject* obj = jsonobj_new_with(JOTValue, self, str_get_const(tmpname));
					jsonobj_prase_value(obj, s);
					jsonobj_move_back(self, obj);
					m = 5;
				}
			}
			break;
		case 5: // Waiting
			if (ch == ',') {
				m = 1;
				str_clear(tmpname);
			} else if (ch == '}') {
				// End of parse
				goto done;
			} else if (isspace(ch)) {
				;
			} else {
				WARN("Syntax error");
				m = -1;
			}
			break;
		}
	}

done:
	str_delete(tmpname);
}


void
jsonobj_display(JsonObject const* self, int depth) {
	if (!self) {
		return;
	}

	for (int i = 0; i < depth; ++i) {
		fputc('\t', stderr);
	}

	fprintf(stderr, "name[%s]", self->name);

	switch (self->type) {
	case JOTDict:
		fprintf(stderr, " type[dict]");		
		break;
	case JOTList:
		if (!self->list) {
			break;
		}

		fprintf(stderr, " type[list]");
		for (int i = 0; i < strarray_length(self->list); ++i) {
			fprintf(stderr, " [%s]", strarray_get_const(self->list, i));
		}
		
		break;
	case JOTValue:
		if (!self->value) {
			break;
		}
		fprintf(stderr, " type[value] [%s]", str_get_const(self->value));
		break;
	default: break;
	}

	fputc('\n', stderr);

	for (int i = 0; i < self->jsonobjs_length; ++i) {
		jsonobj_display(self->jsonobjs[i], depth+1);
	}
}

StringArray*
jsonobj_find_list(JsonObject* self, char const* name) {
	if (strcmp(self->name, name) == 0) {
		return self->list;
	}

	for (int i = 0; i < self->jsonobjs_length; ++i) {
		StringArray* found = jsonobj_find_list(self->jsonobjs[i], name);
		if (found) {
			return found;
		}
	}

	return NULL; // Not found
}

String*
jsonobj_find_value(JsonObject* self, char const* name) {
	if (strcmp(self->name, name) == 0) {
		return self->value;
	}

	for (int i = 0; i < self->jsonobjs_length; ++i) {
		String* found = jsonobj_find_value(self->jsonobjs[i], name);
		if (found) {
			return found;
		}
	}

	return NULL; // Not found
}

JsonObject*
jsonobj_find_dict(JsonObject* self, char const* name) {
	if (strcmp(self->name, name) == 0) {
		return self;
	}

	for (int i = 0; i < self->jsonobjs_length; ++i) {
		JsonObject* found = jsonobj_find_dict(self->jsonobjs[i], name);
		if (found) {
			return found;
		}
	}

	return NULL; // Not found
}

/*******
* Json *
*******/

typedef struct Json Json;

struct Json {
	JsonObject* root;
	Stream* stream; 
	String* buffer;
	void (*mode)(Json*, Stream*); 
};

static void json_mode_first(Json* self, Stream* s);
static void json_mode_quote(Json* self, Stream* s);

void
json_delete(Json* self) {
	if (self) {
		stream_delete(self->stream);
		str_delete(self->buffer);
		jsonobj_delete(self->root);
		free(self);
	}
}

Json*
json_new(void) {
	Json* self = (Json*) mem_ecalloc(1, sizeof(Json));

	self->stream = stream_new();
	self->buffer = str_new();
	
	return self;	
}

static void
json_mode_quote(Json* self, Stream* s) {
	int ch = stream_get(s);

	if (is_quote(ch)) {
		self->mode = json_mode_first;
	} else {
		str_push_back(self->buffer, ch);
	}
}

static void
json_mode_first(Json* self, Stream* s) {
	int ch = stream_get(s);
	
	if (is_quote(ch)) {
		self->mode = json_mode_quote;
	} else if (ch == '{') {
		if (str_empty(self->buffer) && !self->root) {
			// stream_prev(s);
			stream_prev(s);

			if (self->root) {
				jsonobj_delete(self->root);
			}

			self->root = jsonobj_new_with(JOTDict, NULL, "ROOT");
			jsonobj_parse_dict(self->root, s);

		} else {
			WARN("Syntax error");
		}
	}
}

void
json_parse_string(Json* self, char const* src) {
	// Ready state for parse
	self->mode = json_mode_first;
	stream_init(self->stream, src);

	// Check stream
	if (stream_eof(self->stream)) {
		return;
	}

	// Run parser
	for (; !stream_eof(self->stream); ) {
		self->mode(self, self->stream);
	}
}

JsonObject*
json_root(Json* self) {
	return self->root;
}

void
json_display(Json* self) {
	jsonobj_display(self->root, 0);
}

static void
fpad(FILE* fout, int pad, int depth) {
	for (int i = 0; i < depth; ++i) {
		fputc(pad, fout);
	}	
}

bool
jsonobj_write_to_stream(JsonObject const* self, FILE* fout, int depth, int end) {
	switch (self->type) {
	case JOTDict: { 
		bool isroot = strcasecmp(self->name, "root") == 0;

		if (isroot) {
			fpad(fout, '\t', depth);
			fprintf(fout, "{\n");
		} else {
			fpad(fout, '\t', depth);
			fprintf(fout, "\"%s\": {\n", self->name);
		}

		for (int i = 0; i < self->jsonobjs_length-1; ++i) {
			if (!jsonobj_write_to_stream(self->jsonobjs[i], fout, depth+1, ',')) {
				return false;
			}
		}

		if (self->jsonobjs_length > 0) {
			if (!jsonobj_write_to_stream(self->jsonobjs[self->jsonobjs_length-1], fout, depth+1, ' ')) {
				return false;
			}			
		}

		fpad(fout, '\t', depth);
		fprintf(fout, "}%c\n", end);

	} break;
	case JOTList: {
		StringArray const* li = self->list;
		int lilen = strarray_length(li);

		fpad(fout, '\t', depth);
		fprintf(fout, "\"%s\": [\n", self->name);
		
		for (int i = 0; i < lilen-1; ++i) {
			fpad(fout, '\t', depth+1);
			fprintf(fout, "\"%s\",\n", strarray_get_const(li, i));
		}

		if (lilen > 0) {
			fpad(fout, '\t', depth+1);
			fprintf(fout, "\"%s\"\n", strarray_get_const(li, lilen-1));
		}

		fpad(fout, '\t', depth);
		fprintf(fout, "]%c\n", end);

	} break;
	case JOTValue: {
		String* val = self->value;

		fpad(fout, '\t', depth);
		fprintf(fout, "\"%s\": \"%s\"%c\n", self->name, str_get_const(val), end);
	} break;
	}

	return true;
}

bool
json_read_from_stream(Json* self, FILE* fin) {
	String* src = str_new();
	str_read_stream(src, fin);

	json_parse_string(self, str_get_const(src));

	str_delete(src);	

	return true;
}

bool
json_read_from_file(Json* self, char const* fname) {
	FILE* fin = fopen(fname, "rb");
	if (!fin) {
		return false;
	}

	if (!json_read_from_stream(self, fin)) {
		fclose(fin);
		return false;
	}

	if (fclose(fin) != 0) {
		return false;
	}
	return true;
}

bool
json_write_to_stream(Json const* self, FILE* fout) {
	return jsonobj_write_to_stream(self->root, fout, 0, ' ');
}

bool
json_write_to_file(Json const* self, char const* fname) {
	FILE* fout = fopen(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}

	if (!json_write_to_stream(self, fout)) {
		fclose(fout);
		WARN("Failed to write to stream \"%s\"", fname);
		return false;
	}

	if (fclose(fout) != 0) {
		WARN("Failed to close file \"%s\"", fname);
		return false;
	}

	return true;
}

#if defined(TEST_JSON)
#include "file.h"
int
test_list(int argc, char* argv[]) {
	Stream* s = stream_new();
	stream_init(s, "[123,223,323 , 423, 'abc' , 'def','523']");

	JsonObject* obj = jsonobj_new_with(JOTList, NULL, "list");
	jsonobj_prase_list(obj, s);
	jsonobj_display(obj, 0);
	jsonobj_delete(obj);

	return 0;
}

int
test_json(int argc, char* argv[]) {
	char const* src =
		"{"
		"	'setting': {"
		"		'number': 1024,"
		"		'home': 'c:/Users/user',"
		"		'list': [ 'abc','def', 123, 'hige'],"
		"		'string': 'Hello, World!',"
		"		'cd': 'c:/Users/user',"
		"	},"
		"	'server': {"
		"		'root': '/',"
		"		'scripts': ["
		"			'python', 'php', 'ruby'"
		"		],"
		"		'dict': {"
		"			'a': 'aaaa',"
		"			'b': 'bbbb',"
		"			'c': 'cccc',"
		"		},"
		"	},"
		"}"
		;

	Json* json = json_new();
	json_parse_string(json, src);

	JsonObject* root = json_root(json);

	StringArray* list = jsonobj_find_list(root, "scripts");
	if (list) {
		for (int i = 0; i < strarray_length(list); ++i) {
			printf("list[%d] = [%s]\n", i, strarray_get_const(list, i));
		}
	}

	String* value = jsonobj_find_value(root, "string");
	if (value) {
		printf("value[%s]\n", str_get_const(value));
	}

	// json_display(json);
	char path[FILE_NPATH];
	file_solve_path(path, sizeof path, "/Temp/json");

	if (!json_write_to_file(json, path)) {
		WARN("Failed to write to file");
	}

	json_delete(json);
	return 0;
}

int
main(int argc, char* argv[]) {
    test_json(argc, argv);
    fflush(stdout);
    fflush(stderr);
}
#endif
