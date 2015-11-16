#ifndef TERM_H
#define TERM_H

#include <stdio.h>
#include <stdarg.h>

void
term_flush(void);

void
term_eflush(void);

int
term_printf(char const* fmt, ...);

int
term_eprintf(char const* fmt, ...);

#endif

