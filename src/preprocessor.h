

#ifndef __PREPROCESSOR__H__
#define __PREPROCESSOR__H__


#include "config.h"
#include "map.h"


typedef struct array_s*     array_t;
typedef struct token_s*     token_t;
typedef struct lexer_s*     lexer_t;
typedef struct dict_s*      dict_t;
typedef struct option_s*    option_t;
typedef struct reader_s*    reader_t;


typedef enum macro_type_e {
    PP_MACRO_OBJECT,
    PP_MACRO_FUNCTION,
    PP_MACRO_VARIADIC,
    PP_MACRO_PREDEF,
} macro_type_t;


typedef bool (*special_macro_pt) (token_t tok);


typedef struct macro_s {
    macro_type_t type;

    union {
        array_t object;
        special_macro_pt special_macro;
    };
} *macro_t;


typedef struct condition_directive_s {
    bool condiction;
} *condition_directive_t;


typedef struct preprocessor_s {
    array_t std_include_paths;
    array_t snapshot;
    lexer_t lexer;
    map_t macros;
} *preprocessor_t;


preprocessor_t preprocessor_create(lexer_t lexer, option_t option, diag_t diag);
void preprocessor_destroy(preprocessor_t pp);
bool preprocessor_add_include_path(preprocessor_t pp, const char *path);
token_t preprocessor_peek(preprocessor_t pp);
token_t preprocessor_next(preprocessor_t pp);
bool preprocessor_untread(preprocessor_t pp, token_t tok);


#endif
