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
	{"hub", hub_usage},
	{0},
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
	term_eprintf("Cap is simple snippet manager for the programmer.\n");
	term_eprintf("\n");
	term_eprintf("Usage:\n");
	term_eprintf("\n");
	term_eprintf("\tcap command [arguments]\n");
	term_eprintf("\n");
	term_eprintf("The commands are:\n");
	term_eprintf("\n");
	term_eprintf("\thelp   display usage\n");
	term_eprintf("\thome   display or set current home directory path\n");
	term_eprintf("\tpwd    display current working directory\n");
	term_eprintf("\tcd     display or set current directory path\n");
	term_eprintf("\tls     display cap file list\n");
	term_eprintf("\tcat    display cap file\n");
	term_eprintf("\tbrief  display brief of file\n");
	term_eprintf("\tpath   display normalized path of cap file\n");
	term_eprintf("\tmake   display and make by cap's make roule\n");
	term_eprintf("\tdeploy deploy files from directory\n");
	term_eprintf("\tedit   edit cap file\n");
	term_eprintf("\teditor display or set editor path\n");
	term_eprintf("\trm     remove cap file or directory\n");
	term_eprintf("\tmkdir  make directory\n");
	term_eprintf("\trun    run script\n");
	term_eprintf("\talias  add alias of cap's command\n");
	term_eprintf("\tserver run cap server\n");
	term_eprintf("\thub    hub\n");
	term_eprintf("\n");
}
