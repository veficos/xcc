

#ifndef __OPTION__H__
#define __OPTION__H__


#include "config.h"
#include "cstring.h"


typedef struct option_s {
    cstring_t infile;
    cstring_t outfile;

    bool cflag;
    bool Sflag;
    bool Eflag;
    bool dump_ast;

    bool w_unterminated_comment;
} *option_t;


typedef struct reader_option_s {
    bool w_backslash_newline_space;
} *reader_option_t;

static inline
void option_init(option_t op)
{
    op->cflag = false;
    op->Eflag = false;
    op->w_unterminated_comment = true;
}


#define option_get(op, filed) (op)->filed

option_t option_create(void);
void option_destroy(option_t option);


#endif
