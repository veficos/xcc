

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


#undef  ERRORF_WITH_TOKEN
#define ERRORF_WITH_TOKEN(tok, fmt, ...) \
    diag_errorf_with_tok((pp)->diag, (tok), fmt, __VA_ARGS__)


#undef  WARNINGF_WITH_TOKEN
#define WARNINGF_WITH_TOKEN(tok, fmt, ...) \
    diag_warningf_with_tok((pp)->diag, (tok), fmt, __VA_ARGS__)


#undef  ERRORF_WITH_LOC
#define ERRORF_WITH_LOC(loc, fmt, ...) \
    diag_errorf_with_loc((pp)->diag, (loc), fmt, __VA_ARGS__)


#undef  WARNINGF_WITH_LOC
#define WARNINGF_WITH_LOC(loc, fmt, ...) \
    diag_warningf_with_loc((pp)->diag, (loc), fmt, __VA_ARGS__)


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
static inline array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, map_t args, set_t hideset);
static inline array_t __preprocessor_substitute_function_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline array_t __preprocessor_substitute_object_like__(preprocessor_t pp, macro_t macro, array_t args, set_t hideset);
static inline token_t __preprocessor_except_identifier__(preprocessor_t pp);
static inline void __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens);
static inline bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash);
static inline bool __preprocessor_parse_define__(preprocessor_t pp);
static inline bool __preprocessor_parse_function_like__(preprocessor_t pp, token_t token);
static inline bool __preprocessor_parse_object_like__(preprocessor_t pp, token_t token);
static inline bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp);
static inline bool __preprocessor_add_object_like_macro__(preprocessor_t pp, cstring_t macro_name, array_t body);
static inline bool __preprocessor_add_function_like_macro__(preprocessor_t pp, cstring_t macro_name, bool is_variadic, array_t params, array_t body);
static inline bool __preprocessor_add_native_like_macro__(preprocessor_t pp, cstring_t macro_name, native_macro_pt native_macro_fn);
static inline void __preprocessor_skip_one_line__(preprocessor_t pp);


preprocessor_t preprocessor_create(lexer_t lexer, option_t option, diag_t diag)
{
    preprocessor_t pp;

    pp = (preprocessor_t) pmalloc(sizeof(struct preprocessor_s));

    pp->std_include_paths = array_create_n(sizeof(cstring_t), 8);

    pp->macros = map_create();
    pp->lexer = lexer;
    pp->diag = diag;

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
    //printf("privdata: %p, key: %s, value: %p\n", privdata, (const char*)key, value);
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
    for (;;) {
        token_t tok = __preprocessor_expand__(pp);

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
void __add_token__(array_t *a, token_t token)
{
    token_t *item;
    if (*a == NULL) {
        *a = array_create_n(sizeof(token_t), 8);
    }

    item = array_push(*a);
    *item = token;
}

static inline 
array_t __preprocessor_parse_function_like_argument__(preprocessor_t pp)
{
    array_t arg = NULL;

    int level = 0;

    for (; !lexer_is_empty(pp->lexer); ) {
        token_t token = lexer_peek(pp->lexer);
        if (token->type == TOKEN_L_PAREN) {
            level++;
        }
        if (token->type == TOKEN_COMMA ||
            token->type == TOKEN_R_PAREN) {
            if (level == 0) {
                break;
            }else {
                level--;
            }
        }

        __add_token__(&arg, token);

        lexer_next(pp->lexer);
    }

    return arg;
}


static inline
void __preprocessor_add_function_like_argument__(preprocessor_t pp, map_t *args, 
    cstring_t arg_name, array_t arg)
{
    if (*args == NULL) {
        *args = map_create();
    }

    map_add(*args, arg_name, arg);
}


static inline 
token_t __preprocessor_parse_function_like_arguments__(preprocessor_t pp, macro_t macro, map_t *args)
{
    token_t token, r_paren_token;
    size_t argc;
    bool is_variadic = macro->function_like.is_variadic;
    array_t body = macro->function_like.body;
    array_t params = macro->function_like.params;
    token_t *param_names = array_prototype(params, token_t);
    size_t i;

    argc = params ? array_length(params) : 0;


    array_t va_args = array_create_n(sizeof(token_t), 8);

    
    for (i = 0; ; i++) {
        array_t arg;
        
        arg = __preprocessor_parse_function_like_argument__(pp);
        if (arg == NULL) {
            break;
        }
        
        if (i >= argc) {
            array_append(va_args, arg);
        } else {
            __preprocessor_add_function_like_argument__(pp, args, param_names[i]->cs, arg);
        }

        token = lexer_peek(pp->lexer);
        if (token->type == TOKEN_R_PAREN) {
            break;
        }

        if (token->type != TOKEN_COMMA) {
            ERRORF_WITH_TOKEN(token, "expect a comma");
            break;
        }

        if (i >= argc) {
            (*(token_t*)array_push(va_args)) = token;
        }

        lexer_next(pp->lexer);
    }

    __preprocessor_add_function_like_argument__(pp, args, cstring_create("__VA_ARGS__"), va_args);
    
    r_paren_token = lexer_peek(pp->lexer);
    if (r_paren_token->type != TOKEN_R_PAREN) {

    }
    lexer_next(pp->lexer);
    return r_paren_token;
}


static inline 
token_t __preprocessor_expand__(preprocessor_t pp)
{
    token_t tok;
    macro_t macro;
    array_t expand_tokens = NULL;
    set_t hideset = NULL;
    set_t r_paren_hideset = NULL;

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
            map_t args = NULL;
            token_t r_paren;

            if (!lexer_try(pp->lexer, TOKEN_L_PAREN)) {
                return tok;
            }

            r_paren = __preprocessor_parse_function_like_arguments__(pp, macro, &args);
            
            hideset = tok->hideset ? set_dup(tok->hideset) : set_create();
            r_paren_hideset = r_paren->hideset ? set_dup(r_paren->hideset) : set_create();

            hideset = set_intersection(hideset, r_paren_hideset);

            set_add(hideset, tok->cs);

            expand_tokens = __preprocessor_substitute__(pp, macro, args, hideset);

            __propagate_space__(expand_tokens, tok);

            __preprocessor_unget_tokens__(pp, expand_tokens);

            break;
        } else if (macro->type == PP_MACRO_NATIVE) {
            macro->native_macro_fn(tok);

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
array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, map_t args, set_t hideset)
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
        if (macro->function_like.body != NULL) {
            array_foreach(macro->function_like.body, macro_tokens, i) {
                if (macro_tokens[i]->type == TOKEN_IDENTIFIER) {
                    array_t arg = map_find(args, macro_tokens[i]->cs);
                    if (arg != NULL) {
                        size_t j;
                        for (j = 0; j < array_length(arg); j++) {
                            item = array_push(expand_tokens);
                            *item = token_dup(array_at(token_t, arg, j));
                            (*item)->spaces = macro_tokens[i]->spaces;
                        }
                        continue;
                    }
                }
                item = array_push(expand_tokens);
                *item = macro_tokens[i];
            }
        }
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
    token_t macro_name_token;
    token_t r_paren_token;

    macro_name_token = lexer_next(pp->lexer);
    if (macro_name_token->type != TOKEN_IDENTIFIER) {
        /* TODO: macro name must be an identifier */
    }

    /* if get the (, parse function like */
    r_paren_token = lexer_peek(pp->lexer);
    if (r_paren_token->type == TOKEN_L_PAREN && r_paren_token->spaces == 0) {
        return __preprocessor_parse_function_like__(pp, macro_name_token);
    }

    return __preprocessor_parse_object_like__(pp, macro_name_token);
}


static
bool __preprocessor_add_function_like_param__(preprocessor_t pp, array_t *params, token_t param_token)
{
    token_t *tokens;
    token_t *item;
    size_t i;

    if (*params == NULL) {
        *params = array_create_n(sizeof(token_t), 8);
    }

    array_foreach(*params, tokens, i) {
        if (cstring_cmp(tokens[i]->cs, param_token->cs) == 0) {
            ERRORF_WITH_TOKEN(param_token, 
                "duplicate macro parameter \"%s\"", tok2s(param_token));
            return false;
        }
    }

    item = array_push(*params);
    *item = param_token;
    return true;
}


static inline
bool __preprocessor_parse_function_like_params__(preprocessor_t pp, array_t *params, bool *variadic)
{
    token_t token = NULL;
   
    for (;;) {
        token = lexer_next(pp->lexer);

        if (token->type == TOKEN_R_PAREN) {
            return true;
        }

        if (token->type == TOKEN_ELLIPSIS) {
            token = lexer_peek(pp->lexer);
            if (token->type != TOKEN_R_PAREN) {
                break;
            }

            *variadic = true;
            lexer_next(pp->lexer);
            return true;
        }

        if (token->type == TOKEN_COMMA) {
            ERRORF_WITH_TOKEN(token, "parameter name missing");
            break;
        }

        if (token->type == TOKEN_NEWLINE || token->type == TOKEN_END) {
            goto missing_r_paren;
        }

        if (token->type == TOKEN_IDENTIFIER) {
            if (!__preprocessor_add_function_like_param__(pp, params, token)) {
                break;
            }
        } else {
            ERRORF_WITH_TOKEN(token, "\"%s\" may not appear in macro parameter list", tok2s(token));
        }
        
    
        token = lexer_peek(pp->lexer);
        if (token->type != TOKEN_COMMA) {
            token = lexer_peek(pp->lexer);

            if (token->type != TOKEN_R_PAREN) {
                ERRORF_WITH_TOKEN(token, "macro parameters must be comma-separated");
                break;
            }
            lexer_next(pp->lexer);
            return true;
        }

        lexer_next(pp->lexer);
    }

missing_r_paren:
    ERRORF_WITH_TOKEN(token, "missing ')' in macro parameter list");
    return false;
}


static inline
bool __preprocessor_add_function_like_body__(preprocessor_t pp, array_t *macro_body, token_t token)
{
    token_t *item;

    if (*macro_body == NULL) {
        *macro_body = array_create_n(sizeof(token_t), 8);
    }

    item = array_push(*macro_body);
    *item = token;
    return true;
}


static inline
bool __preprocessor_parse_function_like_body__(preprocessor_t pp, array_t *macro_body)
{
    for (; !lexer_is_empty(pp->lexer); ) {
        token_t token = lexer_peek(pp->lexer);
        if (token->type == TOKEN_NEWLINE) {
            return true;
        }
        __preprocessor_add_function_like_body__(pp, macro_body, token);
        lexer_next(pp->lexer);
    }
    return false;
}


static inline
bool __preprocessor_parse_function_like__(preprocessor_t pp, token_t macroname_token)
{
    array_t macro_params = NULL;
    array_t macro_body = NULL;
    bool is_variadic = false;

    /* eat '(' */
    lexer_next(pp->lexer);

    if (!__preprocessor_parse_function_like_params__(pp, &macro_params, &is_variadic)) {
        __preprocessor_skip_one_line__(pp);
        return false;
    }

    if (!__preprocessor_parse_function_like_body__(pp, &macro_body)) {
        __preprocessor_skip_one_line__(pp);
        return false;
    }

    return __preprocessor_add_function_like_macro__(pp, macroname_token->cs,
        is_variadic, macro_params, macro_body);
}


static inline
bool __preprocessor_parse_object_like__(preprocessor_t pp, token_t macro_name_token)
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

    return __preprocessor_add_object_like_macro__(pp, macro_name_token->cs, macro_body);
}


static inline
bool __preprocessor_add_object_like_macro__(preprocessor_t pp, cstring_t macro_name, array_t body)
{
    macro_t macro;;

    if ((macro = (macro_t) pmalloc(sizeof(struct macro_s))) == NULL) {
        return false;
    }

    macro->type = PP_MACRO_OBJECT;

    macro->object_like.body = body;

    map_add(pp->macros, macro_name, macro);

    return true;
}


static inline
bool __preprocessor_add_function_like_macro__(preprocessor_t pp, cstring_t macro_name,
    bool is_variadic, array_t params, array_t body)
{
    macro_t macro;;

    if ((macro = (macro_t) pmalloc(sizeof(struct macro_s))) == NULL) {
        return false;
    }

    macro->type = PP_MACRO_FUNCTION;

    macro->function_like.is_variadic = is_variadic;
    macro->function_like.params = params;
    macro->function_like.body = body;

    map_add(pp->macros, macro_name, macro);

    return true;
}


static inline
bool __preprocessor_add_native_like_macro__(preprocessor_t pp, cstring_t macro_name,
    native_macro_pt native_macro_fn)
{
    macro_t macro;;

    if ((macro = (macro_t)pmalloc(sizeof(struct macro_s))) == NULL) {
        return false;
    }

    macro->type = PP_MACRO_NATIVE;
    macro->native_macro_fn = native_macro_fn;

    map_add(pp->macros, macro_name, macro);

    return true;
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

    if (tokens != NULL) {
        size_t i;
        for (i = array_length(tokens); i--; ) {
            lexer_untread(pp->lexer, array_at(token_t, tokens, i));
        }
    }
}


static inline 
void __preprocessor_skip_one_line__(preprocessor_t pp)
{
    for (;;) {
        token_t token = lexer_next(pp->lexer);
        if (token->type == TOKEN_NEWLINE || 
            token->type == TOKEN_END) {
            break;
        }
    }
}
