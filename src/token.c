

#include "config.h"
#include "token.h"


token_t token_create(void)
{
    token_t tok;
    cstring_t cs;
    source_location_t loc;

    if ((cs = cstring_create_n(NULL, DEFUALT_LITERALS_LENGTH)) == NULL) {
        goto done;
    }
    
    if ((loc = source_location_create()) == NULL) {
        goto clean_cs;
    }

    if ((tok = (token_t) pmalloc(sizeof(struct token_s))) == NULL) {
        goto clean_loc;
    }

    tok->type = TOKEN_UNKNOWN;
    tok->literals = cs;
    tok->location = loc;
    return tok;

clean_loc:
    source_location_destroy(loc);

clean_cs:
    cstring_destroy(cs);

done:
    return NULL;
}


void token_destroy(token_t tok)
{
    assert(tok);

    if (tok->literals != NULL) {
        cstring_destroy(tok->literals);
    }

    source_location_destroy(tok->location);

    pfree(tok);
}


void token_init(token_t token)
{
    token->type = TOKEN_UNKNOWN;

    cstring_clear(token->literals);

    source_location_init(token->location, 0, 0, NULL, NULL);
}


token_t token_dup(token_t tok)
{
    token_t ret;

    if ((ret = pmalloc(sizeof(struct token_s))) == NULL) {
        goto done;
    }
    
    ret->type = tok->type;
    ret->literals = NULL;

    if ((tok->literals != NULL && cstring_length(tok->literals) != 0) &&
        ((ret->literals = cstring_dup(tok->literals)) == NULL)) {
        goto clean_tok;
    }

    if ((ret->location = source_location_dup(tok->location)) == NULL) {
        goto clean_literals;
    }

    return ret;

clean_literals:
    cstring_destroy(ret->literals);

clean_tok:
    pfree(ret);

done:
    return NULL;
}


source_location_t source_location_create(void)
{
    source_location_t loc;
    if ((loc = (source_location_t) pmalloc(sizeof(struct source_location_s))) == NULL) {
        return NULL;
    }

    loc->current_line = NULL;
    loc->filename = NULL;
    loc->line = 0;
    loc->column = 0;
    return loc;
}


void source_location_destroy(source_location_t loc)
{
    assert(loc);

    if (loc->filename != NULL) {
        cstring_destroy(loc->filename);
    }

    if (loc->current_line != NULL) {
        cstring_destroy(loc->current_line);
    }

    pfree(loc);
}


source_location_t source_location_dup(source_location_t loc)
{
    source_location_t ret;

    if ((ret = (source_location_t) pmalloc(sizeof(struct source_location_s))) == NULL) {
        return NULL;
    }

    ret->current_line = loc->current_line;
    ret->filename = loc->filename;
    ret->line = loc->line;
    ret->column = loc->column;
    return ret;
}

