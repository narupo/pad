#include "shell.h"

char*
shell_read_script_line(char* dst, size_t dstsize, FILE* stream) {
	if (!dst || feof(stream) || ferror(stream)) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Read line for parse and runtime script name
	char line[dstsize];
	int tell = ftell(stream);

	if (!io_getline_cstr(line, dstsize, stream)) {
		WARN("Failed to read line");
		return NULL;
	}

	// Check prefix
	const char* pref = "#!";
	size_t preflen = strlen(pref);

	if (strncmp(line, pref, preflen) != 0) {
		fseek(stream, tell, SEEK_SET);
		return NULL; // Not found
	}

	// Parse script name
	char* src = line + preflen;

#if defined(_CAP_WINDOWS)
	char* p = strrchr(src, '/');
	if (p) {
		p += 1; // +1 for '/'
	} else {
		p = src;
	}

	snprintf(dst, dstsize, "%s", p);

#else
	snprintf(dst, dstsize, "%s", src);

#endif

	return dst;
}
