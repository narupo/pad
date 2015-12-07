#ifndef ATCAP_H
#define ATCAP_H

#include "util.h"
#include "buffer.h"
#include "strarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
@cap brief This is string of brief.
@cap brief Bra bra bra.
----
@cap cat die -0 "die: "
@cap make make/main.c
----
@cap tag Java 	JavaScript Go 	 Lisp C++ @cap Oops!
@cap tag Clang GCC GNU Linux
----
@cap{0:sun}@cap{1:moon}
	@cap{2:star}	@cap{3:}

	@cap brief This is tail brief.
*/

// Parser of @cap line in file

#endif
