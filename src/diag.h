

#ifndef __DIAG__H__
#define __DIAG__H__


#include "config.h"


#define MAX_COLUMN_HEAD  64


typedef struct source_location_s* source_location_t;


typedef struct diag_s {
	size_t nwarnings;	
	size_t nerrors;
} *diag_t;


#define diag_nerrors(diag)      ((diag)->nerrors)
#define diag_nwarnings(diag)    ((diag)->nwarnings)


diag_t diag_create(void);
void diag_destroy(diag_t diag);
void diag_report(diag_t diag);

void diag_errorvf(diag_t diag, const char *fmt, va_list ap);
void diag_errorf(diag_t diag, const char *fmt, ...);

void diag_warningvf_with_line(diag_t diag, size_t line, size_t column,
    const char *fn, const char *fmt, va_list ap);
void diag_warningf_with_line(diag_t diag, size_t line, size_t column,
    const char *fn, const char *fmt, ...);

void diag_errorvf_with_loc(diag_t diag, source_location_t loc, const char *fmt, va_list ap);
void diag_errorf_with_loc(diag_t diag, source_location_t loc, const char *fmt, ...);

void diag_warningf_with_loc(diag_t diag, source_location_t loc, const char *fmt, ...);
void diag_warningvf_with_loc(diag_t diag, source_location_t loc, const char *fmt, va_list ap);

void diag_panic(const char *fmt, ...);

#endif
