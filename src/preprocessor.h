

#ifndef __PREPROCESSOR__H__
#define __PREPROCESSOR__H__


#include "config.h"
#include "map.h"
#include "set.h"


typedef struct array_s*     array_t;
typedef struct token_s*     token_t;
typedef struct lexer_s*     lexer_t;
typedef struct dict_s*      dict_t;
typedef struct option_s*    option_t;
typedef struct reader_s*    reader_t;


typedef enum macro_type_e {
    PP_MACRO_OBJECT,
    PP_MACRO_FUNCTION,
    PP_MACRO_NATIVE,
} macro_type_t;


typedef bool (*native_macro_pt) (token_t tok);


typedef struct macro_s {
    macro_type_t type;

    union {
        struct {
            array_t body;
        } object_like;

        struct {
            array_t body;
            array_t params;
            bool is_variadic;
        } function_like;

        native_macro_pt native_macro_fn;
    };

} *macro_t;


typedef struct condition_directive_s {
    bool condiction;
} *condition_directive_t;


typedef struct preprocessor_s {
    array_t std_include_paths;
   
    array_t condition_directive_stack;
  
    array_t snapshot;
    lexer_t lexer;

    map_t macros;
    set_t include_guard;
    set_t once_guard;

    array_t backup;
    diag_t diag;
} *preprocessor_t;


preprocessor_t preprocessor_create(lexer_t lexer, option_t option, diag_t diag);
void preprocessor_destroy(preprocessor_t pp);
void preprocessor_add_include_path(preprocessor_t pp, const char *path);
token_t preprocessor_expand(preprocessor_t pp);
token_t preprocessor_peek(preprocessor_t pp);
token_t preprocessor_next(preprocessor_t pp);
void preprocessor_unget(preprocessor_t pp, token_t tok);


#endif
