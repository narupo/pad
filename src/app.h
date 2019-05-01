#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#include "lib/memory.h"
#include "lib/cl.h"

#include "modules/constant.h"
#include "modules/config.h"
#include "modules/util.h"
#include "modules/alias_manager.h"

#include "modules/commands/home.h"
#include "modules/commands/cd.h"
#include "modules/commands/pwd.h"
#include "modules/commands/ls.h"
#include "modules/commands/cat.h"
#include "modules/commands/run.h"
#include "modules/commands/alias.h"
#include "modules/commands/edit.h"
#include "modules/commands/editor.h"
#include "modules/commands/mkdir.h"
#include "modules/commands/rm.h"
#include "modules/commands/mv.h"
#include "modules/commands/cp.h"
#include "modules/commands/touch.h"
#include "modules/commands/snippet.h"
