#include "trash.h"

static const char PROGNAME[] = "cap trash";
static const char TRASH_FNAME[] = "trash.json";

typedef struct Program Program;
typedef struct Command Command;

struct Command {
	Program* prog;
	const char* name;
	int (*func)(Command*);
};

struct Program {
	int argc;
	int optind;
	char** argv;
	Command* cmd;
	Json* json;
};

/****************
* Command utils *
****************/

static String*
make_oldpath(const char* name) {
	Config* cn = config_instance();
	char path[FILE_NPATH];
	if (!config_path_with_cd(cn, path, sizeof path, name)) {
		caperr(PROGNAME, CAPERR_READ, "config");
		return NULL;
	}
	return str_new_from_string(path);
}

static String*
make_newpath(const char* name) {
	Config* cn = config_instance();
	char tmp[FILE_NPATH];

	const char* basename = file_basename(tmp, sizeof tmp, name);
	if (!basename) {
		caperr(PROGNAME, CAPERR_MAKE, "basename form \"%s\"", name);
		return NULL;
	}

	char path[FILE_NPATH];
	if (!config_dirpath_with(cn, path, sizeof path, "trash", basename)) {
		caperr(PROGNAME, CAPERR_READ, "config");
		return NULL;
	}

	return str_new_from_string(path);
}

/**********
* Command *
**********/

static int
cmd_help(Command* self) {
	trash_usage();
	return 0;
}

static int
cmd_trash(Command* self) {
	Program* prog = self->prog;

	if (prog->argc == prog->optind) {
		return cmd_help(self);
	}

	for (int i = prog->optind; i < prog->argc; ++i) {
		const char* arg = prog->argv[i];
		String* oldpath = make_oldpath(arg);
		String* newpath = make_newpath(arg);
		if (!oldpath || !newpath) {
			str_delete(oldpath);
			str_delete(newpath);
			return caperr(PROGNAME, CAPERR_CONSTRUCT, "string");
		}

		if (!file_is_exists(str_get_const(oldpath))) {
			caperr_printf(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", arg);
			continue;
		}

		term_eprintf("rename [%s] -> [%s]\n", str_get_const(oldpath), str_get_const(newpath));
		if (file_rename(str_get_const(oldpath), str_get_const(newpath)) != 0) {
			caperr_printf(PROGNAME, CAPERR_RENAME, "%s", arg);
		}



		str_delete(oldpath);
		str_delete(newpath);
	}

	return 0;
}

/******
* SEP *
******/

static bool
prog_parse_options(Program* self);

static void
prog_delete(Program* self) {
	if (self) {
		json_delete(self->json);
		free(self);
	}
}

static Program*
prog_new(int argc, char* argv[]) {
	// Construct
	Program* self = (Program*) calloc(1, sizeof(Program));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse command options
	if (!prog_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		free(self);
		return NULL;
	}

	if (!(self->json = json_new())) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "trash config");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
prog_parse_options(Program* self) {
	static Command cmds[] = {
		{ .name="trash", .func=cmd_trash }, // Keep order
		{ .name="help", .func=cmd_help },
	};

	self->cmd = &cmds[0];
	self->cmd->prog = self;

	// Parse options
	optind = 0;

	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{"undo", no_argument, 0, 'u'},
			{"redo", no_argument, 0, 'r'},
			{"clear", no_argument, 0, 'c'},
			{"history", no_argument, 0, 'H'},
			{},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "hurcH", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->cmd = &cmds[1]; break;
		case 'c': break;
		case 'u': break;
		case 'r': break;
		case 'H': break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		return false;
	}

	// Done
	return true;
}

static int
prog_run(Program* self) {
	Command* cmd = self->cmd;
	return cmd->func(cmd);
}

/*************************
* trash public interface *
*************************/

void
trash_usage(void) {
    term_eprintf(
        "Usage:\n"
        "\n"
        "\t%s [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\t-H, --history display history\n"
        "\t-u, --undo undo\n"
        "\t-r, --redo redo\n"
        "\t-c, --clear clear trash\n"
        "\n"
    , PROGNAME);
}

int
trash_main(int argc, char* argv[]) {
	// Construct
	Program* prog = prog_new(argc, argv);
	if (!prog) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "prog");
	}

	// Run
	int ret = prog_run(prog);

	// Done
	prog_delete(prog);
	return ret;
}

/*************
* trash test *
*************/

#if defined(TEST_TRASH)
int
main(int argc, char* argv[]) {
	return trash_main(argc, argv);
}
#endif
