

#ifndef __DIAGNOSTOR__H__
#define __DIAGNOSTOR__H__


#include "config.h"


typedef struct array_s array_t;
typedef struct linenote_caution_s linenote_caution_t;


typedef enum diagnostor_level_e {
    DIAGNOSTOR_LEVEL_NORMAL,
    DIAGNOSTOR_LEVEL_NOTE,
    DIAGNOSTOR_LEVEL_WARNING,
    DIAGNOSTOR_LEVEL_ERROR,
} diagnostor_level_t;


typedef struct diagnostor_s {
    size_t nerrors;
    size_t nwarnings;
} diagnostor_t;


extern diagnostor_t* diagnostor;


#define diagnostor_has_error(diagnostor)    \
    ((diagnostor)->nerrors != 0)

#define diagnostor_has_warning(diagnostor)  \
    ((diagnostor)->nwarnnings != 0)


diagnostor_t* diagnostor_create(void);
void diagnostor_destroy(diagnostor_t *diag);
void diagnostor_note(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, ...);
void diagnostor_note_with_location(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                   size_t line, size_t column, const char *fmt, ...);
void diagnostor_note_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                           size_t line, size_t column, linenote_t linenote,
                                           linenote_caution_t *linenote_caution, const char *fmt, ...);
void diagnostor_panic(diagnostor_t *diag, const char *fmt, ...);
void diagnostor_panic_with_location(diagnostor_t *diag, const char *fn,
                                    size_t line, size_t column, const char *fmt, ...);
void diagnostor_note_linenote(diagnostor_t *diag, diagnostor_level_t level, linenote_t linenote,
                              linenote_caution_t *linenote_caution);
void diagnostor_report(diagnostor_t *diag);


#endif
