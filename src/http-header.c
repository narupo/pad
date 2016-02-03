#include "http-header.h"

enum {
	NMETHOD_NAME = 16,
	NMETHOD_VALUE = 128,
	NHTTP_NAME = 16,
};

struct HttpHeader {
	char method_name[NMETHOD_NAME];
	char method_value[NMETHOD_VALUE];
	char http_name[NHTTP_NAME];
	double http_version;
};

/*******************************
* httpheader private functions *
*******************************/

static int
is_newline(char const* sp) {
	return *sp == '\r' && *(sp+1) == '\n';
}

/****************************
* httpheader delete and new *
****************************/

void
httpheader_delete(HttpHeader* self) {
	if (self) {
		free(self);
	}
}

HttpHeader*
httpheader_new(void) {
	HttpHeader* self = (HttpHeader*) calloc(1, sizeof(HttpHeader));
	if (!self) {
		perror("Failed to construct HttpHeader");
		return NULL;
	}

	return self;
}

/********************
* httpheader getter *
********************/

char const*
httpheader_method_name(HttpHeader const* self) {
	return self->method_name;
}

char const*
httpheader_method_value(HttpHeader const* self) {
	return self->method_value;
}

char const*
httpheader_http_name(HttpHeader const* self) {
	return self->http_name;
}

double
httpheader_http_version(HttpHeader const* self) {
	return self->http_version;
}

/*******************
* httpheader debug *
*******************/

void
httpheader_display(HttpHeader const* self) {
	fprintf(stderr, "method name[%s] value[%s]\n", self->method_name, self->method_value);
	fprintf(stderr, "HTTP name[%s] version[%.1f]\n", self->http_name, self->http_version);
	fflush(stderr);
}

/********************
* httpheader parser *
********************/

HttpHeader*
httpheader_parse_string(HttpHeader* self, char const* src) {
	// Init state
	self->method_name[0] = '\0';
	self->method_value[0] = '\0';
	self->http_name[0] = '\0';
	self->http_version = 0.0;

	// Parse HTTP header
	int m = 0;
	char* dst = self->method_name;
	int ndst = sizeof(self->method_name)-1;
	int di = 0;
	char tmp[512];
	
	// GET / HTTP/1.1\r\n
	for (char const* sp = src; *sp; ++sp) {
		switch (m) {
		case 0: // Method name
			if (*sp == ' ') {
				m = 1;
				dst = self->method_value;
				ndst = sizeof(self->method_value)-1;
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = tolower(*sp);
					dst[di] = '\0';
				}
			}
			break;
		case 1: // Method value
			if (*sp == ' ') {
				m = 2;
				dst = self->http_name;
				ndst = sizeof(self->http_name)-1;
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = *sp;
					dst[di] = '\0';
				}
			}
			break;
		case 2: // HTTP name
			if (*sp == '/') {
				m = 3;
				dst = tmp;
				ndst = sizeof(tmp)-1;
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = tolower(*sp);
					dst[di] = '\0';
				}				
			}
			break;
		case 3: // HTTP version
			if (is_newline(sp)) {
				self->http_version = atof(tmp);
				m = 4;
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = *sp;
					dst[di] = '\0';
				}
			}
			break;
		case 4: // Property name
			if (is_newline(sp)) {
				goto done;
			} else if (*sp == ':') {
				// Do save name
				m = 5;
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = *sp;
					dst[di] = '\0';
				}
			}
			break;
		case 5: // Property value
			if (is_newline(sp)) {
				// Do save value
				m = 4; // Loop
				di = 0;
			} else {
				if (di < ndst) {
					dst[di++] = *sp;
					dst[di] = '\0';
				}
			}
			break;
		}
	}

done:
	return self;
}

#if defined(TEST_HTTPHEADER)
int
main(int argc, char* argv[]) {
	HttpHeader* header = httpheader_new();

	httpheader_parse_string(header,
		"GET / HTTP/1.1\r\n"
		"Host: hoge hige\r\n"
		"\r\n"
	);

	httpheader_display(header);

	httpheader_delete(header);
    return 0;
}
#endif
