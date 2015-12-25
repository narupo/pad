#include "alias.h"

typedef struct Command Command;

enum {
	ALIAS_NKEY = 32,
	ALIAS_NVAL = 128
};

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Set values
	self->name = "cap alias";
	self->argc = argc;
	self->argv = argv;

	// Parse alias options
	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h':
			command_delete(self);
			exit(EXIT_FAILURE);
			break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static int
hashi(char const* src, int nhash) {
	int n = 0;
	for (char const* p = src; *p; ++p) {
		n += *p;
	}
	return n % nhash;
}

static char*
command_make_path(char* dst, size_t dstsize) {
	Config* config = config_instance();
	char const* cd = config_path(config, "cd");
	char const* root = config_root(config);

	strrem(dst, dstsize, cd, '/');
	snprintf(dst, dstsize, "%s/alias-%d", root, hashi(dst, 701));

	return dst;
}

static void
putcol(FILE* stream, char const* str, int colsize) {
	char buf[colsize];
	memset(buf, 0, NUMOF(buf));
	if (str) {
		memmove(buf, str, strlen(str));
	}
	fwrite(buf, sizeof(buf[0]), colsize, stream);
}

static int
command_push_alias_to_file(char const* path, char const* pushkey, char const* pushval) {
	// Random access file
	FILE* stream = file_open(path, "rb+");
	if (!stream) {
		WARN("Failed to open file \"%s\"", path);
		return 1;
	}

	// Find overlap record by key
	for (; !feof(stream); ) {
		// Read key
		char key[ALIAS_NKEY+1] = {0}; // +1 for final nul

		int len = fread(key, sizeof(key[0]), ALIAS_NKEY, stream);
		if (len <= 0) {
			// End of File
			break;
		}

		if (ferror(stream)) {
			WARN("Failed to read stream");
			goto done;
		}

		key[len] = 0; // Nul terminate for string

		if (strcmp(key, pushkey) == 0) {
			// Found overlap key, Override this record
			fseek(stream, -ALIAS_NKEY, SEEK_CUR); // Back to the basic
			putcol(stream, pushkey, ALIAS_NKEY);
			putcol(stream, pushval, ALIAS_NVAL);
			goto done;
		}

		// Next record
		fseek(stream, ALIAS_NVAL, SEEK_CUR);
	}

	// Not found overlap key, Insert new record
	fseek(stream, 0L, SEEK_END);
	putcol(stream, pushkey, ALIAS_NKEY);
	putcol(stream, pushval, ALIAS_NVAL);

done:
	file_close(stream);
	return 0;
}

static int
getcol(FILE* fin, char* dst, size_t colsize) {
	int len = fread(dst, sizeof(dst[0]), colsize, fin);
	if (len <= 0) {
		return len;
	}
	if (ferror(fin)) {
		return -1;
	}
	dst[len] = 0;
	return len;
}

static int
command_disp_alias_list(Command* self) {
	char path[NFILE_PATH];
	command_make_path(path, NUMOF(path));

	FILE* fin = file_open(path, "rb");
	if (!fin) {
		WARN("Failed to open file \"%s\"", path);
		return 1;
	}

	// First
	// Get max length of key for display
	size_t maxkeylen = 0;
	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];
		
		if (getcol(fin, key, ALIAS_NKEY) <= 0) {
			break;
		}
		fseek(fin, ALIAS_NVAL, SEEK_CUR);

		size_t keylen = strlen(key);
		maxkeylen = (keylen > maxkeylen ? keylen : maxkeylen);
	}

	// Second
	// Display key and value with padding
	rewind(fin);

	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];
		char val[ALIAS_NVAL+1];
		
		if (getcol(fin, key, ALIAS_NKEY) <= 0) {
			break;
		}
		getcol(fin, val, ALIAS_NVAL);

		term_printf("%-*s %s\n", maxkeylen, key, val);
	}

	file_close(fin);
	return 0;
}

static int
command_run(Command* self) {
	// Check arugments
	if (self->argc == self->optind) {
		return command_disp_alias_list(self);
	}

	if (self->argc != self->optind + 2) {
		alias_usage();
		return 0;
	}

	// File works
	char path[NFILE_PATH];
	command_make_path(path, NUMOF(path));

	if (!file_is_exists(path)) {
		if (!file_create(path)) {
			WARN("Failed to create file \"%s\"", path);
			return 2;
		}
	}

	char const* pushkey = self->argv[self->optind];
	char const* pushval = self->argv[self->optind + 1];

	return command_push_alias_to_file(path, pushkey, pushval);
}

CsvLine*
alias_to_csvline(char const* findkey) {
	// Make alias path
	char path[NFILE_PATH];
	command_make_path(path, NUMOF(path));

	// Open stream
	FILE* fin = file_open(path, "rb");
	if (!fin) {
		WARN("Failed to open file \"%s\"", path);
		goto fail_file_open;
	}

	// Parse record
	CsvLine* csvline = csvline_new();

	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];
		memset(key, 0, sizeof key);

		int len = fread(key, sizeof(key[0]), ALIAS_NKEY, fin);
		if (len <= 0) {
			goto notfound;
		}

		if (ferror(fin)) {
			WARN("Failed to read stream for alias key");
			goto notfound;
		}

		key[len] = 0;

		if (strcmp(key, findkey) == 0) {
			// Found key, parse value column
			char val[ALIAS_NVAL+1];
			memset(val, 0, sizeof val);
			int len = fread(val, sizeof(val[0]), ALIAS_NVAL, fin);
			if (len <= 0 || ferror(fin)) {
				WARN("Failed to read stream for alias value");
				goto notfound;
			}

			val[len] = 0;

			csvline_parse_line(csvline, val, ' ');

			// Success to csvline
			file_close(fin);
			return csvline;
		}
	}

notfound:
	csvline_delete(csvline);
	file_close(fin);

fail_file_open:
	return NULL;
}

void
alias_usage(void) {
	term_eprintf(
		"cap alias\n"
	);
}

int
alias_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		WARN("Failed to construct command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

#if defined(TEST_ALIAS)
int
main(int argc, char* argv[]) {
	char const* findkey = "fire";
	if (argc >= 2) {
		findkey = argv[1];
	}
	
	CsvLine* csvline = alias_to_csvline(findkey);

	for (int i = 0; i < csvline_length(csvline); ++i) {
		printf("[%d] = [%s]\n", i, csvline_get(csvline, i));
	}

	csvline_delete(csvline);
	return 0;
}
#endif
