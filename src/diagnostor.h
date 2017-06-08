

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


#define diagnostor_has_error(diag) \
    ((diag)->nerrors != 0)

#define diagnostor_has_warning(diag) \
    ((diag)->nwarnnings != 0)

#define has_error() \
    ((diagnostor)->nerrors != 0)

#define has_warning() \
    ((diagnostor)->nwarnnings != 0)

#define report() \
    diagnostor_report(diagnostor)


void warningf(const char *fmt, ...);
void errorf(const char *fmt, ...);
void warningf_with_location(const char *fn, size_t line, size_t column, const char *fmt, ...);
void errorf_with_location(const char *fn, size_t line, size_t column, const char *fmt, ...);
void warningf_with_linenote(const char *fn, size_t line, size_t column, linenote_t linenote,
                            const char *fmt, ...);
void errorf_with_linenote(const char *fn, size_t line, size_t column, linenote_t linenote,
                          const char *fmt, ...);
void warningf_with_linenote_caution(const char *fn, size_t line, size_t column, linenote_t linenote,
                                    linenote_caution_t *linenote_caution, const char *fmt, ...);
void errorf_with_linenote_caution(const char *fn, size_t line, size_t column, linenote_t linenote,
                                  linenote_caution_t *linenote_caution, const char *fmt, ...);
void warningf_with_linenote_position(const char *fn, size_t line, size_t column, linenote_t linenote,
                                     size_t start, size_t length, const char *fmt, ...);
void errorf_with_linenote_position(const char *fn, size_t line, size_t column, linenote_t linenote,
                                   size_t start, size_t length, const char *fmt, ...);
void warningf_with_token(token_t *token, const char *fmt, ...);
void errorf_with_token(token_t *token, const char *fmt, ...);
void panicf(const char *fmt, ...);
void panicf_with_location(const char *fn, size_t line,
                          size_t column, const char *fmt, ...);


diagnostor_t* diagnostor_create(void);
void diagnostor_destroy(diagnostor_t *diag);
void diagnostor_notef(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, ...);
void diagnostor_notevf(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, va_list args);
void diagnostor_notef_with_location(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                   size_t line, size_t column, const char *fmt, ...);
void diagnostor_notevf_with_location(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                   size_t line, size_t column, const char *fmt, va_list args);
void diagnostor_notef_with_linenote(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                    size_t line, size_t column, linenote_t linenote, const char *fmt, ...);
void diagnostor_notevf_with_linenote(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                     size_t line, size_t column, linenote_t linenote, const char *fmt, va_list args);
void diagnostor_notef_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                           size_t line, size_t column, linenote_t linenote,
                                           linenote_caution_t *linenote_caution, const char *fmt, ...);
void diagnostor_notevf_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level,
                                             const char *fn, size_t line, size_t column, linenote_t linenote,
                                             linenote_caution_t *linenote_caution, const char *fmt, va_list args);
void diagnostor_panicf(diagnostor_t *diag, const char *fmt, ...);
void diagnostor_panicvf(diagnostor_t *diag, const char *fmt, va_list args);
void diagnostor_panicf_with_location(diagnostor_t *diag, const char *fn,
                                     size_t line, size_t column, const char *fmt, ...);
void diagnostor_panicvf_with_location(diagnostor_t *diag, const char *fn,
                                     size_t line, size_t column, const char *fmt, va_list args);
void diagnostor_note_linenote(diagnostor_t *diag, linenote_t linenote);
void diagnostor_note_linenote_caution(diagnostor_t *diag, diagnostor_level_t level, linenote_t linenote,
                                      linenote_caution_t *linenote_caution);

void diagnostor_report(diagnostor_t *diag);


#endif
