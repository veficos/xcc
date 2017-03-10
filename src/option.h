

#ifndef __OPTION__H__
#define __OPTION__H__


#include "config.h"


typedef struct option_s {
    bool w_unterminated_comment;
} *option_t;


static inline
void option_init(option_t op)
{
    op->w_unterminated_comment = true;
}


#define option_get(op, filed) (op)->filed


#endif
