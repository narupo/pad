#include "hub.h"

/**********
* Command *
**********/

typedef struct Command Command;

struct Command {
	int argc; // Like a main function arguments
	char** argv; // "
	int optind; // Save getopt's optind

	bool opt_is_usage;
};

static const char PROGNAME[] = "cap hub";

static bool
hub_parse_options(Command* self);

static void
hub_delete(Command* self);

static bool
hub_parse_options(Command* self);

static void
hub_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
hub_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "hub");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse options
	if (!hub_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
hub_parse_options(Command* self) {
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
			self->opt_is_usage = true;
			break;
		case '?':
		default:
			caperr(PROGNAME, CAPERR_INVALID, "option");
			return false;
			break;
		}
	}

	// Check result of parse options
	if (self->argc < optind) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		return false;
	}

	self->optind = optind; // Copy value of global variable in getopt

	// Done
	return true;
}

static int
hub_run(Command* self) {
	int ret = 0;

	if (self->opt_is_usage) {
		hub_usage();
		return ret;
	}

	term_eprintf("hub.\n");
	return ret;
}

/*******************
* Public Interface *
*******************/

void
hub_usage(void) {
	term_eprintf(
		"cap hub\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap hub [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help \n"
		"\n"
	);
}

int
hub_main(int argc, char* argv[]) {
	// Construct
	Command* hub = hub_new(argc, argv);
	if (!hub) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "hub");
	}

	// Run
	int res = hub_run(hub);

	// Done
	hub_delete(hub);
	return res;	
}

/*******
* Test *
*******/

#if defined(_TEST_HUB)
int
main(int argc, char* argv[]) {
	return hub_main(argc, argv);
}
#endif
