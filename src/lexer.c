

#include "config.h"
#include "pmalloc.h"
#include "token.h"
#include "reader.h"
#include "diag.h"
#include "lexer.h"
#include "encoding.h"



static inline void __lexer_errorf_location__(lexer_t lexer, const char *fmt, ...);
static inline void __lexer_warningf_location__(lexer_t lexer, const char *fmt, ...);

static inline token_t __lexer_token_make__(lexer_t lexer, token_type_t type);
static inline void __lexer_token_mark_location__(lexer_t lexer);
static inline void __lexer_skip_white_space__(lexer_t lexer);
static inline void __lexer_skip_comment__(lexer_t lexer);
static inline token_t __lexer_parse_number__(lexer_t lexer);
static inline token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent);
static inline token_t __lexer_parse_string__(lexer_t lexer, encoding_type_t ent);
static inline int __lexer_parse_escaped__(lexer_t lexer);
static inline int __lexer_parse_hex_escaped__(lexer_t lexer);
static inline int __lexer_parse_oct_escaped__(lexer_t lexer);
static inline int __lexer_parse_unc__(lexer_t lexer, int len);
static inline token_t __lexer_parse_identifier__(lexer_t lexer);
static inline encoding_type_t __lexer_parse_encoding__(lexer_t lexer, int ch);

static inline bool __lexer_is_unc__(lexer_t lexer, int ch);

static inline int __todigit__(int ch);
static inline int __isoct__(int ch);
static inline int __ishex__(int ch);
static inline int __isalnum__(int ch);
static inline int __isalpha__(int ch);
static inline int __isspace__(int ch);
static inline int __isdigit__(int ch);
static inline int __isidnum__(int ch);


lexer_t lexer_create(reader_t reader, option_t option, diag_t diag)
{
    lexer_t lexer;
    token_t tok;

    if ((tok = token_create()) == NULL) {
        return NULL;
    }

    if ((lexer = pmalloc(sizeof(struct lexer_s))) == NULL) {
        token_destroy(tok);
        return NULL;
    }

   
    lexer->reader = reader;
    lexer->option = option;
    lexer->diag = diag;
    lexer->tok = tok;
    return lexer;
}


void lexer_destroy(lexer_t lexer)
{
    assert(lexer && lexer->tok);

    token_destroy(lexer->tok);

    pfree(lexer);
}


token_t lexer_scan(lexer_t lexer)
{
    token_t tok = NULL;
    int ch;

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
        if (__isdigit__(reader_peek(lexer->reader))) {
            reader_unget(lexer->reader, ch);
            return __lexer_parse_number__(lexer);
        }
        if (reader_try(lexer->reader, '.')) {
            if (reader_try(lexer->reader, '.')) {
                return __lexer_token_make__(lexer, TOKEN_ELLIPSIS);
            }

            reader_unget(lexer->reader, '.');
            return __lexer_token_make__(lexer, TOKEN_PERIOD);
        }
        return __lexer_token_make__(lexer, TOKEN_PERIOD);
    case '&':
        if (reader_try(lexer->reader, '&'))
            return __lexer_token_make__(lexer, TOKEN_AMPAMP);
        if (reader_try(lexer->reader, '='))
            return __lexer_token_make__(lexer, TOKEN_AMPEQUAL);
        return __lexer_token_make__(lexer, TOKEN_AMP);
    case '*':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_STAREQUAL : TOKEN_STAR);
    case '+':
        if (reader_try(lexer->reader, '+'))	return __lexer_token_make__(lexer, TOKEN_PLUSPLUS);
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_PLUSEQUAL);
        return __lexer_token_make__(lexer, TOKEN_PLUS);
    case '-':
        if (reader_try(lexer->reader, '>')) return __lexer_token_make__(lexer, TOKEN_ARROW);
        if (reader_try(lexer->reader, '-')) return __lexer_token_make__(lexer, TOKEN_MINUSMINUS);
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_MINUSEQUAL);
        return __lexer_token_make__(lexer, TOKEN_MINUS);
    case '~':
        return __lexer_token_make__(lexer, TOKEN_TILDE);
    case '!':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EXCLAIM : TOKEN_EXCLAIMEQUAL);
    case '/':
        if (reader_test(lexer->reader, '/') || reader_test(lexer->reader, '*')) {
            __lexer_skip_comment__(lexer);
            return lexer_scan(lexer);
        }
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH);
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
    case '>':
        if (reader_try(lexer->reader, '>'))
            return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_GREATERGREATEREQUAL : TOKEN_GREATERGREATER);
        if (reader_try(lexer->reader, '='))
            return __lexer_token_make__(lexer, TOKEN_GREATEREQUAL);
        return __lexer_token_make__(lexer, TOKEN_GREATER);
    case '^':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_CARETEQUAL : TOKEN_CARET);
    case '|':
        if (reader_try(lexer->reader, '|'))
            return __lexer_token_make__(lexer, TOKEN_PIPEPIPE);
        if (reader_try(lexer->reader, '='))
            return __lexer_token_make__(lexer, TOKEN_PIPEEQUAL);
        return __lexer_token_make__(lexer, TOKEN_PIPE);
    case '?':
        return __lexer_token_make__(lexer, TOKEN_QUESTION);
    case ':':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '>') ? TOKEN_R_SQUARE : TOKEN_COLON);
    case ';':
        return __lexer_token_make__(lexer, TOKEN_SEMI);
    case '=':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EQUALEQUAL : TOKEN_EQUAL);
    case ',':
        return __lexer_token_make__(lexer, TOKEN_COMMA);
    case '#':
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '#') ? TOKEN_HASHHASH : TOKEN_HASH);
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9':
        reader_unget(lexer->reader, ch);
        return __lexer_parse_number__(lexer);
    case 'u': case 'U': case 'L': {
        encoding_type_t ent = __lexer_parse_encoding__(lexer, ch);

        if (reader_test(lexer->reader, '\"')) {
            return __lexer_parse_string__(lexer, ent);
        }

        if (reader_test(lexer->reader, '\'')) {
            return __lexer_parse_character__(lexer, ent);
        }

        reader_unget(lexer->reader, ch);
        return __lexer_parse_identifier__(lexer);
    }
    case '\'':
        return __lexer_parse_character__(lexer, ENCODING_NONE);
    case '\"':
        return __lexer_parse_string__(lexer, ENCODING_NONE);
    case '\\':
        if (reader_test(lexer->reader, 'u') || reader_test(lexer->reader, 'U'))
            return __lexer_parse_identifier__(lexer);
        return __lexer_token_make__(lexer, TOKEN_BACKSLASH);

    case EOF:
        return __lexer_token_make__(lexer, TOKEN_END);
            
    default:
        if (__isalpha__(ch) || (0x80 <= ch && ch <= 0xfd) || ch == '_' || ch == '$') {
            /* parse identifier */
            reader_unget(lexer->reader, ch);
            return __lexer_parse_identifier__(lexer);
        }
    }
    
    return tok;
}


static inline 
token_t __lexer_parse_number__(lexer_t lexer)
{
    /* lexer's grammar on numbers is not strict. */

    int ch;
    int prev = -1;

#undef  VALID_SIGN
#define VALID_SIGN(c, prevc) \
  (((c) == '+' || (c) == '-') && \
   ((prevc) == 'e' || (prevc) == 'E' \
    || (((prevc) == 'p' || (prevc) == 'P') )))

    for (;;) {
        ch = reader_get(lexer->reader);
        if (!(__isidnum__(ch) || ch == '.' || VALID_SIGN(ch, prev) || ch == '\'')) {
            break;
        }
        if ((lexer->tok->literals = cstring_cat_ch(lexer->tok->literals, ch)) == NULL) {
            return NULL;
        }
        prev = ch;
    }

    reader_unget(lexer->reader, ch);

#undef  VALID_SIGN

    return __lexer_token_make__(lexer, TOKEN_NUMBER);
}


static inline 
token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent)
{
    int ch;
    bool parsed = false;

    for (;;) {
        ch = reader_get(lexer->reader);
        if (ch == '\'' || ch == '\n' || ch == EOF) {
            break;
        }

        if (parsed) {
            continue;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_unc__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer);
            if (isunc) {
                if ((lexer->tok->literals = cstring_append_utf8(lexer->tok->literals, ch)) == NULL) {
                    return NULL;
                }
                parsed = true;
                continue;
            }
        }

        if ((lexer->tok->literals = cstring_cat_ch(lexer->tok->literals, ch)) == NULL) {
            return NULL;
        }

        parsed = true;
    }


    if (ch != '\'') {
        __lexer_errorf_location__(lexer, "missing terminating ' character");
    }

    if (parsed == false) {
        __lexer_errorf_location__(lexer, "empty character constan");
    }

    return __lexer_token_make__(lexer, ent == ENCODING_CHAR16 ? TOKEN_CONSTANT_CHAR16 :
                                       ent == ENCODING_CHAR32 ? TOKEN_CONSTANT_CHAR32 :
                                       ent == ENCODING_UTF8 ? TOKEN_CONSTANT_UTF8CHAR :
                                       ent == ENCODING_WCHAR ? TOKEN_CONSTANT_WCHAR : TOKEN_CONSTANT_CHAR);
}


static inline 
token_t __lexer_parse_string__(lexer_t lexer, encoding_type_t ent)
{
    int ch;

    for (;;) {
        ch = reader_get(lexer->reader);
        if (ch == '\"' || ch == '\n' || ch == EOF) {
            break;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_unc__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer);
            if (isunc) {
                if ((lexer->tok->literals = cstring_append_utf8(lexer->tok->literals, ch)) == NULL) {
                    return NULL;
                }
                continue;
            }
        }

        if ((lexer->tok->literals = cstring_cat_ch(lexer->tok->literals, ch)) == NULL) {
            return NULL;
        }
    }

    if (ch != '\"') {
        __lexer_errorf_location__(lexer, "unterminated string literal");
    }

    return __lexer_token_make__(lexer, ent == ENCODING_CHAR16 ? TOKEN_CONSTANT_STRING16 :
                                       ent == ENCODING_CHAR32 ? TOKEN_CONSTANT_STRING32 :
                                       ent == ENCODING_UTF8 ? TOKEN_CONSTANT_UTF8STRING :
                                       ent == ENCODING_WCHAR ? TOKEN_CONSTANT_WSTRING : TOKEN_CONSTANT_STRING);
}


static inline
int __lexer_parse_escaped__(lexer_t lexer)
{
    int ch = reader_get(lexer->reader);
    switch (ch) {
    case '\'': case '"': case '?': case '\\':
        return ch;
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case 'e': return '\033';  /* '\e' is GNU extension */
    case 'x': return __lexer_parse_hex_escaped__(lexer);
    case 'u': return __lexer_parse_unc__(lexer, 4);
    case 'U': return __lexer_parse_unc__(lexer, 8);
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
        return __lexer_parse_oct_escaped__(lexer);
    }

    __lexer_warningf_location__(lexer, "unknown escape character: \'%c\'", ch);
    return ch;
}


static inline
int __lexer_parse_hex_escaped__(lexer_t lexer)
{
    int hex = 0, ch = reader_peek(lexer->reader);

    if (!__ishex__(ch)) {
        __lexer_errorf_location__(lexer, "\\x used with no following hex digits");
    }

    while (__ishex__(ch)) {
        hex = (hex << 4) + __todigit__(ch);
        reader_get(lexer->reader);
        ch = reader_peek(lexer->reader);
    }

    return hex;
}


static inline 
int __lexer_parse_oct_escaped__(lexer_t lexer)
{
    int ch;
    int oct;
    
    ch = reader_get(lexer->reader);
    oct = __todigit__(ch);

    ch = reader_peek(lexer->reader);
    if (!__isoct__(ch))
        return oct;
    oct = (oct << 3) + __todigit__(ch);

    reader_get(lexer->reader);
    ch = reader_peek(lexer->reader);
    if (!__isoct__(ch))
        return oct;
    oct = (oct << 3) + __todigit__(ch);

    reader_get(lexer->reader);
    return oct;
}


static inline
int __lexer_parse_unc__(lexer_t lexer, int len)
{
    int ch;
    int u = 0;
    int i = 0;

    assert(len == 4 || len == 8);

    for (; i < len; ++i) {
        ch = reader_get(lexer->reader);
        if (!__ishex__(ch)) {
            __lexer_errorf_location__(lexer, "invalid universal character");
        }
        u = (u << 4) + __todigit__(ch);
    }

    return u;
}


static inline 
token_t __lexer_parse_identifier__(lexer_t lexer)
{
    int ch;

    for (;;) {
        ch = reader_get(lexer->reader);
        if (__isidnum__(ch) || ch == '$' || (0x80 <= ch && ch <= 0xfd)) {
            if ((lexer->tok->literals = cstring_cat_ch(lexer->tok->literals, ch)) == NULL) {
                return NULL;
            }
            continue;
        }

        if (__lexer_is_unc__(lexer, ch)) {
            if ((lexer->tok->literals = cstring_append_utf8(lexer->tok->literals, __lexer_parse_escaped__(lexer))) == NULL) {
                return NULL;
            }
            continue;
        }

        break;
    }

    reader_unget(lexer->reader, ch);
    return __lexer_token_make__(lexer, TOKEN_IDENTIFIER);
}


static inline
encoding_type_t __lexer_parse_encoding__(lexer_t lexer, int ch)
{
    switch (ch) {
    case 'u': 
        return reader_try(lexer->reader, '8') ? ENCODING_UTF8 : ENCODING_CHAR16;
    case 'U': 
        return ENCODING_CHAR32;
    case 'L': 
        return ENCODING_WCHAR;
    }
    assert(false);
    return ENCODING_NONE;
}


static inline 
void __lexer_skip_white_space__(lexer_t lexer)
{
    int ch;
    do {
        ch = reader_peek(lexer->reader);
        if (!__isspace__(ch) || ch == '\n' || ch == EOF) {
            break;
        }
        reader_get(lexer->reader);
    } while (true);
}


static inline 
void __lexer_skip_comment__(lexer_t lexer)
{
    if (reader_try(lexer->reader, '/')) {
        while (!reader_is_empty(lexer->reader)) {
            if (reader_peek(lexer->reader) == '\n') {
                return;
            }
            reader_get(lexer->reader);
        }
    } else if (reader_try(lexer->reader, '*')) {
        int ch;
        while (!reader_is_empty(lexer->reader)) {
            ch = reader_get(lexer->reader);
            if (ch == '*' && reader_try(lexer->reader, '/')) {
                return;
            }
        }
        __lexer_errorf_location__(lexer, "unknown identifier");
        return;
    }

    assert(false);
}


static inline
bool __lexer_is_unc__(lexer_t lexer, int ch)
{
    return ch == '\\' && (reader_test(lexer->reader, 'u') || reader_test(lexer->reader, 'U'));
}


static inline 
void __lexer_errorf_location__(lexer_t lexer, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diag_errorvf_location(lexer->diag, lexer->tok->location, fmt, ap);

    va_end(ap);
}


static inline
void __lexer_warningf_location__(lexer_t lexer, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diag_warningvf_location(lexer->diag, lexer->tok->location, fmt, ap);

    va_end(ap);
}


static inline
void __lexer_token_mark_location__(lexer_t lexer)
{
    token_init_loc(lexer->tok,
                   reader_line(lexer->reader),
                   reader_column(lexer->reader),
                   reader_current_line(lexer->reader),
                   cstring_create(reader_name(lexer->reader)));
}


static inline 
token_t __lexer_token_make__(lexer_t lexer, token_type_t type)
{
    token_t tok;

    lexer->tok->type = type;

    if ((tok = token_dup(lexer->tok)) == NULL) {
        return NULL;
    }

    /* reset the current token */
    token_init(lexer->tok);

    return tok;
}


static inline
int __todigit__(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }
    assert(false); 
    return ch;
}


static inline 
int __isoct__(int ch)
{
    return '0' <= ch && ch <= '7';
}


static inline 
int __ishex__(int ch)
{
    return isxdigit(ch);
}


static inline
int __isalnum__(int ch)
{
    return isalnum(ch);
}


static inline
int __isalpha__(int ch)
{
    return isalpha(ch);
}


static inline
int __isspace__(int ch)
{
    return isspace(ch);
}


static inline 
int __isdigit__(int ch)
{
    return isdigit(ch);
}


static inline
int __isidnum__(int ch)
{
    return isalnum(ch) || ch == '_';
}
