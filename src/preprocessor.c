

#include "config.h"
#include "array.h"
#include "cstring.h"
#include "pmalloc.h"
#include "token.h"
#include "lexer.h"
#include "diagnostor.h"
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


static token_t __preprocessor_expand__(preprocessor_t pp);
static bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash);
static inline array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, map_t args, set_t hideset);
static inline void __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens);
static inline bool __preprocessor_parse_define__(preprocessor_t pp);
static inline bool __preprocessor_predefined_std_include_paths__(preprocessor_t pp);


static inline
void __preprocessor_add_macro__(preprocessor_t pp, token_t macroname_token,
    macro_type_t type, native_macro_pt native_macro_fn,
    array_t body, array_t params, bool is_variadic);
static inline
macro_t __macro_create__(macro_type_t type, token_t macroname_token, 
    native_macro_pt native_macro_fn, array_t body, array_t params, bool is_variadic);
static inline
void __macro_destroy__(macro_t macro);

static array_t __create_tokens__(void);
static void __destroy_tokens__(array_t a);


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
    __macro_destroy__(macro);
}


void preprocessor_destroy(preprocessor_t pp)
{
    cstring_t *std_include_paths;
    size_t i;

    array_foreach(pp->std_include_paths, std_include_paths, i) {
        cstring_free(std_include_paths[i]);
    }

    array_destroy(pp->std_include_paths);
    
    map_scan(pp->macros, map_scan_fn, NULL);

    map_destroy(pp->macros);

    pfree(pp);
}


void preprocessor_add_include_path(preprocessor_t pp, const char *path)
{
    array_cast_append(cstring_t, pp->std_include_paths, cstring_new(path));
}


token_t preprocessor_expand(preprocessor_t pp)
{
    for (;;) {
        token_t tok = __preprocessor_expand__(pp);
        if (__preprocessor_parse_directive__(pp, tok)) {
            continue;
        }
        return tok;
    }
}


token_t preprocessor_peek(preprocessor_t pp)
{
    token_t tok = preprocessor_get(pp);
    if (tok->type != TOKEN_END) preprocessor_unget(pp, tok);
    return tok;
}


token_t preprocessor_get(preprocessor_t pp)
{
    for (;;) {
        token_t tok = preprocessor_expand(pp);
        if (tok->type == TOKEN_NEWLINE) {
            continue;
        }
        return tok;
    }
}


void preprocessor_unget(preprocessor_t pp, token_t tok)
{
    assert(tok && tok->type != TOKEN_END);
    lexer_unget(pp->lexer, tok);
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
        token_t t = array_cast_front(token_t, expand_tokens);
        t->spaces = token->spaces;
    }
}


static inline
void __preprocessor_expand_object_macro__(preprocessor_t pp, token_t token, macro_t macro)
{
    array_t expand_tokens;
    set_t hideset;

    hideset = token->hideset ? set_dup(token->hideset) : set_create();

    set_add(hideset, token->cs);

    expand_tokens = __preprocessor_substitute__(pp, macro, NULL, hideset);

    __propagate_space__(expand_tokens, token);

    __preprocessor_unget_tokens__(pp, expand_tokens);

    set_destroy(hideset);

    token_destroy(token);

    array_destroy(expand_tokens);
}


static
array_t __preprocessor_parse_function_like_argument__(preprocessor_t pp, bool is_vararg)
{
    array_t arg = __create_tokens__();
    size_t level = 0;

    for (; !lexer_is_eos(pp->lexer); ) {
        token_t token = lexer_peek(pp->lexer);
        if (((token->type == TOKEN_R_PAREN) || 
             (token->type == TOKEN_COMMA && is_vararg == false)) && level == 0) {
            break;
        }

        if (token->type == TOKEN_L_PAREN) {
            level++;
        }

        if (token->type == TOKEN_R_PAREN) {
            level--;
        }

        array_cast_append(token_t, arg, token);
        lexer_get(pp->lexer);
    }

    return arg;
}


static
bool __preprocessor_parse_function_like_arguments__(preprocessor_t pp, 
    token_t macroname_token, macro_t macro, map_t args)
{
    token_t separator;
    token_t *param_tokens;
    size_t i, nparams;
    
    nparams = array_length(macro->function_like.params);
    param_tokens = array_prototype(macro->function_like.params, token_t);
    for (i = 0; !lexer_is_eos(pp->lexer); i++) {
        if (i < nparams) {
            array_t arg = __preprocessor_parse_function_like_argument__(pp,
                param_tokens[i]->is_vararg);
            map_add(args, param_tokens[i]->cs, arg);
        } else {
            array_t arg = __preprocessor_parse_function_like_argument__(pp,
                false);
            __destroy_tokens__(arg);
        }
        
        separator = lexer_peek(pp->lexer);
        if (separator->type == TOKEN_R_PAREN) {
            break;
        }

        if (separator->type != TOKEN_COMMA) {
            ERRORF_WITH_TOKEN(macroname_token,
                "unterminated argument list invoking macro \"%s\"", token_as_text(macroname_token));
            return false;
        }

        lexer_eat(pp->lexer);
    }

    return true;
}


static
bool __preprocessor_expand_function_macro__(preprocessor_t pp, token_t token, macro_t macro)
{
    map_t args = map_create();
    token_t r_paren_token;
    array_t expand_tokens;
    set_t hideset;

    if (!lexer_try(pp->lexer, TOKEN_L_PAREN)) {
        return false;
    }
   
    if (!__preprocessor_parse_function_like_arguments__(pp, token, macro, args)) {
        return false;
    }

    r_paren_token = lexer_peek(pp->lexer);
    if (r_paren_token->type != TOKEN_R_PAREN) {
        ERRORF_WITH_TOKEN(token,
            "unterminated argument list invoking macro \"%s\"", token_as_text(token));
        return false;
    }
    lexer_get(pp->lexer);

    hideset = token->hideset ? set_dup(token->hideset) : set_create();

    if (r_paren_token->hideset != NULL) {
        hideset = set_intersection(hideset, r_paren_token->hideset);
    }

    set_add(hideset, token->cs);
    
    expand_tokens = __create_tokens__();//__preprocessor_substitute__(pp, macro, args, hideset);

    __propagate_space__(expand_tokens, token);

    __preprocessor_unget_tokens__(pp, expand_tokens);

    set_destroy(hideset);

    token_destroy(token);

    array_destroy(expand_tokens);

    return true;
}


static 
token_t __preprocessor_expand__(preprocessor_t pp)
{
    token_t token;
    macro_t macro;

    for (;;) {
        token = lexer_get(pp->lexer);

        if ((token->type != TOKEN_IDENTIFIER) || 
            (token->type == TOKEN_NEWLINE) || 
            (token->hideset && set_has(token->hideset, token->cs)) || 
            ((macro = map_find(pp->macros, token->cs)) == NULL)) {
            return token;
        }
   
        if (macro->type == PP_MACRO_OBJECT) {
            __preprocessor_expand_object_macro__(pp, token, macro);
            break;
        } else if (macro->type == PP_MACRO_FUNCTION) {
            if (!__preprocessor_expand_function_macro__(pp, token, macro)) {
                return token;
            }
            break;
        } else if (macro->type == PP_MACRO_NATIVE) {
            macro->native_macro_fn(token);
            break;
        } else {
            assert(false);
        }
    }

    return __preprocessor_expand__(pp);
}


static inline 
array_t __preprocessor_substitute_object_like__(preprocessor_t pp, array_t macro_body)
{
    array_t expand_tokens;
    token_t *macro_tokens;
    size_t i;
    
    expand_tokens = __create_tokens__();

    array_foreach(macro_body, macro_tokens, i) {
        token_t token = token_dup(macro_tokens[i]);
        array_cast_append(token_t, expand_tokens, token);
    }
   
    return expand_tokens;
}


static
bool __add_hide_set__(set_t hideset, array_t expand_tokens)
{
    token_t *tokens;
    size_t i;

    array_foreach(expand_tokens, tokens, i) {
        set_t token_hs = tokens[i]->hideset;
        if (token_hs != NULL) {
            tokens[i]->hideset = set_union(hideset, token_hs);
            set_destroy(token_hs);
        } else {
            tokens[i]->hideset = set_dup(hideset);
        }
    }

    return true;
}


/**
* Select an argument for expansion.
*/
static inline
array_t __preprocessor_select__(map_t args, token_t index)
{
    array_t arg;

    if (index->type != TOKEN_IDENTIFIER) {
        return NULL;
    }

    arg = map_find(args, index->cs);
    if (arg != NULL) {
        array_t replacements;
        size_t i, n;

        replacements = array_create_n(sizeof(token_t), 2);

        for (i = 0, n = array_length(arg); i < n; i++) {
            token_t token = array_cast_at(token_t, arg, i);
            array_cast_append(token_t, replacements, token);
        }

        __propagate_space__(replacements, index);
        return replacements;
    }

    return NULL;
}


static inline
token_t __preprocessor_stringify__(preprocessor_t pp, token_t template, array_t arg)
{
    token_t dst;
    cstring_t cs = cstring_new_n(NULL, 24);
    token_t *tokens;
    size_t i, spaces;

    array_foreach(arg, tokens, i) {
        spaces = tokens[i]->spaces;
        while (spaces--) {
            cs = cstring_concat_ch(cs, ' ');
        }
        cs = cstring_concat_n(cs, token_as_text(tokens[i]), strlen(token_as_text(tokens[i])));
    }

    dst = token_dup(template);
    if (dst->cs) {
        cstring_free(dst->cs);
    }

    dst->type = TOKEN_CONSTANT_STRING;
    dst->cs = cs;
    return dst;
}


static inline 
array_t __preprocessor_glue_token__(preprocessor_t pp, token_t left, token_t right)
{
    cstring_t cs;
    cs = cstring_new(token_as_text(left));
    cs = cstring_concat_n(cs, token_as_text(right), strlen(token_as_text(right)));
    lexer_push(pp->lexer, STREAM_TYPE_STRING, cs);
    return lexer_tokenize(pp->lexer);
}


static inline 
void __preprocessor_glue__(preprocessor_t pp, array_t expand_tokens, token_t token)
{
    token_t last;
    array_t glue_token;

    last = array_cast_back(token_t, expand_tokens);

    array_pop_back(expand_tokens);

    glue_token = __preprocessor_glue_token__(pp, last, token);

    array_extend(expand_tokens, glue_token);
}


static inline 
array_t __preprocessor_substitute_function_like__(preprocessor_t pp, bool is_variadic, 
    array_t macro_body, map_t args)
{
    array_t expand_tokens;
    size_t i, n;

    expand_tokens = array_create_n(sizeof(token_t), 8);

    n = array_length(macro_body);

    for (i = 0; i < n; i++) {
        token_t token;
        
        token = array_cast_at(token_t, macro_body, i);
        if (token->type == TOKEN_HASH) {
            token_t stringify = array_cast_at(token_t, macro_body, ++i);
            array_t replacements = __preprocessor_select__(args, stringify);

            array_cast_append(token_t, expand_tokens, __preprocessor_stringify__(pp, token, replacements));
            continue;
        } else if (token->type == TOKEN_HASHHASH) {
            token_t stringify = array_cast_at(token_t, macro_body, ++i);
            array_t replacements = __preprocessor_select__(args, stringify);
            if (replacements == NULL) {
                __preprocessor_glue__(pp, expand_tokens, stringify);

            } else {
                size_t j, n;

                if (array_length(replacements) == 0) {
                    i++;
                    continue;
                } else {
                    stringify = array_cast_front(token_t, replacements);
                    __preprocessor_glue__(pp, expand_tokens, stringify);

                    for (j = 1, n = array_length(replacements); j < n; j++) {
                        array_cast_append(token_t, expand_tokens, array_cast_at(token_t, replacements, j));
                    }
                    continue;
                }
            }
            
        } else {
            array_t replacements = __preprocessor_select__(args, token);
            if (cstring_compare(token->cs, "c") == 0) {
                printf("");
            }
            if (replacements != NULL) {
                if (i + 1 < n && array_cast_at(token_t, macro_body, i + 1)->type == TOKEN_HASHHASH) {
                    if (array_length(replacements) == 0) {
                        i++;
                    } else {
                        array_extend(expand_tokens, replacements);
                    }
                } else {
                    array_extend(expand_tokens, replacements);
                }
                continue;
            } 
        }

        token = token_dup(token);
        array_cast_append(token_t, expand_tokens, token);
    }

    return expand_tokens;
}


static inline 
array_t __preprocessor_substitute__(preprocessor_t pp, macro_t macro, map_t args, set_t hideset)
{
    array_t expand_tokens;

    switch (macro->type) {
    case PP_MACRO_OBJECT:
        expand_tokens = __preprocessor_substitute_object_like__(pp, 
            macro->object_like.body);
        break;
    case PP_MACRO_FUNCTION:
        expand_tokens = __preprocessor_substitute_function_like__(pp,
            macro->function_like.is_variadic, macro->function_like.body, args);
        break;
    default:
        assert(false);
    }

    __add_hide_set__(hideset, expand_tokens);

    return expand_tokens;
}


static
bool __preprocessor_check_macro_body__(preprocessor_t pp, array_t body)
{
    token_t token;

    if (array_is_empty(body)) {
        return true;
    }

    token = array_cast_front(token_t, body);
    if (token->type == TOKEN_HASHHASH) {
        ERRORF_WITH_TOKEN(token, "'##' cannot appear at start of macro expansion");
        return false;
    }

    token = array_cast_back(token_t, body);
    if (token->type == TOKEN_HASHHASH) {
        ERRORF_WITH_TOKEN(token, "'##' cannot appear at end of macro expansion");
        return false;
    }

    return true;
}


static
void __preprocessor_skip_one_line__(preprocessor_t pp)
{
    for (; !lexer_is_eos(pp->lexer); ) {
        token_t token = lexer_get(pp->lexer);
        if (token->type == TOKEN_NEWLINE) {
            token_destroy(token);
            break;
        }
        token_destroy(token);
    }
}


static
bool __preprocessor_parse_object_like__(preprocessor_t pp, token_t macroname_token)
{
    array_t macro_body;

    macro_body = array_create_n(sizeof(token_t), 8);

    for (;;) {
        token_t token = lexer_peek(pp->lexer);
        if (token->type == TOKEN_NEWLINE) {
            break;
        }
        lexer_get(pp->lexer);
        array_cast_append(token_t, macro_body, token);
    }

    if (!__preprocessor_check_macro_body__(pp, macro_body)) {
        token_destroy(macroname_token);
        __destroy_tokens__(macro_body);
        __preprocessor_skip_one_line__(pp);
        return false;
    }

    __preprocessor_add_macro__(pp, macroname_token,
        PP_MACRO_OBJECT, NULL, macro_body, NULL, false);

    return true;
}


static
bool __preprocessor_add_function_like_param__(preprocessor_t pp,
    array_t params, token_t identifier_token)
{
    token_t *tokens;
    size_t i;

    array_foreach(params, tokens, i) {
        if (cstring_compare(tokens[i]->cs, identifier_token->cs) == 0) {
            ERRORF_WITH_TOKEN(identifier_token,
                "duplicate macro parameter \"%s\"", token_as_text(identifier_token));
            return false;
        }
    }

    array_cast_append(token_t, params, identifier_token);
    return true;
}


static
bool __preprocessor_parse_function_like_params__(preprocessor_t pp, array_t params, bool *variadic)
{
    token_t token;
    bool prev_ident = false;

    for (;;) {
        token = lexer_peek(pp->lexer);
        switch (token->type) {
        case TOKEN_IDENTIFIER:
            if (prev_ident == true) {
                ERRORF_WITH_TOKEN(token, "macro parameters must be comma-separated");
                return false;
            }

            prev_ident = true;

            if (!__preprocessor_add_function_like_param__(pp, params, token)) {
                return false;
            }

            lexer_get(pp->lexer);
            continue;
        case TOKEN_R_PAREN:
            if (prev_ident == true || array_is_empty(params)) {
                lexer_eat(pp->lexer);
                return true;
            }
        case TOKEN_COMMA:
            if (prev_ident == false) {
                ERRORF_WITH_TOKEN(token, "parameter name missing");
                return false;
            }
            lexer_eat(pp->lexer);
            prev_ident = false;
            continue;
        case TOKEN_ELLIPSIS:
            *variadic = true;

            if (prev_ident == false) {
                /* anonymous variadic macros */
                const char *va_args = "__VA_ARGS__";
                const size_t n_va_args = 11;
                token = token_dup(token);
                token->type = TOKEN_IDENTIFIER;
                token->cs = cstring_copy_n(token->cs, va_args, n_va_args);
                token->is_vararg = true;
                if (!__preprocessor_add_function_like_param__(pp, params, token)) {
                    return false;
                }

            } else {
                /* named variadic macros */
                array_cast_back(token_t, params)->is_vararg = true;
            }

            lexer_eat(pp->lexer);

            if (lexer_try(pp->lexer, TOKEN_R_PAREN)) {
                return true;
            }
        case TOKEN_NEWLINE:
        case TOKEN_END:
            ERRORF_WITH_TOKEN(token, "missing ')' in macro parameter list");
            return false;
        default:
            ERRORF_WITH_TOKEN(token, 
                "\"%s\" may not appear in macro parameter list", token_as_text(token));
            return false;
        }
    }

    return false;
}


static
bool __preprocessor_parse_function_like_body__(preprocessor_t pp, array_t macro_body)
{
    for (; !lexer_is_empty(pp->lexer); ) {
        token_t token = lexer_peek(pp->lexer);
        if (token->type == TOKEN_NEWLINE) {
            lexer_eat(pp->lexer);
            return true;
        }

        array_cast_append(token_t, macro_body, token);
        lexer_get(pp->lexer);
    }

    if (!__preprocessor_check_macro_body__(pp, macro_body)) {
        __destroy_tokens__(macro_body);
        return false;
    }

    return false;
}


static
bool __preprocessor_parse_function_like__(preprocessor_t pp, token_t macroname_token)
{
    array_t macro_params;
    array_t macro_body;
    bool is_variadic = false;

    /* eat '(' */
    lexer_eat(pp->lexer);

    macro_params = __create_tokens__();
    if (!__preprocessor_parse_function_like_params__(pp, macro_params, &is_variadic)) {
        assert((is_variadic == false) || (is_variadic == true && !array_is_empty(macro_params)));
        __preprocessor_skip_one_line__(pp);
        __destroy_tokens__(macro_params);
        token_destroy(macroname_token);
        return false;
    }

    macro_body = __create_tokens__();
    if (!__preprocessor_parse_function_like_body__(pp, macro_body)) {
        __preprocessor_skip_one_line__(pp);
        __destroy_tokens__(macro_params);
        __destroy_tokens__(macro_body);
        token_destroy(macroname_token);
        return false;
    }

    __preprocessor_add_macro__(pp, macroname_token,
        PP_MACRO_FUNCTION, NULL, macro_body, macro_params, is_variadic);

    return true;
}


static
bool __preprocessor_parse_define__(preprocessor_t pp)
{
    token_t macroname_token;
    token_t l_paren_token;

    macroname_token = lexer_get(pp->lexer);
    if (macroname_token->type != TOKEN_IDENTIFIER) {
        ERRORF_WITH_TOKEN(macroname_token, "macro names must be identifiers");
        token_destroy(macroname_token);
        __preprocessor_skip_one_line__(pp);
        return false;
    }

    l_paren_token = lexer_peek(pp->lexer);
    if (l_paren_token->type == TOKEN_L_PAREN && l_paren_token->spaces == 0) {
        return __preprocessor_parse_function_like__(pp, macroname_token);
    }

    return __preprocessor_parse_object_like__(pp, macroname_token);
}


static
bool __preprocessor_parse_directive__(preprocessor_t pp, token_t hash)
{
    if (hash->begin_of_line && 
        hash->type == TOKEN_HASH && 
        (hash->hideset == NULL || set_is_empty(hash->hideset))) {
        token_t directive_token;

        directive_token = lexer_get(pp->lexer);

        if (directive_token->type == TOKEN_NEWLINE) {
            token_destroy(hash);
            return false;
        }

        if (directive_token->type == TOKEN_NUMBER) {
            /* TODO: GNU remark linenum */
            return false;
        }

        if (cstring_compare(directive_token->cs, "define") == 0) {
            __preprocessor_parse_define__(pp);
        }

        token_destroy(hash);
        token_destroy(directive_token);
        return true;
    }

    return false;
}


static inline 
void __preprocessor_unget_tokens__(preprocessor_t pp, array_t tokens)
{
    size_t i;
    if (tokens != NULL) {
        for (i = array_length(tokens); i--; ) {
            lexer_unget(pp->lexer, array_cast_at(token_t, tokens, i));
        }
    }
}


static inline
void __preprocessor_add_macro__(preprocessor_t pp, token_t macroname_token,
    macro_type_t type, native_macro_pt native_macro_fn,
    array_t body, array_t params, bool is_variadic)
{
    macro_t macro;

    if (map_has(pp->macros, macroname_token->cs)) {
        WARNINGF_WITH_TOKEN(macroname_token, "\"%s\" redefined", macroname_token->cs);
        __macro_destroy__(map_find(pp->macros, macroname_token->cs));
        map_del(pp->macros, macroname_token->cs);
    }

    macro = __macro_create__(type, macroname_token, native_macro_fn, body, params, is_variadic);

    map_add(pp->macros, macroname_token->cs, macro);
}


static inline
macro_t __macro_create__(macro_type_t type, token_t macroname_token,
    native_macro_pt native_macro_fn, array_t body, array_t params, bool is_variadic)
{
    macro_t macro = (macro_t) pmalloc(sizeof(struct macro_s));

    switch (type) {
    case PP_MACRO_OBJECT: {
        macro->object_like.body = body;
        break;
    }
    case PP_MACRO_FUNCTION: {
        macro->function_like.is_variadic = is_variadic;
        macro->function_like.params = params;
        macro->function_like.body = body;
        break;
    }
    case PP_MACRO_NATIVE: {
        macro->native_macro_fn = native_macro_fn;
        break;
    }
    default:
        assert(false);
    }

    macro->name_token = macroname_token;
    macro->type = type;
    return macro;
}


static inline
void __macro_destroy__(macro_t macro)
{
    token_t *tokens;
    size_t i;

    switch (macro->type) {
    case PP_MACRO_OBJECT: {
        array_foreach(macro->object_like.body, tokens, i) {
            token_destroy(tokens[i]);
        }
        array_destroy(macro->object_like.body);

        break;
    }
    case PP_MACRO_FUNCTION: {
        array_foreach(macro->function_like.body, tokens, i) {
            token_destroy(tokens[i]);
        }
        array_destroy(macro->function_like.body);

        array_foreach(macro->function_like.params, tokens, i) {
            token_destroy(tokens[i]);
        }

        array_destroy(macro->function_like.params);

        break;
    }
    case PP_MACRO_NATIVE: {
        break;
    }
    default:
        assert(false);
    }

    if (macro->name_token != NULL) {
        token_destroy(macro->name_token);
    }
    pfree(macro);
}


static
array_t __create_tokens__(void)
{
    return array_create_n(sizeof(token_t), 8);
}


static
void __destroy_tokens__(array_t a)
{
    token_t *tokens;
    size_t i;

    array_foreach(a, tokens, i) {
        token_destroy(tokens[i]);
    }

    array_destroy(a);
}
