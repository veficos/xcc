

#ifndef __PREPROCESSOR__H__
#define __PREPROCESSOR__H__


#include "config.h"


typedef struct array_s* array_t;
typedef struct token_s* token_t;
typedef struct lexer_s* lexer_t;


typedef enum macro_type_e {
    PP_MACRO_OBJECT,
    PP_MACRO_FUNCTION,
    PP_MACRO_VARIADIC,
    PP_MACRO_PREDEF,
} macro_type_t;


typedef struct macro_s {
    macro_type_t type;
} *macro_t;


typedef struct preprocessor_s {
    array_t std_include_paths;

    lexer_t lexer;

} *preprocessor_t;


preprocessor_t preprocessor_create(lexer_t lexer);
void preprocessor_destroy(preprocessor_t pp);
bool preprocessor_add_include_path(preprocessor_t pp, const char *path);
token_t preprocessor_peek(preprocessor_t pp);
token_t preprocessor_get(preprocessor_t pp);
token_t preprocessor_unget(preprocessor_t pp);


#endif
