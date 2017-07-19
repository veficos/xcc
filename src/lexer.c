

#include "config.h"
#include "pmalloc.h"
#include "token.h"
#include "reader.h"
#include "diagnostor.h"
#include "lexer.h"
#include "array.h"
#include "utils.h"
#include "cstring.h"
#include "encoding.h"
#include "option.h"


static inline token_t* __lexer_parse_number__(lexer_t *lexer, token_t *token, int ch);
static inline encoding_type_t __lexer_parse_encoding__(lexer_t *lexer, int ch);
static inline token_t* __lexer_parse_character__(lexer_t *lexer, token_t *token, encoding_type_t ent);
static inline token_t* __lexer_parse_string__(lexer_t *lexer, token_t *token, encoding_type_t ent);
static inline token_t* __lexer_parse_identifier__(lexer_t *lexer, token_t *token);
static inline bool __lexer_parse_spaces__(lexer_t *lexer, token_t *token);
static inline token_t* __lexer_parse_comment__(lexer_t *lexer, token_t *token);

static inline token_t* __lexer_make_token__(lexer_t *lexer, token_t *token, token_type_t type);
static inline void __lexer_mark_location__(lexer_t *lexer, token_t *token);
static inline void __remark_location__(lexer_t *lexer, token_t *token);


lexer_t* lexer_create(void)
{
    lexer_t *lexer;

    lexer = pmalloc(sizeof(struct lexer_s));

    lexer->reader = reader_create();

    return lexer;
}


lexer_t* lexer_create_csp(cspool_t *csp)
{
    lexer_t *lexer;

    lexer = pmalloc(sizeof(struct lexer_s));

    lexer->reader = reader_create_csp(csp);

    return lexer;
}


bool lexer_push(lexer_t *lexer, stream_type_t type, const unsigned char* s)
{
    return reader_push(lexer->reader, type, s);
}


void lexer_destroy(lexer_t *lexer)
{
    assert(lexer != NULL);

    reader_destroy(lexer->reader);

    pfree(lexer);
}


token_t* lexer_scan(lexer_t *lexer)
{
    int ch;
    token_t *token;

    if (reader_is_empty(lexer->reader)) {
        return token_create(TOKEN_END, NULL, NULL);
    }

    token = token_create(TOKEN_UNKNOWN, cstring_new_n(NULL, 8), NULL);

    __lexer_mark_location__(lexer, token);

    if (__lexer_parse_spaces__(lexer, token)) {
        return __lexer_make_token__(lexer, token, TOKEN_SPACE);
    }

    ch = reader_get(lexer->reader);
    switch (ch) {
    case '\n':
        return __lexer_make_token__(lexer, token, TOKEN_NEWLINE);
    case '[':
        return __lexer_make_token__(lexer, token, TOKEN_L_SQUARE);
    case ']':
        return __lexer_make_token__(lexer, token, TOKEN_R_SQUARE);
    case '(':
        return __lexer_make_token__(lexer, token, TOKEN_L_PAREN);
    case ')':
        return __lexer_make_token__(lexer, token, TOKEN_R_PAREN);
    case '{':
        return __lexer_make_token__(lexer, token, TOKEN_L_BRACE);
    case '}':
        return __lexer_make_token__(lexer, token, TOKEN_R_BRACE);
    case '.':
        if (ISDIGIT(reader_peek(lexer->reader))) {
            return __lexer_parse_number__(lexer, token, ch);
        }
        if (reader_try(lexer->reader, '.')) {
            if (reader_try(lexer->reader, '.')) {
                return __lexer_make_token__(lexer, token, TOKEN_ELLIPSIS);
            }

            reader_unget(lexer->reader, '.');
            return __lexer_make_token__(lexer, token, TOKEN_PERIOD);
        }
        return __lexer_make_token__(lexer, token, TOKEN_PERIOD);
    case '&':
        if (reader_try(lexer->reader, '&'))
            return __lexer_make_token__(lexer, token, TOKEN_AMPAMP);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, token, TOKEN_AMPEQUAL);
        return __lexer_make_token__(lexer, token, TOKEN_AMP);
    case '*':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_STAREQUAL : TOKEN_STAR);
    case '+':
        if (reader_try(lexer->reader, '+'))	return __lexer_make_token__(lexer, token, TOKEN_PLUSPLUS);
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, token, TOKEN_PLUSEQUAL);
        return __lexer_make_token__(lexer, token, TOKEN_PLUS);
    case '-':
        if (reader_try(lexer->reader, '>')) return __lexer_make_token__(lexer, token, TOKEN_ARROW);
        if (reader_try(lexer->reader, '-')) return __lexer_make_token__(lexer, token, TOKEN_MINUSMINUS);
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, token, TOKEN_MINUSEQUAL);
        return __lexer_make_token__(lexer, token, TOKEN_MINUS);
    case '~':
        return __lexer_make_token__(lexer, token, TOKEN_TILDE);
    case '!':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_EXCLAIM : TOKEN_EXCLAIMEQUAL);
    case '/':
        if (reader_test(lexer->reader, '/') || reader_test(lexer->reader, '*')) {
            return __lexer_parse_comment__(lexer, token);
        }
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH);
    case '%':
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, token, TOKEN_PERCENTEQUAL);
        if (reader_try(lexer->reader, '>')) return __lexer_make_token__(lexer, token, TOKEN_R_BRACE);
        if (reader_try(lexer->reader, ':')) {
            if (reader_try(lexer->reader, '%')) {
                if (reader_try(lexer->reader, ':'))
                    return __lexer_make_token__(lexer, token, TOKEN_HASHHASH);
                reader_unget(lexer->reader, '%');
            }
            return __lexer_make_token__(lexer, token, TOKEN_HASH);
        }
        return __lexer_make_token__(lexer, token, TOKEN_PERCENT);
    case '<':
        if (reader_try(lexer->reader, '<'))
            return __lexer_make_token__(lexer, token,
                                        reader_try(lexer->reader, '=') ? TOKEN_LESSLESSEQUAL : TOKEN_LESSLESS);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, token, TOKEN_LESSEQUAL);
        if (reader_try(lexer->reader, ':'))
            return __lexer_make_token__(lexer, token, TOKEN_L_SQUARE);
        if (reader_try(lexer->reader, '%'))
            return __lexer_make_token__(lexer, token, TOKEN_L_BRACE);
        return __lexer_make_token__(lexer, token, TOKEN_LESS);
    case '>':
        if (reader_try(lexer->reader, '>'))
            return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_GREATERGREATEREQUAL
                                                                                     : TOKEN_GREATERGREATER);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, token, TOKEN_GREATEREQUAL);
        return __lexer_make_token__(lexer, token, TOKEN_GREATER);
    case '^':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_CARETEQUAL : TOKEN_CARET);
    case '|':
        if (reader_try(lexer->reader, '|'))
            return __lexer_make_token__(lexer, token, TOKEN_PIPEPIPE);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, token, TOKEN_PIPEEQUAL);
        return __lexer_make_token__(lexer, token, TOKEN_PIPE);
    case '?':
        return __lexer_make_token__(lexer, token, TOKEN_QUESTION);
    case ':':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '>') ? TOKEN_R_SQUARE : TOKEN_COLON);
    case ';':
        return __lexer_make_token__(lexer, token, TOKEN_SEMI);
    case '=':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '=') ? TOKEN_EQUALEQUAL : TOKEN_EQUAL);
    case ',':
        return __lexer_make_token__(lexer, token, TOKEN_COMMA);
    case '#':
        return __lexer_make_token__(lexer, token, reader_try(lexer->reader, '#') ? TOKEN_HASHHASH : TOKEN_HASH);
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9':
        return __lexer_parse_number__(lexer, token, ch);
    case 'u': case 'U': case 'L': {
        encoding_type_t ent = __lexer_parse_encoding__(lexer, ch);

        if (reader_test(lexer->reader, '\"')) {
            __remark_location__(lexer, token);
            reader_get(lexer->reader);
            return __lexer_parse_string__(lexer, token, ent);
        }

        if (ent != ENCODING_UTF8 && reader_test(lexer->reader, '\'')) {
            __remark_location__(lexer, token);
            reader_get(lexer->reader);
            return __lexer_parse_character__(lexer, token, ent);
        }

        if (ent == ENCODING_UTF8) {
            reader_unget(lexer->reader, '8');
        }

        reader_unget(lexer->reader, ch);
        return __lexer_parse_identifier__(lexer, token);
    }
    case '\'':
        return __lexer_parse_character__(lexer, token, ENCODING_NONE);
    case '\"':
        return __lexer_parse_string__(lexer, token, ENCODING_NONE);
    case '\\':
        if (reader_test(lexer->reader, 'u') || reader_test(lexer->reader, 'U'))
            return __lexer_parse_identifier__(lexer, token);
        return __lexer_make_token__(lexer, token, TOKEN_BACKSLASH);
    case EOF:
        reader_pop(lexer->reader);
        return __lexer_make_token__(lexer, token, TOKEN_EOF);
    default:
        if (ISALPHA(ch) || (0x80 <= ch && ch <= 0xfd) || ch == '_' || ch == '$') {
            /* parse identifier */
            reader_unget(lexer->reader, ch);
            return __lexer_parse_identifier__(lexer, token);
        }
    }
    
    assert(false);
    return NULL;
}


static inline
bool __lexer_parse_spaces__(lexer_t *lexer, token_t *token)
{
    int ch;

    for (;;) {
        ch = reader_peek(lexer->reader);
        if (!ISSPACE(ch) || ch == '\n' || ch == EOF) {
            break;
        }
        reader_get(lexer->reader);
        token->spaces++;
    }

    return token->spaces > 0;
}


static inline
token_t* __lexer_parse_comment__(lexer_t *lexer, token_t *token)
{
#undef  reserve_comment
#define reserve_comment(ch)                                 \
    do {                                                    \
        if (option_get(reserve_comment)) {                  \
            token->cs = cstring_push_ch(token->cs, ch);     \
        }                                                   \
    } while(false)

    reserve_comment('/');

    if (reader_try(lexer->reader, '/')) {
        int ch;

        token->cs = cstring_push_ch(token->cs, '/');

        while (!reader_is_empty(lexer->reader)) {
            if (reader_peek(lexer->reader) == '\n') {
                return __lexer_make_token__(lexer, token, TOKEN_COMMENT);
            }
            ch = reader_get(lexer->reader);

            reserve_comment(ch);
        }
    } else if (reader_try(lexer->reader, '*')) {
        int ch;

        reserve_comment('*');

        while ((ch = reader_get(lexer->reader)) != EOF) {
            reserve_comment(ch);

            if (ch == '*' && reader_try(lexer->reader, '/')) {
                reserve_comment('/');
                return __lexer_make_token__(lexer, token, TOKEN_COMMENT);
            }
        }

        token_add_linenote_caution(token, token->location.column, 2);

        errorf_with_token(token, "unterminated comment");
        return __lexer_make_token__(lexer, token, TOKEN_COMMENT);
    }

#undef reserve_comment
    assert(false);
    return NULL;
}


static inline
bool __is_valid_universal_char__(unsigned int u)
{
    if (0xD800 <= u && u <= 0xDFFF)
        return false;
    return 0xA0 <= u || u == '$' || u == '@' || u == '`';
}


static inline
bool __lexer_is_universal_char__(lexer_t *lexer, int ch)
{
    return ch == '\\' && (reader_test(lexer->reader, 'u') || reader_test(lexer->reader, 'U'));
}


static inline
token_t* __lexer_parse_number__(lexer_t *lexer, token_t *token, int ch)
{
    /* lexer's grammar on numbers is not strict. */
    int prev = -1;

#undef  VALID_SIGN
#define VALID_SIGN(c, prevc) \
  (((c) == '+' || (c) == '-') && \
   ((prevc) == 'e' || (prevc) == 'E' \
    || (((prevc) == 'p' || (prevc) == 'P') )))

    token->cs = cstring_concat_ch(token->cs, ch);

    for (;;) {
        ch = reader_peek(lexer->reader);
        if (!(ISIDNUM(ch) || ch == '.' || VALID_SIGN(ch, prev) || ch == '\'')) {
            break;
        }

        token->cs = cstring_concat_ch(token->cs, ch);

        prev = ch;

        reader_get(lexer->reader);
    }

#undef  VALID_SIGN

    return __lexer_make_token__(lexer, token, TOKEN_NUMBER);
}


static inline
int __lexer_parse_hex_escaped__(lexer_t *lexer, token_t *token)
{
    int hex = 0, ch = reader_peek(lexer->reader);

    if (!ISHEX(ch)) {
        errorf_with_token(token, "\\x used with no following hex digits");
    }

    while (ISHEX(ch)) {
        hex = (hex << 4) + TODIGIT(ch);
        reader_get(lexer->reader);
        ch = reader_peek(lexer->reader);
    }

    return hex;
}


static inline
int __lexer_parse_universal_char__(lexer_t *lexer, token_t *token, int len)
{
    int ch, i;
    unsigned int u = 0;

    assert(len == 4 || len == 8);

    for (i = 0; i < len; ++i) {
        ch = reader_get(lexer->reader);
        if (!ISHEX(ch)) {
            errorf_with_token(token, "incomplete universal character name");
        }
        u = (u << 4) + TODIGIT(ch);
    }

    if (!__is_valid_universal_char__(u)) {
        /* ERRORF_WITH_LOC("\\%c%0*x is not a valid universal character", (len == 4) ? 'u' : 'U', len, u); */
    }

    return u;
}


static inline
int __lexer_parse_oct_escaped__(lexer_t *lexer, int ch)
{
    int oct;

    oct = TODIGIT(ch);

    ch = reader_peek(lexer->reader);
    if (!ISOCT(ch))
        return oct;
    oct = (oct << 3) + TODIGIT(ch);

    reader_get(lexer->reader);
    ch = reader_peek(lexer->reader);
    if (!ISOCT(ch))
        return oct;
    oct = (oct << 3) + TODIGIT(ch);

    reader_get(lexer->reader);
    return oct;
}


static inline
int __lexer_parse_escaped__(lexer_t *lexer, token_t *token)
{
    int ch;

    ch = reader_get(lexer->reader);
    switch (ch) {
    case '\'': case '"':
    case '?': case '\\':
        return ch;
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case 'e': case 'E':
        return '\033';  /* '\e' is GNU extension */
    case 'x': return __lexer_parse_hex_escaped__(lexer, token);
    case 'u': return __lexer_parse_universal_char__(lexer, token, 4);
    case 'U': return __lexer_parse_universal_char__(lexer, token, 8);
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        return __lexer_parse_oct_escaped__(lexer, ch);
    }

    //WARNINGF_WITH_LOC("unknown escape sequence: \'%c\'", ch);
    return ch;
}


static inline
encoding_type_t __lexer_parse_encoding__(lexer_t *lexer, int ch)
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
token_t* __lexer_parse_identifier__(lexer_t *lexer, token_t *token)
{
    int ch;

    for (;;) {
        ch = reader_get(lexer->reader);
        if (ISIDNUM(ch) || ch == '$' || (0x80 <= ch && ch <= 0xfd)) {
            token->cs = cstring_concat_ch(token->cs, ch);
            continue;
        }

        if (__lexer_is_universal_char__(lexer, ch)) {
            token->cs = cstring_append_utf8(token->cs, __lexer_parse_escaped__(lexer, token));
            continue;
        }

        break;
    }

    reader_unget(lexer->reader, ch);
    return __lexer_make_token__(lexer, token, TOKEN_IDENTIFIER);
}

static inline 
token_t* __lexer_parse_character__(lexer_t *lexer, token_t *token, encoding_type_t ent)
{
    int ch;

    for (; !reader_is_empty(lexer->reader) ;) {
        ch = reader_get(lexer->reader);
        if (ch == '\'' || ch == '\n') {
            break;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_universal_char__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer, token);
            if (isunc) {
                token->cs = cstring_append_utf8(token->cs, ch);
                continue;
            }
        }

        token->cs = cstring_concat_ch(token->cs, ch);
    }

    if (ch != '\'') {
        errorf_with_token(token, "missing terminating ' character");
    } else if (cstring_length(token->cs) == 0) {
        errorf_with_token(token, "empty character constant");
    }
   
    return __lexer_make_token__(lexer, token, ent2tokt(ent, CHAR));
}


static inline 
token_t* __lexer_parse_string__(lexer_t *lexer, token_t *token, encoding_type_t ent)
{
    int ch;

    for (; !reader_is_empty(lexer->reader) ;) {
        ch = reader_get(lexer->reader);
        if (ch == '\"' || ch == '\n') {
            break;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_universal_char__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer, token);
            if (isunc) {
                token->cs = cstring_append_utf8(token->cs, ch);
                continue;
            }
        }

        token->cs = cstring_concat_ch(token->cs, ch);
    }

    if (ch != '\"') {
        errorf_with_token(token, "unterminated string literal");
    }

    return __lexer_make_token__(lexer, token, ent2tokt(ent, STRING));
}


static inline
token_t* __lexer_make_token__(lexer_t *lexer, token_t *token, token_type_t type)
{
    token->type = type;
    return token;
}


static inline
void __lexer_mark_location__(lexer_t *lexer, token_t *token)
{
    token->location.line = reader_line(lexer->reader);
    token->location.column = reader_column(lexer->reader);
    token->location.filename = reader_filename(lexer->reader);
    token->location.linenote = reader_linenote(lexer->reader);
}


static inline
void __remark_location__(lexer_t *lexer, token_t *token)
{
    token->location.line = reader_line(lexer->reader);
    token->location.column = reader_column(lexer->reader);
    token->location.linenote = reader_linenote(lexer->reader);
}