

#ifndef __LEXER__H__
#define __LEXER__H__


#include "config.h"
#include "cstring.h"


typedef struct array_s     array_t;
typedef struct reader_s    reader_t;
typedef struct token_s     token_t;
typedef enum token_type_e  token_type_t;
typedef enum stream_type_e stream_type_t;


typedef struct lexer_s {
    reader_t *reader;
} lexer_t;


lexer_t* lexer_create(void);
lexer_t* lexer_create_csp(cspool_t *csp);
void lexer_destroy(lexer_t *lexer);
bool lexer_push(lexer_t *lexer, stream_type_t type, const unsigned char* s);
array_t* lexer_tokenize(lexer_t *lexer);
token_t* lexer_scan(lexer_t *lexer);
token_t* lexer_scan_header_name(lexer_t *lexer);
token_t* lexer_get(lexer_t *lexer);
token_t* lexer_peek(lexer_t *lexer);
void lexer_eat(lexer_t *lexer);
void lexer_unget(lexer_t *lexer, token_t *tok);
bool lexer_try(lexer_t *lexer, token_type_t tt);

void lexer_stash(lexer_t *lexer);
void lexer_unstash(lexer_t *lexer);
cstring_t lexer_date(lexer_t *lexer);
cstring_t lexer_time(lexer_t *lexer);


#endif
