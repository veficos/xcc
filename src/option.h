

#ifndef __OPTION__H__
#define __OPTION__H__


#include "config.h"


typedef enum lang_standard_e {
    LANG_STANDARD_ANSI,                 /* ISO C90 (for C) or ISO C++ 1998 (for C++) */
    LANG_STANDARD_C89,                  /* ISO C90 (sic) */
    LANG_STANDARD_C99,                  /* ISO C99 */
    LANG_STANDARD_C11,                  /* ISO C11 */
    LANG_STANDARD_GNU89,                /* ISO C90 plus GNU extensions (including some C99) */
    LANG_STANDARD_GNU99,                /* ISO C99 plus GNU extensions */
    LANG_STANDARD_GNU11,                /* ISO C11 plus GNU extensions */
    LANG_STANDARD_DEFAULT = LANG_STANDARD_ANSI
} lang_standard_t;


typedef struct option_s {
    lang_standard_t lang;

    const char* infile;
    const char* outfile;

    size_t ferror_limit;

    bool cflag: 1;
    bool Sflag: 1;
    bool Eflag: 1;
    bool dump_ast: 1;

    bool w_unterminated_comment: 1;
    bool w_backslash_newline_space: 1;
} option_t;


extern option_t* option;

#define option_get(opt, filed) (opt)->filed

option_t* option_create(void);
void option_init(option_t *opt);
void option_destroy(option_t *opt);


#endif
