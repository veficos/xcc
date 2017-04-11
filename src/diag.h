

#ifndef __DIAG__H__
#define __DIAG__H__


#include "config.h"
#include "color.h"
#include "vpfmt.h"


typedef struct token_s*             token_t;
typedef struct source_location_s*   source_location_t;


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


#define diag_nerrors(diag)      ((diag)->nerrors)

#define diag_nwarnings(diag)    ((diag)->nwarnings)

#define diag_errorf_with_line(diag, fn, line, column, fmt, ...)                                         \
    do {                                                                                                \
         pfmt(stderr, "%s:%u:%u " BRUSH_PURPLE("error: ") fmt "\n", fn, line, column, __VA_ARGS__);     \
         (diag)->nerrors++;                                                                             \
    } while (false)

#define diag_warningf_with_line(diag, fn, line, column, fmt, ...)                                       \
    do {                                                                                                \
         pfmt(stderr, "%s:%u:%u " BRUSH_PURPLE("warning: ") fmt "\n", fn, line, column, __VA_ARGS__);   \
         (diag)->nerrors++;                                                                             \
    } while (false)

#define diag_errorf_with_loc(diag, loc, fmt, ...)                                                       \
    do {                                                                                                \
         pfmt(stderr, "%S: " BRUSH_PURPLE("error: ") fmt "\n", loc, __VA_ARGS__);                       \
         (diag)->nerrors++;                                                                             \
    } while (false)

#define diag_warningf_with_loc(diag, loc, fmt, ...)                                                     \
    do {                                                                                                \
         pfmt(stderr, "%S: " BRUSH_PURPLE("warning: ") fmt "\n", loc, __VA_ARGS__);                     \
         (diag)->nwarnings++;                                                                           \
    } while (false)

#define diag_errorf_with_tok(diag, tok, fmt, ...)                                                       \
    do {                                                                                                \
         pfmt(stderr, "%T: " BRUSH_PURPLE("error: ") fmt "\n", tok, __VA_ARGS__);                       \
         (diag)->nerrors++;                                                                             \
    } while (false)

#define diag_warningf_with_tok(diag, tok, fmt, ...)                                                     \
    do {                                                                                                \
         pfmt(stderr, "%T: " BRUSH_PURPLE("warning: ") fmt "\n", tok, __VA_ARGS__);                     \
         (diag)->nwarnings++;                                                                           \
    } while (false)


#endif
