#ifndef STRING_IMP_H
#define STRING_IMP_H

/* Include public header */
#include "string.h"

/*********
* struct *
*********/

struct String {
	int length;
	int capacity;
	String_type* buffer;
};

#endif
