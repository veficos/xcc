

#include "config.h"
#include "pmalloc.h"
#include "option.h"


static option_t __option__ = {
    LANG_STANDARD_DEFAULT,
    "",
    "",
    5,
    false,
    false,
    true,
    true
};


option_t *option = &__option__;


option_t* option_create(void)
{
    option_t *option;

    option = (option_t*) pmalloc(sizeof(option_t));

    option_init(option);

    return option;
}


void option_destroy(option_t *option)
{
    assert(option != NULL);
    pfree(option);
}


void option_init(option_t *op)
{
    op->lang = LANG_STANDARD_DEFAULT;
    op->infile = "";
    op->outfile = "";
    op->ferror_limit = 5;
    op->cflag = false;
    op->Eflag = false;
    op->w_unterminated_comment = true;
    op->w_backslash_newline_space = true;
}
