

#ifndef __LEXER__H__
#define __LEXER__H__


#include "config.h"
#include "cstring.h"


typedef struct array_s*     array_t;
typedef struct option_s*    option_t;
typedef struct reader_s*    reader_t;
typedef struct diag_s*      diag_t;
typedef struct token_s*     token_t;
typedef enum token_type_e   token_type_t;


typedef struct lexer_s {
    option_t option;
    reader_t reader;
    diag_t diag;
    token_t tok;        /* current token */
    array_t snapshots;
    struct tm tm;
} *lexer_t;


lexer_t lexer_create(reader_t reader, option_t option, diag_t diag);
void lexer_destroy(lexer_t lexer);
token_t lexer_scan(lexer_t lexer);
token_t lexer_next(lexer_t lexer);
token_t lexer_peek(lexer_t lexer);
bool lexer_try(lexer_t lexer, token_type_t tt);
bool lexer_untread(lexer_t lexer, token_t tok);

bool lexer_stash(lexer_t lexer);
void lexer_unstash(lexer_t lexer);
cstring_t lexer_date(lexer_t lexer);
cstring_t lexer_time(lexer_t lexer);


#endif
