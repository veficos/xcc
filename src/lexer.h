

#ifndef __LEXER__H__
#define __LEXER__H__


#include "config.h"
#include "reader.h"
#include "token.h"
#include "option.h"
#include "cstring_pool.h"


typedef struct lexer_s {
    reader_t reader;
    cstring_pool_t pool;
    option_t option;
    struct token_s tok;
}* lexer_t;


lexer_t lexer_create(reader_t reader, cstring_pool_t pool, option_t option);
void lexer_destroy(lexer_t lexer);
token_t lexer_scan(lexer_t lexer);


#endif
