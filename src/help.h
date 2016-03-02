#ifndef HELP_H
#define HELP_H

#include "util.h"
#include "term.h"
#include "cat.h"
#include "ls.h"
#include "rm.h"
#include "cd.h"
#include "pwd.h"
#include "home.h"
#include "edit.h"
#include "editor.h"
#include "deploy.h"
#include "make.h"
#include "path.h"
#include "run.h"
#include "alias.h"
#include "mkdir.h"
#include "brief.h"
#include "server.h"

int
help_main(int argc, char* argv[]);

void
help_usage(void);

#endif
