#include "io.h"

typedef enum {
	IO_CSTRING,
	IO_STRING,
	IO_BUFFER,
} Type;

typedef struct {
	Type type;
	union {
		struct {
			char* buf;
			const size_t bufsize;
			size_t i;
		} cstr;
		String* str;
		Buffer* buf;
	};
} Object;

void*
io_getline(Object* obj, FILE* fin) {
	// Check arguments
	switch (obj->type) {
	case IO_CSTRING:
		if (!obj || !fin || !obj->cstr.buf || obj->cstr.bufsize == 0) {
			WARN("Invalid arguments");
			return NULL;
		}
		break;
	case IO_STRING:
		if (!obj || !fin || !obj->str) {
			WARN("Invalid arguments");
			return NULL;
		}
		break;
	case IO_BUFFER:
		if (!obj || !fin || !obj->buf) {
			WARN("Invalid arguments");
			return NULL;
		}
		break;
	default: WARN("Invalid type"); return NULL; break;
	}

	// Init state
	switch (obj->type) {
	case IO_CSTRING:
		obj->cstr.buf[0] = '\0';
		obj->cstr.i = 0;
		break;
	case IO_STRING: str_clear(obj->str); break;
	case IO_BUFFER: buf_clear(obj->buf); break;
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
		case IO_CSTRING:
			if (obj->cstr.i >= obj->cstr.bufsize-1) {
				goto done;
			}
			obj->cstr.buf[obj->cstr.i++] = ch;
			break;
		case IO_STRING: str_push_back(obj->str, ch); break;
		case IO_BUFFER: buf_push_back(obj->buf, ch); break;
		}
	}

done:
	switch (obj->type) {
	case IO_CSTRING:
		obj->cstr.buf[obj->cstr.i] = '\0';
		return (void*) obj->cstr.buf;
		break;
	case IO_STRING: return (void*) obj->str; break;
	case IO_BUFFER: return (void*) obj->buf; break;
	default: return NULL; break;
	}
}

Buffer*
io_getline_buf(Buffer* buf, FILE* fin) {
	Object obj = {
		.type = IO_BUFFER,
		.buf = buf,
	};
	return (Buffer*) io_getline(&obj, fin);
}

String*
io_getline_str(String* str, FILE* fin) {
	Object obj = {
		.type = IO_STRING,
		.str = str,
	};
	return (String*) io_getline(&obj, fin);
}

char*
io_getline_cstr(char* dst, size_t dstsize, FILE* fin) {
	Object obj = {
		.type = IO_CSTRING,
		.cstr = {
			.buf = dst,
			.bufsize = dstsize,
			.i = 0,
		}
	};
	return (char*) io_getline(&obj, fin);
}
