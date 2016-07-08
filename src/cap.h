/*
	{# comment 1 #}
	left{{ var }}right
	{{ CD }}
	{{ HOME }}
	{% if (var) : %}
		{# comment 2 #}
		{{ dog }}
	{% end %}
	{% for (el in lis): %}
		{{ el }}
	{% end %}
*/
#ifndef CAP_H
#define CAP_H

#define _BSD_SOURCE 1 /* In cap.h: For the setenv */
#include <stdio.h>
#include <sys/wait.h>
#include <getopt.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "config.h"
#include "array.h"

#endif

