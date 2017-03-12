

#ifndef __DIAG__H__
#define __DIAG__H__


#include "config.h"
#include "token.h"
#include <stdarg.h>


#define MAX_COLUMN_HEAD  64


typedef struct diag_s {
	size_t nwarnings;	
	size_t nerrors;
} *diag_t;

diag_t diag_create();
void diag_destroy(diag_t diag);
void diag_errorvf(diag_t diag, const char *fmt, va_list ap);
void diag_errorf(diag_t diag, const char *fmt, ...);
void diag_errorvf_with_location(diag_t diag, source_location_t loc, const char *fmt, va_list ap);
void diag_errorf_with_location(diag_t diag, source_location_t loc, const char *fmt, ...);


#endif
