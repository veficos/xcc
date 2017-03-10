

#ifndef __DIAG__H__
#define __DIAG__H__


#include "config.h"
#include "token.h"
#include <stdarg.h>


#define MAX_COLUMN_HEAD  64


void diag_errorvf(const char *fmt, va_list ap);
void diag_errorf(const char *fmt, ...);
void diag_errorvf_with_line(source_location_t loc, const char *fmt, va_list ap);
void diag_errorf_with_line(source_location_t loc, const char *fmt, ...);


#endif
