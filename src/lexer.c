

#include "config.h"
#include "pmalloc.h"
#include "token.h"
#include "reader.h"
#include "diag.h"
#include "lexer.h"
#include "array.h"
#include "charop.h"
#include "encoding.h"


static inline void __lexer_errorf_location__(lexer_t lexer, const char *fmt, ...);
static inline void __lexer_warningf_location__(lexer_t lexer, const char *fmt, ...);

static inline token_t __lexer_token_make__(lexer_t lexer, token_type_t type);
static inline void __lexer_token_mark_loc__(lexer_t lexer);
static inline void __lexer_token_remark_loc__(lexer_t lexer);

static inline bool __lexer_skip_white_space__(lexer_t lexer);
static inline void __lexer_skip_comment__(lexer_t lexer);
static inline token_t __lexer_parse_number__(lexer_t lexer);
static inline token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent);
static inline token_t __lexer_parse_string__(lexer_t lexer, encoding_type_t ent);
static inline int __lexer_parse_escaped__(lexer_t lexer);
static inline int __lexer_parse_hex_escaped__(lexer_t lexer);
static inline int __lexer_parse_oct_escaped__(lexer_t lexer, int ch);
static inline int __lexer_parse_unc__(lexer_t lexer, int len);
static inline token_t __lexer_parse_identifier__(lexer_t lexer);
static inline encoding_type_t __lexer_parse_encoding__(lexer_t lexer, int ch);

static inline bool __lexer_is_unc__(lexer_t lexer, int ch);


static inline bool __lexer_make_stash__(array_t a);


lexer_t lexer_create(reader_t reader, option_t option, diag_t diag)
{
    lexer_t lexer;
    token_t tok;
    array_t snapshot;

    if ((snapshot = array_create_n(sizeof(array_t), 12)) == NULL) {
        goto done;
    }

    if (!__lexer_make_stash__(snapshot)) {
        goto done;
    }

    if ((tok = token_create()) == NULL) {
        goto clean_ss;
    }

    if ((lexer = pmalloc(sizeof(struct lexer_s))) == NULL) {
        goto clean_tok;
    }

    time_t timet = time(NULL);
    localtime_r(&timet, &lexer->tm);

    lexer->reader = reader;
    lexer->option = option;
    lexer->diag = diag;
    lexer->tok = tok;
    lexer->snapshot = snapshot;
    return lexer;

clean_tok:
    token_destroy(tok);

clean_ss:
    array_destroy(snapshot);

done:
    return NULL;
}


void lexer_destroy(lexer_t lexer)
{
    array_t *arrs;
    token_t *toks;
    size_t i, j;

    assert(lexer && lexer->tok && lexer->snapshot);

    array_foreach(lexer->snapshot, arrs, i) {
        array_foreach(arrs[i], toks, j) {
            token_destroy(toks[j]);
        }
        array_destroy(arrs[i]);
    }

    array_destroy(lexer->snapshot);

    token_destroy(lexer->tok);

    pfree(lexer);
}


token_t lexer_scan(lexer_t lexer)
{
    int ch;

    __lexer_token_mark_loc__(lexer);

    if (__lexer_skip_white_space__(lexer)) {
        return __lexer_token_make__(lexer, TOKEN_SPACE);
    }

    ch = reader_next(lexer->reader);
    switch (ch) {
    case '\n':
        return __lexer_token_make__(lexer, TOKEN_NEW_LINE);
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
        if (chisdigit(reader_peek(lexer->reader))) {
            reader_untread(lexer->reader, ch);
            return __lexer_parse_number__(lexer);
        }
        if (reader_try(lexer->reader, '.')) {
            if (reader_try(lexer->reader, '.')) {
                return __lexer_token_make__(lexer, TOKEN_ELLIPSIS);
            }

            reader_untread(lexer->reader, '.');
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
            return __lexer_token_make__(lexer, TOKEN_COMMENT);
        }
        return __lexer_token_make__(lexer, reader_try(lexer->reader, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH);
    case '%':
        if (reader_try(lexer->reader, '=')) return __lexer_token_make__(lexer, TOKEN_PERCENTEQUAL);
        if (reader_try(lexer->reader, '>')) return __lexer_token_make__(lexer, TOKEN_R_BRACE);
        if (reader_try(lexer->reader, ':')) {
            if (reader_try(lexer->reader, '%')) {
                if (reader_try(lexer->reader, ':'))
                    return __lexer_token_make__(lexer, TOKEN_HASHHASH);
                reader_untread(lexer->reader, '%');
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
        reader_untread(lexer->reader, ch);
        return __lexer_parse_number__(lexer);
    case 'u': case 'U': case 'L': {
        encoding_type_t ent = __lexer_parse_encoding__(lexer, ch);

        if (reader_try(lexer->reader, '\"')) {
            return __lexer_parse_string__(lexer, ent);
        }

        if (reader_try(lexer->reader, '\'')) {
            return __lexer_parse_character__(lexer, ent);
        }

        reader_untread(lexer->reader, ch);
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
        if (chisalpha(ch) || (0x80 <= ch && ch <= 0xfd) || ch == '_' || ch == '$') {
            /* parse identifier */
            reader_untread(lexer->reader, ch);
            return __lexer_parse_identifier__(lexer);
        }
    }
    
    assert(false);
    return NULL;
}


token_t lexer_next(lexer_t lexer)
{
    token_t tok;
    bool begin_of_line = false;
    bool leading_space = false;
    
    begin_of_line = reader_line(lexer->reader) == 1 ? true : false;

    tok = lexer_scan(lexer);
    while (tok->type == TOKEN_SPACE || 
           tok->type == TOKEN_COMMENT) {
        token_destroy(tok);
        tok = lexer_scan(lexer);
        leading_space = true;
    }

    tok->begin_of_line = begin_of_line;
    tok->leading_space = leading_space;
    return tok;
}


token_t lexer_tokenize(lexer_t lexer)
{
    array_t *arrs;
    array_t tokarr;
    token_t *toks;

    arrs = array_prototype(lexer->snapshot, array_t);
    tokarr = arrs[array_size(lexer->snapshot)];
    toks = array_prototype(tokarr, token_t);
}


bool lexer_push_back(lexer_t lexer, token_t tok)
{
    array_t *arrs;
    array_t tail;
    token_t *item;

    assert(array_size(lexer->snapshot) > 0);

    arrs = array_prototype(lexer->snapshot, array_t);
    tail = arrs[array_size(lexer->snapshot)];

    item = array_push(tail);
    if (item == NULL) {
        return false;
    }

    *item = tok;
    return true;
}


bool lexer_stash(lexer_t lexer)
{
    return __lexer_make_stash__(lexer->snapshot);
}


void lexer_unstash(lexer_t lexer)
{
    assert(array_size(lexer->snapshot) > 0);
    array_pop(lexer->snapshot);
}


cstring_t lexer_date(lexer_t lexer)
{
    char buf[20];
    strftime(buf, sizeof(buf), "%b %e %Y", &lexer->tm);
    return cstring_create(buf);
}


cstring_t lexer_time(lexer_t lexer)
{
    char buf[10];
    strftime(buf, sizeof(buf), "%T", &lexer->tm);
    return cstring_create(buf);
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
        ch = reader_next(lexer->reader);
        if (!(chisidnum(ch) || ch == '.' || VALID_SIGN(ch, prev) || ch == '\'')) {
            break;
        }
        if ((lexer->tok->literals = cstring_cat_ch(lexer->tok->literals, ch)) == NULL) {
            return NULL;
        }
        prev = ch;
    }

    reader_untread(lexer->reader, ch);

#undef  VALID_SIGN

    return __lexer_token_make__(lexer, TOKEN_NUMBER);
}


static inline 
token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent)
{
    int ch;
    bool parsed = false;

    for (;;) {
        ch = reader_next(lexer->reader);
        if (ch == '\'' || ch == '\n' || ch == EOF) {
            break;
        }

        if (parsed == true) {
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
        __lexer_errorf_location__(lexer, "empty character constant");
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
        ch = reader_next(lexer->reader);
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
    int ch;

    __lexer_token_remark_loc__(lexer);

    ch = reader_next(lexer->reader);
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
    case 'x': return __lexer_parse_hex_escaped__(lexer);
    case 'u': return __lexer_parse_unc__(lexer, 4);
    case 'U': return __lexer_parse_unc__(lexer, 8);
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
        return __lexer_parse_oct_escaped__(lexer, ch);
    }

    __lexer_warningf_location__(lexer, "unknown escape character: \'%c\'", ch);
    return ch;
}


static inline
int __lexer_parse_hex_escaped__(lexer_t lexer)
{
    int hex = 0, ch = reader_peek(lexer->reader);

    if (!chishex(ch)) {
        __lexer_errorf_location__(lexer, "\\x used with no following hex digits");
    }

    while (chishex(ch)) {
        hex = (hex << 4) + chtodigit(ch);
        reader_next(lexer->reader);
        ch = reader_peek(lexer->reader);
    }

    return hex;
}


static inline 
int __lexer_parse_oct_escaped__(lexer_t lexer, int ch)
{
    int oct;
    
    oct = chtodigit(ch);

    ch = reader_peek(lexer->reader);
    if (!chisoct(ch))
        return oct;
    oct = (oct << 3) + chtodigit(ch);

    reader_next(lexer->reader);
    ch = reader_peek(lexer->reader);
    if (!chisoct(ch))
        return oct;
    oct = (oct << 3) + chtodigit(ch);

    reader_next(lexer->reader);
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
        ch = reader_next(lexer->reader);
        if (!chishex(ch)) {
            __lexer_errorf_location__(lexer, "invalid universal character");
        }
        u = (u << 4) + chtodigit(ch);
    }

    return u;
}


static inline 
token_t __lexer_parse_identifier__(lexer_t lexer)
{
    int ch;

    for (;;) {
        ch = reader_next(lexer->reader);
        if (chisidnum(ch) || ch == '$' || (0x80 <= ch && ch <= 0xfd)) {
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

    reader_untread(lexer->reader, ch);
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
bool __lexer_skip_white_space__(lexer_t lexer)
{
    int ch;
    bool ret = false;

    for (;;) {
        ch = reader_peek(lexer->reader);
        if (!chisspace(ch) || ch == '\n' || ch == EOF) {
            break;
        }
        reader_next(lexer->reader);
        ret = true;
    }

    return ret;
}


static inline 
void __lexer_skip_comment__(lexer_t lexer)
{
    if (reader_try(lexer->reader, '/')) {
        while (!reader_is_empty(lexer->reader)) {
            if (reader_peek(lexer->reader) == '\n') {
                return;
            }
            reader_next(lexer->reader);
        }
    } else if (reader_try(lexer->reader, '*')) {
        int ch;
        while (!reader_is_empty(lexer->reader)) {
            ch = reader_next(lexer->reader);
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
bool __lexer_make_stash__(array_t a)
{
    array_t *item = array_push(a);
    if (item == NULL) {
        return false;
    }

    if ((*item = array_create_n(sizeof(struct token_s), 12)) == NULL) {
        return false;
    }

    return true;
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

    diag_errorvf_with_loc(lexer->diag, lexer->tok->loc, fmt, ap);

    va_end(ap);
}


static inline
void __lexer_warningf_location__(lexer_t lexer, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diag_warningvf_with_loc(lexer->diag, lexer->tok->loc, fmt, ap);

    va_end(ap);
}


static inline
void __lexer_token_mark_loc__(lexer_t lexer)
{
    token_mark_loc(lexer->tok,
                   reader_line(lexer->reader),
                   reader_column(lexer->reader),
                   reader_row(lexer->reader),
                   cstring_create(reader_name(lexer->reader)));
}


static inline 
void __lexer_token_remark_loc__(lexer_t lexer)
{
    token_remark_loc(lexer->tok, reader_line(lexer->reader),
        reader_column(lexer->reader));
}


static inline 
token_t __lexer_token_make__(lexer_t lexer, token_type_t type)
{
    token_t tok;

    lexer->tok->type = type;

    if ((tok = token_dup(lexer->tok)) == NULL) {
        return NULL;
    }

    token_init(lexer->tok);

    return tok;
}

