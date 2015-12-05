#include "help.h"

typedef void (*usage_type)(void);

static struct Command {
	char const* name;
	usage_type usage;
} commands[] = {
	{"cat", cat_usage},
	{"ls", ls_usage},
	{"cd", cd_usage},
	{"edit", edit_usage},
	{"editor", editor_usage},
	{"deploy", deploy_usage},
	{"make", make_usage},
	{"path", path_usage},
	{"run", run_usage},
	{NULL, NULL},
};

static usage_type
find_command(char const* cmdname) {
	for (int i = 0; ; ++i) {
		struct Command* cmd = &commands[i];
		if (!cmd->name) {
			break;
		}

		if (strcmp(cmdname, cmd->name) == 0) {
			return cmd->usage;
		}
	}
	return NULL;
}

int
help_main(int argc, char* argv[]) {
	if (argc < 2) {
		help_usage();	
	} else {
		char const* cmdname = argv[1];
		usage_type usage = find_command(cmdname);
		if (usage) {
			usage();
		} else {
			term_eprintf("Not found command name \"%s\"\n", cmdname);
		}
	}
	return 0;
}

void _Noreturn
help_usage(void) {
	term_eprintf(
		"Cap is simple snippet manager for programmer.\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap command [arguments]\n"
		"\n"
		"The commands are:\n"
		"\n"
		"\thelp\tdisplay usage\n"
		"\tcat\tdisplay cap file\n"
		"\tls\tdisplay cap file list\n"
		"\tcd\tdisplay or set current directory path\n"
		"\tedit\tedit cap file\n"
		"\teditor\tdisplay or set editor path\n"
		"\tdeploy\tdeploy files from directory\n"
		"\tmake\tdisplay and make by cap's make roule\n"
		"\tpath\tdisplay normalized path of cap file\n"
		"\trun\trun script\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}
