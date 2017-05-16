

#include "config.h"
#include "pmalloc.h"
#include "option.h"


option_t option_create(void)
{
    option_t option;

    option = (option_t) pmalloc(sizeof(struct option_s));

    option_init(option);

    return option;
}


void option_destroy(option_t option)
{
    assert(option != NULL);
    pfree(option);
}
