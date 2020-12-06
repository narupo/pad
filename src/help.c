#include "help.h"

typedef void (*Usage)(void);

static char const* PROGNAME = "cap help";

static const struct Command {
	char const* name;
	Usage usage;
} commands[] = {
	{"ls", ls_usage},
	{"cd", cd_usage},
	{"help", help_usage},
	{"cat", cat_usage},
	{"home", home_usage},
	{"edit", edit_usage},
	{"editor", editor_usage},
	{"deploy", deploy_usage},
	{"make", make_usage},
	{"path", path_usage},
	{"run", run_usage},
	{"pwd", pwd_usage},
	{"alias", alias_usage},
	{"brief", brief_usage},
	{NULL, NULL},
};

static Usage
find_command(char const* cmdname) {
	for (const struct Command* i = commands; i->name; ++i) {
		if (strcmp(cmdname, i->name) == 0) {
			return i->usage;
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
		Usage usage = find_command(cmdname);
		if (usage) {
			usage();
			return 0;
		} else {
			return caperr(PROGNAME, CAPERR_NOTFOUND, "command name \"%s\"", cmdname);
		}
	}
	return 0;
}

void
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
		"\thelp   display usage\n"
		"\tls     display cap file list\n"
		"\tcd     display or set current directory path\n"
		"\tcat    display cap file\n"
		"\trun    run script\n"
		"\tpwd    display current directory\n"
		"\thome   display or set home directory path\n"
		"\tmake   display and make by cap's make roule\n"
		"\tedit   edit cap file\n"
		"\tpath   display normalized path of cap file\n"
		"\talias  add alias of cap's command\n"
		"\tbrief  display brief of file\n"
		"\teditor display or set editor path\n"
		"\tdeploy deploy files from directory\n"
		"\n"
	);
}
