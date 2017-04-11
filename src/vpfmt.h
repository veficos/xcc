

#ifndef __VPFMT__H__
#define __VPFMT__H__


#include "config.h"


void pfmt(FILE *fp, const char *fmt, ...);
void vpfmt(FILE *fp, const char *fmt, va_list ap);


#endif
