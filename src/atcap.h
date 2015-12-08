#ifndef ATCAP_H
#define ATCAP_H

#include "util.h"
#include "buffer.h"
#include "strarray.h"
#include "page.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct AtCap AtCap;

void
atcap_delete(AtCap* self); 

AtCap*
atcap_new(void); 

AtCap*
atcap_new_from_file(char const* fname);

// @cap brief This is brief string.

AtCap*
atcap_parse_stream(AtCap* self, FILE* stream);

/*
@cap brief This is brief string 2.

@cap tag Linux	Unix	Mac Windows 

@cap cat func/die.c -0 "die: "
@cap cat func/warn.c -0 "die: "
@cap make make/main.c

My name is @cap{0:hoge}, but my sex is @cap{1:hige}@cap{2:ehe}.
*/

#endif
