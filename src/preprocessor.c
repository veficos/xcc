

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

#define NATIVE_MACRO_VARIADIC   "__VA_ARGS__"
#define NATIVE_MACRO_COUNTER    "__COUNTER__"
#define NATIVE_MACRO_DATE       "__DATE__"


static inline token_t __preprocessor_expand__(preprocessor_t pp);
static inline array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_function_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_object_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline token_t __preprocessor_except_identifier__(preprocessor_t pp);
static inline void __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens);
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


void map_scan_fn(void *privdata, const void *key, const void *value)
{
    macro_t macro = (macro_t)value;
    
    if (macro->type == PP_MACRO_OBJECT) {
        token_t *tokens;
        size_t i;
        array_foreach(macro->object_like.body, tokens, i) {
            token_destroy(tokens[i]);
        }
        array_destroy(macro->object_like.body);
    }

    pfree(macro);
    printf("privdata: %p, key: %s, value: %p\n", privdata, (const char*)key, value);
}


void preprocessor_destroy(preprocessor_t pp)
{
    cstring_t *std_include_paths;
    size_t i;

    array_foreach(pp->std_include_paths, std_include_paths, i) {
        cstring_destroy(std_include_paths[i]);
    }
    
    map_scan(pp->macros, map_scan_fn, NULL);

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
    if (tok->type != TOKEN_END) preprocessor_unget(pp, tok);
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


bool preprocessor_unget(preprocessor_t pp, token_t tok)
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
void __propagate_space__(array_t expand_tokens, token_t token)
{
    if ((expand_tokens != NULL) && (array_length(expand_tokens) > 0)) {
        token_t t = array_at(token_t, expand_tokens, 0);
        t->spaces = token->spaces;
    }
}


static inline 
token_t __preprocessor_expand__(preprocessor_t pp)
{
    token_t tok;
    macro_t macro;
    array_t expand_tokens = NULL;
    set_t hideset = NULL;


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

        if (macro->type == PP_MACRO_OBJECT) {

            hideset = tok->hideset ? set_dup(tok->hideset) : set_create();

            set_add(hideset, tok->cs);

            expand_tokens = __preprocessor_substitute__(pp, macro, NULL, hideset);

            __propagate_space__(expand_tokens, tok);

            __preprocessor_unget_tokens__(pp, expand_tokens);

            break;
        } else if (macro->type == PP_MACRO_FUNCTION) {

            __propagate_space__(expand_tokens, tok);

            __preprocessor_unget_tokens__(pp, expand_tokens);

            break;
        } else if (macro->type == PP_MACRO_NATIVE) {
            macro->native(tok);

            break;
        } else {
            assert(false);
        }
    }

    return __preprocessor_expand__(pp);
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
            token_destroy(hash);
            token_destroy(tok);
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
array_t __preprocessor_parse_function_like_params__(preprocessor_t pp)
{
    array_t params = NULL;

#undef  ADD_PARAM
#define ADD_PARAM

    for (;;) {
        token_t token = lexer_next(pp->lexer);

        if (token->type == TOKEN_R_PAREN) {
            return params;
        }

        if (token->type == TOKEN_NEWLINE) {
            /* TODO: report error */
            return params;
        }

        if (token->type == TOKEN_ELLIPSIS) {
            if (!lexer_try(pp->lexer, TOKEN_R_PAREN)) {
                /* TODO: report error */
            }
            return params;
        }

        if (token->type != TOKEN_IDENTIFIER) {
            /* the token may not appear in macro parameter list */
        }

        if (!lexer_try(pp->lexer, TOKEN_COMMA)) {
            if (!lexer_try(pp->lexer, TOKEN_R_PAREN)) {

            }
        }
    }

#undef  MAP_PUT

    return params;
}


static inline
bool __preprocessor_parse_function_like__(preprocessor_t pp, cstring_t macro_name)
{
    map_t params = NULL;
    array_t macro_body = NULL;

    /* eat ( */
    lexer_next(pp->lexer);
    
    params = __preprocessor_parse_function_like_params__(pp);

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
            /* TODO: cleanup macro_body items */

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
void __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens)
{
    token_t *toks;
    size_t i;

    if (tokens != NULL) {
        array_foreach(tokens, toks, i) {
            lexer_untread(pp->lexer, toks[i]);
        }
    }
}
