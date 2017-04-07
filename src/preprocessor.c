

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
static inline token_t __preprocessor_expand_newline__(preprocessor_t pp);
static inline array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_function_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_object_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline token_t __preprocessor_except_identifier__(preprocessor_t pp);
static inline bool __preprocessor_untread_tokens__(preprocessor_t pp, array_t tokens);
static inline bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash);
static inline bool __preprocessor_parse_define__(preprocessor_t pp);
static inline bool __preprocessor_parse_function_like__(preprocessor_t pp, cstring_t macro_name);
static inline bool __preprocessor_parse_function_parameter__(preprocessor_t pp);
static inline bool __preprocessor_parse_object_like__(preprocessor_t pp, cstring_t macro_name);
static inline bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp);
static inline bool __preprocessor_add_macro__(preprocessor_t pp, cstring_t macro_name,
    macro_type_t type, array_t params, array_t body, native_macro_pt native);


#define next_token(tok, pp, sweep)                             \
    do {                                                       \
        if ((tok = lexer_next(pp->lexer)) == NULL) {           \
            sweep                                              \
        }                                                      \
    } while(false)


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

        next_token(tok, pp, {
            return NULL;
        });

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

    next_token(tok, pp, {
        return NULL;
    });

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

        if ((expand_tokens = __preprocessor_substitute__(pp, macro, NULL, tok->hideset)) == NULL) {
            set_destroy(tok->hideset);
            return NULL;
        }

        if (!__preprocessor_untread_tokens__(pp, expand_tokens)) {
            set_destroy(tok->hideset);
            return NULL;
        }

    }
    case PP_MACRO_FUNCTION:
        ;
        
    case PP_MACRO_NATIVE:
        ;
    }

    assert(false);
}


static inline 
array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset)
{
    array_t tokens;
    array_t macro_body;

    tokens = array_create_n(sizeof(struct token_s), TOKEN_EXPAND_NUMBER);
    if (tokens == NULL) {
        return NULL;
    }

    return tokens;
}


static inline
bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash)
{
    token_t tok;
    cstring_t cs;

    if (hash->begin_of_line && hash->type == TOKEN_HASH && hash->hideset == NULL) {
        hash = lexer_next(pp->lexer);

        if (hash->type == TOKEN_NEW_LINE) {
            return false;
        }

        if (hash->type == TOKEN_NUMBER) {
            /* TODO: GNU remark linenum */
            return false;
        }

        cs = hash->literals;

        if (cstring_cmp(cs, "define") == 0) {
            __preprocessor_parse_define__(pp);
        }
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

    macro_name = tok->literals;

    /* if get the (, parse function like */
    tok = lexer_peek(pp->lexer);
    if (tok->type == TOKEN_L_PAREN && tok->leading_space == false) {
        return __preprocessor_parse_function_like__(pp, macro_name);
    }

    return __preprocessor_parse_object_like__(pp, macro_name);
}


static inline
bool __preprocessor_parse_function_like__(preprocessor_t pp, cstring_t macro_name)
{
    array_t params;
    array_t macro_body;

    return __preprocessor_add_macro__(pp, macro_name,
        PP_MACRO_FUNCTION, params, macro_body, NULL);
}


static inline
bool __preprocessor_parse_object_like__(preprocessor_t pp, cstring_t macro_name)
{
    array_t macro_body;

    if ((macro_body = array_create_n(sizeof(token_t), MACRO_BODY)) == NULL) {
        return false;
    }

    for (;;) {
        token_t tok;
        token_t *item;

        next_token(tok, pp, {
            /* TODO: clear macro_body */
            return false;
        });

        if (tok->type == TOKEN_HASHHASH) {
            /* TODO: '##' cannot appear at either end of a macro expansion */
            /* TODO: clear macro_body items */

            array_destroy(macro_body);
            return false;
        }

        if (tok->type == TOKEN_NEW_LINE) {
            break;
        }

        if ((item = (token_t*)array_push(macro_body)) == NULL) {
            return false;
        }

        *item = tok;
    }

    return __preprocessor_add_macro__(pp, macro_name,
        PP_MACRO_OBJECT, NULL, macro_body, NULL);
}


static inline 
bool __preprocessor_add_macro__(preprocessor_t pp, cstring_t macro_name, 
    macro_type_t type, array_t params, array_t body, native_macro_pt native)
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
        macro->function_like.params = body;
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
