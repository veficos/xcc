

#include "config.h"
#include "token.h"


token_t token_dup(token_t tok)
{
    token_t ret;

    if ((ret = pmalloc(sizeof(struct token_s))) == NULL) {
        return NULL;
    }
    
    ret->type = tok->type;
    ret->location = tok->location;

    return ret;
}
