

#include "config.h"
#include "array.h"
#include "cstring.h"
#include "pmalloc.h"
#include "token.h"
#include "lexer.h"
#include "diag.h"
#include "map.h"
#include "set.h"
#include "preprocessor.h"


#ifndef TOKEN_EXPAND_NUMBER
#define TOKEN_EXPAND_NUMBER     24
#endif


static inline token_t __preprocessor_expand__(preprocessor_t pp);
static inline token_t __preprocessor_expand_newline__(preprocessor_t pp);
static inline array_t __preprocessor_subst__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline macro_t __preprocessor_create_object_macro__(preprocessor_t pp);
static inline token_t __preprocessor_except_identifier__(preprocessor_t pp);
static inline bool __preprocessor_untread_tokens__(preprocessor_t pp, array_t tokens);
static inline bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp);


preprocessor_t preprocessor_create(lexer_t lexer, option_t option, diag_t diag)
{
    preprocessor_t pp;
    array_t std_include_paths;
    map_t macros;

    if ((std_include_paths = array_create_n(sizeof(cstring_t), 8)) == NULL) {
        goto done;
    }

    if ((macros = map_create()) == NULL) {
        goto clean_paths;
    }

    pp = (preprocessor_t) pmalloc(sizeof(struct preprocessor_s));
    if (pp == NULL) {
        goto clean_macros;
    }

    pp->std_include_paths = std_include_paths;
    pp->lexer = lexer;
    pp->macros = macros;

    __preprocessor_predefined_std_include_paths__(pp);

    return pp;

clean_macros:
    map_destroy(macros);
clean_paths:
    array_destroy(std_include_paths);
done:
    return NULL;
}


void preprocessor_destroy(preprocessor_t pp)
{
    cstring_t *std_include_paths;
    size_t i;

    array_foreach(pp->std_include_paths, std_include_paths, i) {
        cstring_destroy(std_include_paths[i]);
    }

    map_destroy(pp->macros);
    array_destroy(pp->std_include_paths);

    pfree(pp);
}


bool preprocessor_add_include_path(preprocessor_t pp, const char *path)
{
    cstring_t *item;
    cstring_t cs;
    
    if ((cs = cstring_create(path)) == NULL) {
        return false;
    }

    if ((item = (cstring_t *)array_push(pp->std_include_paths)) == NULL) {
        cstring_destroy(cs);
        return false;
    }

    *item = cs;
    return true;
}


token_t preprocessor_peek(preprocessor_t pp)
{
    token_t tok = preprocessor_next(pp);
    if (tok->type != TOKEN_END) preprocessor_untread(pp, tok);
    return tok;
}


token_t preprocessor_next(preprocessor_t pp)
{
    token_t tok;
    for (;;) {
        tok = __preprocessor_expand__(pp);
    }
}


bool preprocessor_untread(preprocessor_t pp, token_t tok)
{
    assert(tok && tok->type != TOKEN_END);
    lexer_untread(pp->lexer, tok);
}


static inline
bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp)
{
    const char *std_paths[] = {
        "/usr/local/lib/occ/include",
        "/usr/local/include",
        "/usr/include",
        "/usr/include/linux",
        "/usr/include/x86_64-linux-gnu",
    };

    size_t npaths = sizeof(std_paths) / sizeof(const char*);

    size_t i;

    for (i = 0; i < npaths; i++) {
        cstring_t cs;

        if ((cs = array_push(pp->std_include_paths)) == NULL) {
            return false;
        }

        preprocessor_add_include_path(pp, std_paths[i]);
    }

    return true;
}


static inline 
token_t __preprocessor_expand__(preprocessor_t pp)
{
    token_t tok;
    for (;;) {
        tok = lexer_next(pp->lexer);
        if (tok->type != TOKEN_NEW_LINE) {
            return tok;
        }
    }
}


static inline
token_t __preprocessor_expand_newline__(preprocessor_t pp)
{
    token_t tok;
    macro_t macro;
    cstring_t name;

    tok = lexer_next(pp->lexer);
    if (tok->type != TOKEN_IDENTIFIER) {
        return tok;
    }

    if (tok->hideset && set_has(tok->hideset, tok->literals)) {
        return tok;
    }

    macro = map_find(pp->macros, tok->literals);
    if (macro == NULL) {
        return tok;
    }

    
    switch (macro->type) {
    case PP_MACRO_OBJECT: {
        array_t expand_tokens;

        if ((tok->hideset = tok->hideset ? tok->hideset : set_create()) == NULL) {
            return NULL;
        }
        
        set_add(tok->hideset, tok->literals);

        if ((expand_tokens = __preprocessor_subst__(pp, macro, NULL, tok->hideset)) == NULL) {
            set_destroy(tok->hideset);
            return NULL;
        }

        if (!__preprocessor_untread_tokens__(pp, expand_tokens)) {
            set_destroy(tok->hideset);
            return NULL;
        }

    }
    case PP_MACRO_FUNCTION:
    case PP_MACRO_PREDEF:
    case PP_MACRO_VARIADIC:
        ;
        
    }

    assert(false);
}


static inline 
array_t __preprocessor_subst__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset)
{
    array_t tokens;

    tokens = array_create_n(sizeof(struct token_s), TOKEN_EXPAND_NUMBER);
    if (tokens == NULL) {
        return NULL;
    }


}


static inline 
macro_t __preprocessor_create_maroc__(preprocessor_t pp)
{
    macro_t macro = (macro_t) pmalloc(sizeof(struct macro_s));
    return macro;
}


static inline 
token_t __preprocessor_except_identifier__(preprocessor_t pp)
{
    token_t tok = lexer_next(pp->lexer);
    if (tok->type != TOKEN_IDENTIFIER) {
        /* TODO: report error. */
    }
    return tok;
}


static inline 
bool __preprocessor_untread_tokens__(preprocessor_t pp, array_t tokens)
{
    token_t *toks;
    size_t i;

    toks = array_prototype(tokens, token_t);
    for (i = 0; i < array_length(tokens); i++) {
        if (!lexer_untread(pp->lexer, toks[i])) {
            return false;
        }
    }

    return true;
}
