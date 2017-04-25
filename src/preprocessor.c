

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


#ifndef MACRO_BODY
#define MACRO_BODY              64
#endif


static inline token_t __preprocessor_expand__(preprocessor_t pp);
static inline array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_function_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_object_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline token_t __preprocessor_except_identifier__(preprocessor_t pp);
static inline bool __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens);
static inline bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash);
static inline bool __preprocessor_parse_define__(preprocessor_t pp);
static inline bool __preprocessor_parse_function_like__(preprocessor_t pp, cstring_t macro_name);
static inline bool __preprocessor_parse_function_parameter__(preprocessor_t pp);
static inline bool __preprocessor_parse_object_like__(preprocessor_t pp, cstring_t macro_name);
static inline bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp);
static inline bool __preprocessor_add_macro__(preprocessor_t pp, cstring_t macro_name,
    macro_type_t type, map_t params, array_t body, native_macro_pt native);


preprocessor_t preprocessor_create(lexer_t lexer, option_t option, diag_t diag)
{
    preprocessor_t pp;

    pp = (preprocessor_t) pmalloc(sizeof(struct preprocessor_s));

    pp->std_include_paths = array_create_n(sizeof(cstring_t), 8);

    pp->macros = map_create();
    pp->lexer = lexer;

    __preprocessor_predefined_std_include_paths__(pp);

    return pp;
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


void preprocessor_add_include_path(preprocessor_t pp, const char *path)
{
    cstring_t *item;
    
    item = array_push(pp->std_include_paths);
    *item = cstring_create(path);
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

        if (__preprocessor_parse_directive__(pp, tok)) {
            continue;
        }

        return tok;
    }
}


bool preprocessor_untread(preprocessor_t pp, token_t tok)
{
    assert(tok && tok->type != TOKEN_END);
    lexer_untread(pp->lexer, tok);
    return true;
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
        preprocessor_add_include_path(pp, std_paths[i]);
    }

    return true;
}


static inline 
token_t __preprocessor_expand__(preprocessor_t pp)
{
    token_t tok;
    macro_t macro;

    for (;;) {
        tok = lexer_next(pp->lexer);

        if (tok->type != TOKEN_IDENTIFIER || tok->type == TOKEN_NEWLINE) {
            return tok;
        }

        if (tok->hideset && set_has(tok->hideset, tok->cs)) {
            return tok;
        }

        if ((macro = map_find(pp->macros, tok->cs)) == NULL) {
            return tok;
        }

        switch (macro->type) {
        case PP_MACRO_OBJECT: {
            array_t expand_tokens;

            tok->hideset = tok->hideset ? tok->hideset : set_create();

            set_add(tok->hideset, tok->cs);

            expand_tokens = __preprocessor_substitute__(pp, macro, NULL, tok->hideset);

            __preprocessor_unget_tokens__(pp, expand_tokens);
            break;
        }
        case PP_MACRO_FUNCTION: {

        }
        case PP_MACRO_NATIVE: {

        }
        default:
            assert(false);
        }
    }
}


static 
bool __add_hide_set__(set_t hideset, array_t expand_tokens)
{
    token_t *tokens;
    size_t i;

    array_foreach(expand_tokens, tokens, i) {
        tokens[i] = token_dup(tokens[i]);
        tokens[i]->hideset = tokens[i]->hideset ? 
            (hideset ? set_union(tokens[i]->hideset, hideset) : NULL) : hideset;
    }

    return true;
}


static inline 
array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset)
{
    array_t expand_tokens;
    token_t *item;
    token_t *macro_tokens;
    size_t i;

    expand_tokens = array_create_n(sizeof(token_t), TOKEN_EXPAND_NUMBER);

    switch (macro->type) {
    case PP_MACRO_OBJECT: {
        array_foreach(macro->object_like.body, macro_tokens, i) {
            item = array_push(expand_tokens);
            *item = macro_tokens[i];
        }
        break;
    }
    case PP_MACRO_FUNCTION: {
    
        break;
    }
    default:
        assert(false);
    }

    __add_hide_set__(hideset, expand_tokens);
    return expand_tokens;
}


static inline
bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash)
{
    if (hash->begin_of_line && hash->type == TOKEN_HASH && hash->hideset == NULL) {
        token_t tok;
        cstring_t cs;

        tok = lexer_next(pp->lexer);

        if (tok->type == TOKEN_NEWLINE) {
            return false;
        }

        if (tok->type == TOKEN_NUMBER) {
            /* TODO: GNU remark linenum */
            return false;
        }

        cs = tok->cs;

        if (cstring_cmp(cs, "define") == 0) {
            __preprocessor_parse_define__(pp);
        }

        return true;
    }

    return false;
}


static inline 
bool __preprocessor_parse_define__(preprocessor_t pp)
{
    token_t tok;
    cstring_t macro_name;

    tok = lexer_next(pp->lexer);
    if (tok->type != TOKEN_IDENTIFIER) {
        /* TODO: macro name must be an identifier */
    }

    macro_name = tok->cs;

    /* if get the (, parse function like */
    tok = lexer_peek(pp->lexer);
    if (tok->type == TOKEN_L_PAREN && tok->spaces == 0) {
        return __preprocessor_parse_function_like__(pp, macro_name);
    }

    return __preprocessor_parse_object_like__(pp, macro_name);
}


static inline
bool __preprocessor_parse_function_like__(preprocessor_t pp, cstring_t macro_name)
{
    map_t params = NULL;
    array_t macro_body = NULL;

    return __preprocessor_add_macro__(pp, macro_name,
        PP_MACRO_FUNCTION, params, macro_body, NULL);
}


static inline
bool __preprocessor_parse_object_like__(preprocessor_t pp, cstring_t macro_name)
{
    array_t macro_body;

    macro_body = array_create_n(sizeof(token_t), MACRO_BODY);

    for (;;) {
        token_t tok;
        token_t *item;

        tok = lexer_next(pp->lexer);

        if (tok->type == TOKEN_HASHHASH) {
            /* TODO: '##' cannot appear at either end of a macro expansion */
            /* TODO: clear macro_body items */

            array_destroy(macro_body);
            return false;
        }

        if (tok->type == TOKEN_NEWLINE) {
            break;
        }

        item = (token_t*)array_push(macro_body);
        *item = tok;
    }

    return __preprocessor_add_macro__(pp, macro_name,
        PP_MACRO_OBJECT, NULL, macro_body, NULL);
}


static inline 
bool __preprocessor_add_macro__(preprocessor_t pp, cstring_t macro_name, 
    macro_type_t type, map_t params, array_t body, native_macro_pt native)
{
    macro_t macro;;

    if ((macro = (macro_t) pmalloc(sizeof(struct macro_s))) == NULL) {
        return false;
    }

    macro->type = type;
    switch (type) {
    case PP_MACRO_FUNCTION:
        assert(native == NULL);
        macro->function_like.params = params;
        macro->function_like.body = body;
        break;
    case PP_MACRO_OBJECT:
        assert(params == NULL && native == NULL);
        macro->object_like.body = body;
        break;
    case PP_MACRO_NATIVE:
        assert(params == NULL && body == NULL);
        macro->native = native;
        break;
    default:
        assert(false);
    }

    return map_add(pp->macros, macro_name, macro);
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
bool __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens)
{
    token_t *toks;
    size_t i;

    array_foreach(tokens, toks, i) {
        if (!lexer_untread(pp->lexer, toks[i])) {
            return false;
        }
    }

    return true;
}
