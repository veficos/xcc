

#ifndef __LEXER__H__
#define __LEXER__H__


#include "config.h"
#include "token.h"


typedef struct option_s* option_t;
typedef struct reader_s* reader_t;
typedef struct cstring_pool_s* cstring_pool_t;
typedef struct diag_s* diag_t;


typedef struct lexer_s {
    option_t option;
    reader_t reader;
    cstring_pool_t pool;
    diag_t diag;
    struct token_s tok;
}* lexer_t;


lexer_t lexer_create(reader_t reader, cstring_pool_t pool, option_t option);
void lexer_destroy(lexer_t lexer);
token_t lexer_scan(lexer_t lexer);


#endif
