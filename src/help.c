#include "help.h"

typedef void (*Usage)(void);

static char const PROGNAME[] = "cap help";

static const struct Command {
	char const* name;
	Usage usage;
} commands[] = {
	{"ls", ls_usage},
	{"rm", rm_usage},
	{"cd", cd_usage},
	{"cat", cat_usage},
	{"run", run_usage},
	{"pwd", pwd_usage},
	{"help", help_usage},
	{"home", home_usage},
	{"make", make_usage},
	{"path", path_usage},
	{"edit", edit_usage},
	{"alias", alias_usage},
	{"mkdir", mkdir_usage},
	{"brief", brief_usage},
	{"editor", editor_usage},
	{"deploy", deploy_usage},
	{"server", server_usage},
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
		"Cap is simple snippet manager for the programmer.\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap command [arguments]\n"
		"\n"
		"The commands are:\n"
		"\n"
		"\thelp   display usage\n"
		"\thome   display or set current home directory path\n"
		"\tpwd    display current working directory\n"
		"\tcd     display or set current directory path\n"
		"\tls     display cap file list\n"
		"\tcat    display cap file\n"
		"\tbrief  display brief of file\n"
		"\tpath   display normalized path of cap file\n"
		"\tmake   display and make by cap's make roule\n"
		"\tdeploy deploy files from directory\n"
		"\tedit   edit cap file\n"
		"\teditor display or set editor path\n"
		"\trm     remove cap file or directory\n"
		"\tmkdir  make directory\n"
		"\trun    run script\n"
		"\talias  add alias of cap's command\n"
		"\tserver run cap server\n"
		"\n"
	);
}
