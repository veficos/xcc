

#include "config.h"
#include "pmalloc.h"
#include "lexer.h"
#include "reader.h"

#include <assert.h>
#include <ctype.h>


static inline token_t __lexer_token_make__(lexer_t lexer, token_type_t type);
static inline void __lexer_token_mark_location__(lexer_t lexer);
static inline void __lexer_skip_white_space__(lexer_t lexer);
static inline void __lexer_skip_comment__(lexer_t lexer);
static inline void __lexer_error_with_line__(lexer_t lexer);


lexer_t lexer_create(reader_t reader, cstring_pool_t pool, option_t option)
{
    lexer_t lexer = pmalloc(sizeof(struct lexer_s));
    if (!lexer) {
        return NULL;
    }

    lexer->reader = reader;
    lexer->pool = pool;
    lexer->option = option;

    return lexer;
}


void lexer_destroy(lexer_t lexer)
{
    if (!lexer) {
        pfree(lexer);
    }
}


token_t lexer_scan(lexer_t lexer)
{
    token_t tok = NULL;
    int ch;

    token_init(&lexer->tok);

    __lexer_skip_white_space__(lexer);

    __lexer_token_mark_location__(lexer);

    if (reader_try(lexer->reader, '\n')) {
        return __lexer_token_make__(lexer, TOKEN_NEW_LINE);
    }

    ch = reader_get(lexer->reader);
    switch (ch) {
    case '[':
        return __lexer_token_make__(lexer, TOKEN_L_SQUARE);
    case ']':
        return __lexer_token_make__(lexer, TOKEN_R_SQUARE);
    case '(':
        return __lexer_token_make__(lexer, TOKEN_L_PAREN);
    case ')':
        return __lexer_token_make__(lexer, TOKEN_R_PAREN);
    case '{':
        return __lexer_token_make__(lexer, TOKEN_L_BRACE);
    case '}':
        return __lexer_token_make__(lexer, TOKEN_R_BRACE);
    case '.':
        if (isdigit(reader_peek(lexer->reader))) {
            /* TODO Number */
        }
        if (reader_try(lexer->reader, '.')) {
            if (reader_try(lexer->reader, '.')) 
                return __lexer_token_make__(lexer, TOKEN_ELLIPSIS);

            reader_unget(lexer->reader, '.');

            return __lexer_token_make__(lexer, TOKEN_PERIOD);
        }
        return __lexer_token_make__(lexer, TOKEN_PERIOD);
    case '#':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '#') ? TOKEN_HASHHASH : TOKEN_HASH);
    case ':': 
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '>') ? TOKEN_R_SQUARE : TOKEN_COLON);
    case '?': 
        return __lexer_token_make__(lexer, TOKEN_QUESTION);
    case ',': 
        return __lexer_token_make__(lexer, TOKEN_COMMA);
    case '~':
        return __lexer_token_make__(lexer, TOKEN_TILDE);
    case ';':
        return __lexer_token_make__(lexer, TOKEN_SEMI);
    case '-':
        if (reader_try(lexer->reader, '>')) return __lexer_token_make__(lexer, TOKEN_ARROW);
        if (reader_try(lexer->reader, '-')) return __lexer_token_make__(lexer, TOKEN_MINUSMINUS);
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_MINUSEQUAL);
        return __lexer_token_make__(lexer, TOKEN_MINUS);
    case '+':
        if (reader_try(lexer->reader, '+'))	return __lexer_token_make__(lexer, TOKEN_PLUSPLUS);
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_PLUSEQUAL);
        return __lexer_token_make__(lexer, TOKEN_PLUS);
    case '<':
        if (reader_try(lexer->reader, '<')) 
            return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_LESSLESSEQUAL : TOKEN_LESSLESS);
        if (reader_try(lexer->reader, '=')) 
            return __lexer_token_make__(lexer, TOKEN_LESSEQUAL);
        if (reader_try(lexer->reader, ':')) 
            return __lexer_token_make__(lexer, TOKEN_L_SQUARE);
        if (reader_try(lexer->reader, '%')) 
            return __lexer_token_make__(lexer, TOKEN_L_BRACE);
        return __lexer_token_make__(lexer, TOKEN_LESS);
    case '%':
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_PERCENTEQUAL);
        if (reader_try(lexer->reader, '>')) return __lexer_token_make__(lexer, TOKEN_R_BRACE);
        if (reader_try(lexer->reader, ':')) {
            if (reader_try(lexer->reader, '%')) {
                if (reader_try(lexer->reader, ':'))	
                    return __lexer_token_make__(lexer, TOKEN_HASHHASH);
                reader_unget(lexer->reader, '%');
            }
            return __lexer_token_make__(lexer, TOKEN_HASH);
        }
        return __lexer_token_make__(lexer, TOKEN_PERCENT);
    case '>':
        if (reader_try(lexer->reader, '>')) 
            return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_GREATERGREATEREQUAL : TOKEN_GREATERGREATER);
        if (reader_try(lexer->reader, '=')) 
            return __lexer_token_make__(lexer, TOKEN_GREATEREQUAL);
        return __lexer_token_make__(lexer, TOKEN_GREATER);
    case '=':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EQUALEQUAL : TOKEN_EQUAL);
    case '!': 
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EXCLAIM : TOKEN_EXCLAIMEQUAL);
    case '&':
        if (reader_try(lexer->reader, '&')) 
            return __lexer_token_make__(lexer, TOKEN_AMPAMP);
        if (reader_try(lexer->reader, '=')) 
            return __lexer_token_make__(lexer, TOKEN_AMPEQUAL);
        return __lexer_token_make__(lexer, TOKEN_AMP);
    case '|':
        if (reader_try(lexer->reader, '|'))
            return __lexer_token_make__(lexer, TOKEN_PIPEPIPE);
        if (reader_try(lexer->reader, '='))
            return __lexer_token_make__(lexer, TOKEN_PIPEEQUAL);
        return __lexer_token_make__(lexer, TOKEN_PIPE);
    case '*':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_STAREQUAL : TOKEN_STAR);
    case '/':
        if (reader_test(lexer->reader, '/') || reader_test(lexer->reader, '*')) {
            __lexer_skip_comment__(lexer);
            return lexer_scan(lexer);
        }
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH);
    }
    
    return tok;
}


static inline 
void __lexer_token_mark_location__(lexer_t lexer)
{
    const char *name = reader_name(lexer->reader);
    size_t n = strlen(name);

    source_location_init(&lexer->tok.location,
                         reader_line(lexer->reader),
                         reader_column(lexer->reader),
                         cstring_pool_push(lexer->pool, reader_current_line(lexer->reader)),
                         cstring_pool_push_n(lexer->pool, name, n));
}


static inline 
void __lexer_skip_white_space__(lexer_t lexer)
{
    while (isspace(reader_peek(lexer->reader)) && reader_peek(lexer->reader) != '\n') {
        (void) reader_get(lexer->reader);
    }
}


static inline 
void __lexer_skip_comment__(lexer_t lexer)
{
    int ch;

    if (reader_try(lexer->reader, '/')) {
        while ((ch = reader_peek(lexer->reader)) != EOF) {
            if (ch == '\n') {
                return ;
            }
            reader_get(lexer->reader);
        }
    } else if (reader_try(lexer->reader, '*')) {
        while ((ch = reader_get(lexer->reader)) != EOF) {
            if (ch == '*' && reader_try(lexer->reader, '/')) {
                return ;
            }
        }
    }

    assert(false);
}


static inline
void __lexer_error_with_line__(lexer_t lexer)
{

}


static inline 
token_t __lexer_token_make__(lexer_t lexer, token_type_t type)
{
    token_t tok;

    tok = token_dup(&lexer->tok);

    tok->type = type;

    return tok;
}
