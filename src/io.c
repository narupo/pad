#include "io.h"

typedef enum {
	STRING,
	BUFFER,
} Type;

typedef struct {
	Type type;
	union {
		String* str;
		Buffer* buf;
	};
} Object;

void*
io_getline(Object* obj, FILE* fin) {
	// Check arguments
	if (!obj || !fin) {
		return NULL;
	}

	// Init state
	switch (obj->type) {
	case STRING: str_clear(obj->str); break;
	case BUFFER: buf_clear(obj->buf); break;
	default: WARN("Invalid type"); return NULL; break;
	}

	if (feof(fin)) {
		return NULL;
	}

	for (int ch; ; ) {
		ch = fgetc(fin);

		if (ch == EOF || ferror(fin)) {
			return NULL;
		}

		if (ch == '\n') {
			break;
		} else if (ch == '\r') {
			int nc = fgetc(fin);
			if (nc == EOF) {
				return NULL;
			}
			if (nc == '\n') {
				break;
			} else {
				if (ungetc(ch, fin) == EOF) {
					return NULL;
				}
				break;
			}
		}

		switch (obj->type) {
		case STRING: str_push_back(obj->str, ch); break;
		case BUFFER: buf_push_back(obj->buf, ch); break;
		}
	}

	switch (obj->type) {
	case STRING: return (void*) obj->str; break;
	case BUFFER: return (void*) obj->buf; break;
	default: return NULL; break;
	}
}

Buffer*
io_getline_buf(Buffer* buf, FILE* fin) {
	Object obj = {
		.type = BUFFER,
		.buf = buf,
	};
	return (Buffer*) io_getline(&obj, fin);
}

String*
io_getline_str(String* str, FILE* fin) {
	Object obj = {
		.type = STRING,
		.str = str,
	};
	return (String*) io_getline(&obj, fin);
}
