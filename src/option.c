

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


void option_destroy(option_t *opt)
{
    assert(opt != NULL);
    pfree(opt);
}


void option_init(option_t *opt)
{
    opt->lang = LANG_STANDARD_DEFAULT;
    opt->infile = "";
    opt->outfile = "";
    opt->ferror_limit = 5;
    opt->cflag = false;
    opt->Eflag = false;
    opt->w_unterminated_comment = true;
    opt->w_backslash_newline_space = true;
}
