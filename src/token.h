

#ifndef __TOKEN__H__
#define __TOKEN__H__


#include "config.h"
#include "cstring_pool.h"


typedef enum token_type_e {
    TOKEN_UNKNOWN = -1,
    TOKEN_END,
    TOKEN_IGNORE,
    
    TOKEN_L_SQUARE,         /* [ */
    TOKEN_R_SQUARE,         /* ] */
    TOKEN_L_PAREN,          /* ) */
    TOKEN_R_PAREN,          /* ( */
    TOKEN_L_BRACE,          /* { */
    TOKEN_R_BRACE,          /* } */
    TOKEN_PERIOD,           /* . */

    
    TOKEN_COLON,            /* : */
    TOKEN_COMMA,            /* , */
    TOKEN_SEMI,             /* ; */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_XOR,
    
    
    
    TOKEN_TILDE,                /* ~ */
    TOKEN_NOT,
    TOKEN_QUESTION,
    
    TOKEN_NEW_LINE,             /* \n */
    TOKEN_ELLIPSIS,             /* ... */

    TOKEN_PERCENT,              /* % */
    TOKEN_PERCENTEQUAL,         /* %= */

    TOKEN_LESS,                 /* < */
    TOKEN_LESSLESS,             /* << */
    TOKEN_LESSLESSEQUAL,        /* <<= */
    TOKEN_LESSEQUAL,            /* <= */
    TOKEN_GREATER,              /* > */
    TOKEN_GREATERGREATER,       /* >> */
    TOKEN_GREATEREQUAL,         /* >= */
    TOKEN_GREATERGREATEREQUAL,  /* >>= */

    TOKEN_EXCLAIM,              /* ! */
    TOKEN_EXCLAIMEQUAL,         /* != */

    TOKEN_EQUAL,                /* = */
    TOKEN_EQUALEQUAL,           /* == */

    TOKEN_AMP,                  /* & */
    TOKEN_AMPAMP,               /* && */
    TOKEN_AMPEQUAL,             /* &= */
    TOKEN_PIPE,                 /* | */
    TOKEN_PIPEPIPE,             /* || */
    TOKEN_PIPEEQUAL,            /* |= */
    TOKEN_STAR,                 /* * */
    TOKEN_STAREQUAL,            /* *= */
    TOKEN_SLASH,                /* / */
    TOKEN_SLASHEQUAL,           /* /= */

    TOKEN_PLUSPLUS,         /* ++ */
    TOKEN_PLUSEQUAL,        /* += */
    TOKEN_MINUSMINUS,       /* -- */
    TOKEN_MINUSEQUAL,       /* -= */
    TOKEN_ARROW,            /* -> */

    TOKEN_HASH,             /* # */
    TOKEN_HASHHASH,         /* ## */

    TOKEN_PTR,
    TOKEN_INC,
    TOKEN_DEC,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,

    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_LEFT_ASSIGN,
    TOKEN_RIGHT_ASSIGN,
    TOKEN_AND_ASSIGN,
    TOKEN_XOR_ASSIGN,
    TOKEN_OR_ASSIGN,

    TOKEN_CONST,
    TOKEN_RESTRICT,
    TOKEN_VOLATILE,
    TOKEN_ATOMIC,

    TOKEN_VOID,
    TOKEN_CHAR,
    TOKEN_SHORT,
    TOKEN_INT,
    TOKEN_LONG,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_SIGNED,
    TOKEN_UNSIGNED,
    TOKEN_BOOL,
    TOKEN_COMPLEX,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,

    TOKEN_ATTRIBUTE,

    TOKEN_INLINE,
    TOKEN_NORETURN,

    TOKEN_ALIGNAS,

    TOKEN_STATIC_ASSERT,
    TOKEN_TYPEDEF,
    TOKEN_EXTERN,
    TOKEN_STATIC,
    TOKEN_THREAD,
    TOKEN_AUTO,
    TOKEN_REGISTER,

    TOKEN_BREAK,
    TOKEN_CASE,
    TOKEN_CONTINUE,
    TOKEN_DEFAULT,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_GOTO,
    TOKEN_IF,
    TOKEN_RETURN,
    TOKEN_SIZEOF,
    TOKEN_SWITCH,
    TOKEN_WHILE,
    TOKEN_ALIGNOF,
    TOKEN_GENERIC,
    TOKEN_IMAGINARY,

    TOKEN_IDENTIFIER,
    TOKEN_CONSTANT,
    TOKEN_ICONSTANT,
    TOKEN_CCONSTANT,
    TOKEN_FCONSTANT,
    TOKEN_LITERAL,

    TOKEN_POSTFIX_INC,
    TOKEN_POSTFIX_DEC,
    TOKEN_PREFIX_INC,
    TOKEN_PREFIX_DEC,
    
    TOKEN_DEREF,
    TOKEN_CAST,

    TOKEN_PP_IF,
    TOKEN_PP_IFDEF,
    TOKEN_PP_IFNDEF,
    TOKEN_PP_ELIF,
    TOKEN_PP_ELSE,
    TOKEN_PP_ENDIF,
    TOKEN_PP_INCLUDE,
    TOKEN_PP_DEFINE,
    TOKEN_PP_UNDEF,
    TOKEN_PP_LINE,
    TOKEN_PP_ERROR,
    TOKEN_PP_PRAGMA,
    TOKEN_PP_NONE,
    TOKEN_PP_EMPTY,

} token_type_t;


typedef struct source_location_s {
    size_t line;
    size_t column;
    cstring_t current_line;
    cstring_t filename;
} *source_location_t;


typedef struct token_s {
    token_type_t type;
    cstring_t literal;
    struct source_location_s location;
} *token_t;


token_t token_dup(token_t tok);

static inline
void token_init(token_t token)
{
    token->type = TOKEN_UNKNOWN;
    token->location.line = 0;
    token->location.column = 0;
    token->location.current_line = NULL;
    token->location.filename = NULL;
}


static inline
void source_location_init(source_location_t sl, size_t line, size_t column, cstring_t current_line, cstring_t filename)
{
    sl->line = line;
    sl->column = column;
    sl->current_line = current_line;
    sl->filename = filename;
}


#endif
