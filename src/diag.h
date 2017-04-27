

#ifndef __DIAG__H__
#define __DIAG__H__


#include "config.h"
#include "color.h"


typedef struct diag_s {
	size_t nwarnings;	
	size_t nerrors;
} *diag_t;


void diag_fmt(const char *fmt, ...);
void diag_panic(const char *fmt, ...);

diag_t diag_create(void);
void diag_destroy(diag_t diag);
void diag_report(diag_t diag);
void diag_errorvf(diag_t diag, const char *fmt, va_list ap);
void diag_errorf(diag_t diag, const char *fmt, ...);

void debug_linenote(const char* linenote, size_t start, size_t tilde);

#define diag_nerrors(diag)      ((diag)->nerrors)

#define diag_nwarnings(diag)    ((diag)->nwarnings)


#define diag_errorf_with_line(diag, fn, line, column, linenote, start, tilde, fmt, ...)                 \
    do {                                                                                                \
        fprintf(stderr, "%s:%u:%u " BRUSH_RED("error: ") fmt "\n", fn, line, column, __VA_ARGS__);      \
        if ((linenote) != NULL) debug_linenote((linenote), (start), (tilde));                           \
        (diag)->nerrors++;                                                                              \
    } while (false)


#define diag_warningf_with_line(diag, fn, line, column, linenote, start, tilde, fmt, ...)               \
    do {                                                                                                \
        fprintf(stderr, "%s:%u:%u " BRUSH_PURPLE("warning: ") fmt "\n", fn, line, column, __VA_ARGS__); \
        if ((linenote) != NULL) debug_linenote((linenote), (start), (tilde));                           \
        (diag)->nerrors++;                                                                              \
    } while (false)


#define diag_errorf_with_loc(diag, loc, fmt, ...)                                                       \
    diag_errorf_with_line((diag), (loc)->fn, (loc)->line, (loc)->column,                                \
        (loc)->linenote, (loc)->column, 0, fmt, __VA_ARGS__)
    

#define diag_warningf_with_loc(diag, loc, fmt, ...)                                                     \
    diag_warningf_with_line((diag), (loc)->fn, (loc)->line, (loc)->column,                              \
        (loc)->linenote, (loc)->column, 0, fmt, __VA_ARGS__)


#define diag_errorf_with_tok(diag, tok, fmt, ...)                                                       \
    diag_errorf_with_line((diag), (tok)->loc->fn, (tok)->loc->line, (tok)->loc->column,                 \
        (tok)->loc->linenote, (tok)->loc->column, (tok)->cs ? cstring_length((tok)->cs) : 0, fmt, __VA_ARGS__)


#define diag_warningf_with_tok(diag, tok, fmt, ...)                                                     \
     diag_warningf_with_line((diag), (tok)->loc->fn, (tok)->loc->line, (tok)->loc->column,              \
        (tok)->loc->linenote, (tok)->loc->column, (tok)->cs ? cstring_length((tok)->cs) : 0, fmt, __VA_ARGS__)


#endif
