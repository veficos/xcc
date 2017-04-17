

#ifndef __TOKEN__H__
#define __TOKEN__H__


#include "config.h"
#include "array.h"
#include "set.h"
#include "reader.h"
#include "cstring.h"
#include "encoding.h"


typedef enum token_type_e {
    TOKEN_UNKNOWN = -1,
    TOKEN_END,
    
    TOKEN_L_SQUARE,                         /* [ */
    TOKEN_R_SQUARE,                         /* ] */
    TOKEN_L_PAREN,                          /* ) */
    TOKEN_R_PAREN,                          /* ( */
    TOKEN_L_BRACE,                          /* { */
    TOKEN_R_BRACE,                          /* } */
    TOKEN_PERIOD,                           /* . */
    TOKEN_ELLIPSIS,                         /* ... */
    TOKEN_AMP,                              /* & */
    TOKEN_AMPAMP,                           /* && */
    TOKEN_AMPEQUAL,                         /* &= */
    TOKEN_STAR,                             /* * */
    TOKEN_STAREQUAL,                        /* *= */
    TOKEN_PLUS,                             /* + */
    TOKEN_PLUSPLUS,                         /* ++ */
    TOKEN_PLUSEQUAL,                        /* += */
    TOKEN_MINUS,                            /* - */
    TOKEN_MINUSMINUS,                       /* -- */
    TOKEN_MINUSEQUAL,                       /* -= */
    TOKEN_ARROW,                            /* -> */
    TOKEN_TILDE,                            /* ~ */
    TOKEN_EXCLAIM,                          /* ! */
    TOKEN_EXCLAIMEQUAL,                     /* != */
    TOKEN_SLASH,                            /* / */
    TOKEN_SLASHEQUAL,                       /* /= */
    TOKEN_PERCENT,                          /* % */
    TOKEN_PERCENTEQUAL,                     /* %= */
    TOKEN_LESS,                             /* < */
    TOKEN_LESSLESS,                         /* << */
    TOKEN_LESSLESSEQUAL,                    /* <<= */
    TOKEN_LESSEQUAL,                        /* <= */
    TOKEN_GREATER,                          /* > */
    TOKEN_GREATERGREATER,                   /* >> */
    TOKEN_GREATEREQUAL,                     /* >= */
    TOKEN_GREATERGREATEREQUAL,              /* >>= */
    TOKEN_CARET,                            /* ^ */
    TOKEN_CARETEQUAL,                       /* ^= */
    TOKEN_PIPE,                             /* | */
    TOKEN_PIPEPIPE,                         /* || */
    TOKEN_PIPEEQUAL,                        /* |= */
    TOKEN_QUESTION,                         /* ? */
    TOKEN_COLON,                            /* : */
    TOKEN_SEMI,                             /* ; */
    TOKEN_EQUAL,                            /* = */
    TOKEN_EQUALEQUAL,                       /* == */
    TOKEN_COMMA,                            /* , */
    TOKEN_HASH,                             /* # */
    TOKEN_HASHHASH,                         /* ## */
    TOKEN_BACKSLASH,                        /* \ */
    TOKEN_NEW_LINE,                         /* \n */
    TOKEN_SPACE,                            /* spaces */
    TOKEN_COMMENT,                          /* comment */

    TOKEN_CONSTANT_STRING,                  /* "" */
    TOKEN_CONSTANT_WSTRING,                 /* L"" */
    TOKEN_CONSTANT_STRING16,                /* u"" */
    TOKEN_CONSTANT_STRING32,                /* U"" */
    TOKEN_CONSTANT_UTF8STRING,              /* u8"" */

    TOKEN_CONSTANT_CHAR,                    /* '' */
    TOKEN_CONSTANT_WCHAR,                   /* L'' */
    TOKEN_CONSTANT_CHAR16,                  /* u'' */
    TOKEN_CONSTANT_CHAR32,                  /* U'' */
    TOKEN_CONSTANT_UTF8CHAR,                /* u8'' */
    
    TOKEN_NUMBER,                           /* number */
    TOKEN_IDENTIFIER,                       /* identifier */

    TOKEN_CONST,
    TOKEN_RESTRICT,
    TOKEN_VOLATILE,
    TOKEN_ATOMIC,

    TOKEN_VOID,
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

    TOKEN_CONSTANT,
    TOKEN_ICONSTANT,
    TOKEN_CCONSTANT,
    TOKEN_FCONSTANT,
    TOKEN_LITERAL,

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


#define DEFUALT_LITERALS_LENGTH     12
#define DEFAULT_FILENAME_LENGTH     32
#define DEFAULT_CURRENTLINE_LENGTH  64


typedef struct source_location_s {
    size_t line;
    size_t column;
    linenote_t linenote;
    cstring_t fn;
} *source_location_t;


typedef struct token_s {
    token_type_t type;
    cstring_t cs;
    source_location_t loc;
    set_t hideset;          /* used by the preprocessor for macro expansion */
    bool begin_of_line;     /* true if the token is at the beginning of a line */
    size_t spaces;          /* >0 if the token has a leading space */
} *token_t;


#define token_mark_loc(tok, line, column, linenote, fn) \
    source_location_mark((tok)->loc, line, column, linenote, fn)

#define token_remark_loc(tok, line, column, linenote) \
    source_location_remark((tok)->loc, line, column, linenote)


token_t token_create(void);
void token_init(token_t token);
void token_destroy(token_t tok);
token_t token_dup(token_t tok);
const char *tok2id(token_t tok);


source_location_t source_location_create(void);
void source_location_destroy(source_location_t loc);
source_location_t source_location_dup(source_location_t loc);


static inline
void source_location_mark(source_location_t loc, 
    size_t line, size_t column, linenote_t linenote, cstring_t fn)
{
    loc->line = line;
    loc->column = column;
    loc->linenote = linenote;
    loc->fn = fn;
}


static inline
void source_location_remark(source_location_t loc, 
    size_t line, size_t column, linenote_t linenote)
{
    loc->line = line;
    loc->column = column;
    loc->linenote = linenote;
}


static inline
token_type_t ent2toktype(encoding_type_t ent)
{
    return ent == ENCODING_CHAR16 ? TOKEN_CONSTANT_STRING16 :
           ent == ENCODING_CHAR32 ? TOKEN_CONSTANT_STRING32 :
           ent == ENCODING_UTF8   ? TOKEN_CONSTANT_UTF8STRING :
           ent == ENCODING_WCHAR  ? TOKEN_CONSTANT_WSTRING : TOKEN_CONSTANT_STRING;
}


#endif
