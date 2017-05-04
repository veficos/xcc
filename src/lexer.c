

#include "config.h"
#include "pmalloc.h"
#include "token.h"
#include "reader.h"
#include "diag.h"
#include "lexer.h"
#include "array.h"
#include "utils.h"
#include "encoding.h"


#undef  ERRORF_WITH_TOKEN
#define ERRORF_WITH_TOKEN(fmt, ...) \
    diag_errorf_with_tok((lexer)->diag, (lexer)->tok, fmt, __VA_ARGS__)


#undef  WARNINGF_WITH_TOKEN
#define WARNINGF_WITH_TOKEN(fmt, ...) \
    diag_warningf_with_tok((lexer)->diag, (lexer)->tok, fmt, __VA_ARGS__)


#undef  ERRORF_WITH_LOC
#define ERRORF_WITH_LOC(fmt, ...) \
    diag_errorf_with_loc((lexer)->diag, (lexer)->tok->loc, fmt, __VA_ARGS__)


#undef  WARNINGF_WITH_LOC
#define WARNINGF_WITH_LOC(fmt, ...) \
    diag_warningf_with_loc((lexer)->diag, (lexer)->tok->loc, fmt, __VA_ARGS__)


static inline token_t __lexer_parse_number__(lexer_t lexer, int ch);
static inline encoding_type_t __lexer_parse_encoding__(lexer_t lexer, int ch);
static inline token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent);
static inline token_t __lexer_parse_string__(lexer_t lexer, encoding_type_t ent);
static inline token_t __lexer_parse_identifier__(lexer_t lexer);
static inline bool __lexer_parse_white_space__(lexer_t lexer);
static inline void __lexer_parse_comment__(lexer_t lexer);

static inline void __lexer_make_stash__(array_t a);
static inline array_t __lexer_last_snapshot__(lexer_t lexer);

static inline token_t __lexer_make_token__(lexer_t lexer, token_type_t type);
static inline void __lexer_mark_loc__(lexer_t lexer);
static inline void __lexer_remark_loc__(lexer_t lexer);


lexer_t lexer_create(reader_t reader, option_t option, diag_t diag)
{
    lexer_t lexer;
    time_t t;

    lexer = pmalloc(sizeof(struct lexer_s));

    lexer->reader = reader;
    lexer->option = option;
    lexer->diag = diag;
    lexer->tok = token_create();
    lexer->snapshots = array_create_n(sizeof(array_t), 12);

    t = time(NULL);
    localtime_r(&t, &lexer->tm);

    __lexer_make_stash__(lexer->snapshots);
    return lexer;
}


void lexer_destroy(lexer_t lexer)
{
    array_t *snapshot;
    token_t *tokens;
    size_t i, j;

    assert(lexer != NULL);

    array_foreach(lexer->snapshots, snapshot, i) {
        array_foreach(snapshot[i], tokens, j) {
            token_destroy(tokens[j]);
        }
        array_destroy(snapshot[i]);
    }

    array_destroy(lexer->snapshots);

    token_destroy(lexer->tok);

    pfree(lexer);
}


bool lexer_push(lexer_t lexer, stream_type_t type, const unsigned char* s)
{
    return reader_push(lexer->reader, type, s);
}


token_t lexer_scan(lexer_t lexer)
{
    int ch;

    if (reader_peek(lexer->reader) == EOF) {
        return __lexer_make_token__(lexer, TOKEN_END);
    }

    __lexer_mark_loc__(lexer);

    if (__lexer_parse_white_space__(lexer)) {
        return __lexer_make_token__(lexer, TOKEN_SPACE);
    }

    ch = reader_get(lexer->reader);
    switch (ch) {
    case '\n':
        return __lexer_make_token__(lexer, TOKEN_NEWLINE);
    case '[':
        return __lexer_make_token__(lexer, TOKEN_L_SQUARE);
    case ']':
        return __lexer_make_token__(lexer, TOKEN_R_SQUARE);
    case '(':
        return __lexer_make_token__(lexer, TOKEN_L_PAREN);
    case ')':
        return __lexer_make_token__(lexer, TOKEN_R_PAREN);
    case '{':
        return __lexer_make_token__(lexer, TOKEN_L_BRACE);
    case '}':
        return __lexer_make_token__(lexer, TOKEN_R_BRACE);
    case '.':
        if (ISDIGIT(reader_peek(lexer->reader))) {
            return __lexer_parse_number__(lexer, ch);
        }
        if (reader_try(lexer->reader, '.')) {
            if (reader_try(lexer->reader, '.')) {
                return __lexer_make_token__(lexer, TOKEN_ELLIPSIS);
            }

            reader_unget(lexer->reader, '.');
            return __lexer_make_token__(lexer, TOKEN_PERIOD);
        }
        return __lexer_make_token__(lexer, TOKEN_PERIOD);
    case '&':
        if (reader_try(lexer->reader, '&'))
            return __lexer_make_token__(lexer, TOKEN_AMPAMP);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, TOKEN_AMPEQUAL);
        return __lexer_make_token__(lexer, TOKEN_AMP);
    case '*':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_STAREQUAL : TOKEN_STAR);
    case '+':
        if (reader_try(lexer->reader, '+'))	return __lexer_make_token__(lexer, TOKEN_PLUSPLUS);
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, TOKEN_PLUSEQUAL);
        return __lexer_make_token__(lexer, TOKEN_PLUS);
    case '-':
        if (reader_try(lexer->reader, '>')) return __lexer_make_token__(lexer, TOKEN_ARROW);
        if (reader_try(lexer->reader, '-')) return __lexer_make_token__(lexer, TOKEN_MINUSMINUS);
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, TOKEN_MINUSEQUAL);
        return __lexer_make_token__(lexer, TOKEN_MINUS);
    case '~':
        return __lexer_make_token__(lexer, TOKEN_TILDE);
    case '!':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EXCLAIM : TOKEN_EXCLAIMEQUAL);
    case '/':
        if (reader_test(lexer->reader, '/') || reader_test(lexer->reader, '*')) {
            __lexer_parse_comment__(lexer);
            return __lexer_make_token__(lexer, TOKEN_COMMENT);
        }
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH);
    case '%':
        if (reader_try(lexer->reader, '=')) return __lexer_make_token__(lexer, TOKEN_PERCENTEQUAL);
        if (reader_try(lexer->reader, '>')) return __lexer_make_token__(lexer, TOKEN_R_BRACE);
        if (reader_try(lexer->reader, ':')) {
            if (reader_try(lexer->reader, '%')) {
                if (reader_try(lexer->reader, ':'))
                    return __lexer_make_token__(lexer, TOKEN_HASHHASH);
                reader_unget(lexer->reader, '%');
            }
            return __lexer_make_token__(lexer, TOKEN_HASH);
        }
        return __lexer_make_token__(lexer, TOKEN_PERCENT);
    case '<':
        if (reader_try(lexer->reader, '<'))
            return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_LESSLESSEQUAL : TOKEN_LESSLESS);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, TOKEN_LESSEQUAL);
        if (reader_try(lexer->reader, ':'))
            return __lexer_make_token__(lexer, TOKEN_L_SQUARE);
        if (reader_try(lexer->reader, '%'))
            return __lexer_make_token__(lexer, TOKEN_L_BRACE);
        return __lexer_make_token__(lexer, TOKEN_LESS);
    case '>':
        if (reader_try(lexer->reader, '>'))
            return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_GREATERGREATEREQUAL : TOKEN_GREATERGREATER);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, TOKEN_GREATEREQUAL);
        return __lexer_make_token__(lexer, TOKEN_GREATER);
    case '^':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_CARETEQUAL : TOKEN_CARET);
    case '|':
        if (reader_try(lexer->reader, '|'))
            return __lexer_make_token__(lexer, TOKEN_PIPEPIPE);
        if (reader_try(lexer->reader, '='))
            return __lexer_make_token__(lexer, TOKEN_PIPEEQUAL);
        return __lexer_make_token__(lexer, TOKEN_PIPE);
    case '?':
        return __lexer_make_token__(lexer, TOKEN_QUESTION);
    case ':':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '>') ? TOKEN_R_SQUARE : TOKEN_COLON);
    case ';':
        return __lexer_make_token__(lexer, TOKEN_SEMI);
    case '=':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '=') ? TOKEN_EQUALEQUAL : TOKEN_EQUAL);
    case ',':
        return __lexer_make_token__(lexer, TOKEN_COMMA);
    case '#':
        return __lexer_make_token__(lexer, reader_try(lexer->reader, '#') ? TOKEN_HASHHASH : TOKEN_HASH);
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9':
        return __lexer_parse_number__(lexer, ch);
    case 'u': case 'U': case 'L': {
        encoding_type_t ent = __lexer_parse_encoding__(lexer, ch);

        if (reader_test(lexer->reader, '\"')) {
            __lexer_remark_loc__(lexer);
            reader_get(lexer->reader);
            return __lexer_parse_string__(lexer, ent);
        }

        if (ent != ENCODING_UTF8 && reader_test(lexer->reader, '\'')) {
            __lexer_remark_loc__(lexer);
            reader_get(lexer->reader);
            return __lexer_parse_character__(lexer, ent);
        }

        if (ent == ENCODING_UTF8) {
            reader_unget(lexer->reader, '8');
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
        return __lexer_make_token__(lexer, TOKEN_BACKSLASH);
    case EOF:
        return __lexer_make_token__(lexer, TOKEN_END);
    default:
        if (ISALPHA(ch) || (0x80 <= ch && ch <= 0xfd) || ch == '_' || ch == '$') {
            /* parse identifier */
            reader_unget(lexer->reader, ch);
            return __lexer_parse_identifier__(lexer);
        }
    }
    
    assert(false);
    return NULL;
}


array_t lexer_tokenize(lexer_t lexer, stream_type_t type, const unsigned char *s)
{
    size_t level;
    array_t tokens;
    token_t tok;

    level = reader_level(lexer->reader);

    if (!reader_push(lexer->reader, type, s)) {
        return NULL;
    }
    
    lexer_stash(lexer);
    tokens = array_create_n(sizeof(token_t), 2);
    for (;;) {
        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_END || reader_level(lexer->reader) <= level) {
            break;
        }

        tok = lexer_next(lexer);
        if (tok->type == TOKEN_NEWLINE) {
            token_destroy(tok);
            continue;
        }

        array_cast_append(token_t, tokens, tok);
    }
    lexer_unstash(lexer);
    return tokens;
}


token_t lexer_next(lexer_t lexer)
{
    token_t tok;
    array_t tokens;
    bool begin_of_line = false;
    size_t spaces = 0;

    tokens = __lexer_last_snapshot__(lexer);
    if (!array_is_empty(tokens)) {
        tok = array_cast_back(token_t, tokens);
        array_pop_back(tokens);
        return tok;
    }
    
    begin_of_line = reader_column(lexer->reader) == 1 ? true : false;

    tok = lexer_scan(lexer);
    while (tok->type == TOKEN_SPACE || 
           tok->type == TOKEN_COMMENT) {
        spaces += tok->spaces;
        token_destroy(tok);
        tok = lexer_scan(lexer);
    }

    tok->begin_of_line = begin_of_line;
    tok->spaces = spaces;
    return tok;
}


token_t lexer_peek(lexer_t lexer)
{
    token_t tok = lexer_next(lexer);
    if (tok->type != TOKEN_END) lexer_unget(lexer, tok);
    return tok;
}


bool lexer_try(lexer_t lexer, token_type_t tt)
{
    token_t tok = lexer_peek(lexer);
    if (tok->type == tt) {
        lexer_next(lexer);
        return true;
    }
    return false;
}


void lexer_unget(lexer_t lexer, token_t tok)
{
    assert(tok != NULL && 
           tok->type != TOKEN_END &&
           array_length(lexer->snapshots) > 0);

    array_cast_append(token_t, __lexer_last_snapshot__(lexer), tok);
}


bool lexer_is_empty(lexer_t lexer)
{
    token_t token = lexer_peek(lexer);
    if (token->type == TOKEN_END) return true;
    return false;
}


void lexer_stash(lexer_t lexer)
{
    __lexer_make_stash__(lexer->snapshots);
}


void lexer_unstash(lexer_t lexer)
{
    assert(array_length(lexer->snapshots) > 0);
    array_pop_back(lexer->snapshots);
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
token_t __lexer_parse_number__(lexer_t lexer, int ch)
{
    /* lexer's grammar on numbers is not strict. */
    int prev = -1;

#undef  VALID_SIGN
#define VALID_SIGN(c, prevc) \
  (((c) == '+' || (c) == '-') && \
   ((prevc) == 'e' || (prevc) == 'E' \
    || (((prevc) == 'p' || (prevc) == 'P') )))

    lexer->tok->cs = cstring_cat_ch(lexer->tok->cs, ch);

    for (;;) {
        ch = reader_peek(lexer->reader);
        if (!(ISIDNUM(ch) || ch == '.' || VALID_SIGN(ch, prev) || ch == '\'')) {
            break;
        }

        lexer->tok->cs = cstring_cat_ch(lexer->tok->cs, ch);

        prev = ch;

        reader_get(lexer->reader);
    }

#undef  VALID_SIGN

    return __lexer_make_token__(lexer, TOKEN_NUMBER);
}


static inline
int __lexer_parse_hex_escaped__(lexer_t lexer)
{
    int hex = 0, ch = reader_peek(lexer->reader);

    if (!ISHEX(ch)) {
        ERRORF_WITH_TOKEN("\\x used with no following hex digits");
    }

    while (ISHEX(ch)) {
        hex = (hex << 4) + TODIGIT(ch);
        reader_get(lexer->reader);
        ch = reader_peek(lexer->reader);
    }

    return hex;
}


static inline
bool __is_valid_universal_char__(unsigned int u)
{
    if (0xD800 <= u && u <= 0xDFFF)
        return false;
    return 0xA0 <= u || u == '$' || u == '@' || u == '`';
}


static inline
int __lexer_parse_universal_char__(lexer_t lexer, int len)
{
    int ch, i;
    unsigned int u = 0;

    assert(len == 4 || len == 8);

    for (i = 0; i < len; ++i) {
        ch = reader_get(lexer->reader);
        if (!ISHEX(ch)) {
            ERRORF_WITH_LOC("incomplete universal character name");
        }
        u = (u << 4) + TODIGIT(ch);
    }

    if (!__is_valid_universal_char__(u)) {
        ERRORF_WITH_LOC("\\%c%0*x is not a valid universal character", (len == 4) ? 'u' : 'U', len, u);
    }

    return u;
}


static inline
int __lexer_parse_oct_escaped__(lexer_t lexer, int ch)
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
int __lexer_parse_escaped__(lexer_t lexer)
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
    case 'x': return __lexer_parse_hex_escaped__(lexer);
    case 'u': return __lexer_parse_universal_char__(lexer, 4);
    case 'U': return __lexer_parse_universal_char__(lexer, 8);
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        return __lexer_parse_oct_escaped__(lexer, ch);
    }

    WARNINGF_WITH_LOC("unknown escape sequence: \'%c\'", ch);
    return ch;
}


static inline
bool __lexer_is_universal_char__(lexer_t lexer, int ch)
{
    return ch == '\\' && (reader_test(lexer->reader, 'u') || reader_test(lexer->reader, 'U'));
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
token_t __lexer_parse_character__(lexer_t lexer, encoding_type_t ent)
{
    int ch;

    for (; !reader_is_empty(lexer->reader) ;) {
        ch = reader_get(lexer->reader);
        if (ch == '\'' || ch == '\n') {
            break;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_universal_char__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer);
            if (isunc) {
                lexer->tok->cs = cstring_append_utf8(lexer->tok->cs, ch);
                continue;
            }
        }

        lexer->tok->cs = cstring_cat_ch(lexer->tok->cs, ch);
    }

    if (ch != '\'') {
        ERRORF_WITH_TOKEN("missing terminating ' character");
    } else if (cstring_length(lexer->tok->cs) == 0) {
        ERRORF_WITH_TOKEN("empty character constant");
    }
   
    return __lexer_make_token__(lexer, ent2tokt(ent, CHAR));
}


static inline 
token_t __lexer_parse_string__(lexer_t lexer, encoding_type_t ent)
{
    int ch;

    for (; !reader_is_empty(lexer->reader) ;) {
        ch = reader_get(lexer->reader);
        if (ch == '\"' || ch == '\n') {
            break;
        }

        if (ch == '\\') {
            bool isunc = __lexer_is_universal_char__(lexer, ch);
            ch = __lexer_parse_escaped__(lexer);
            if (isunc) {
                lexer->tok->cs = cstring_append_utf8(lexer->tok->cs, ch);
                continue;
            }
        }

        lexer->tok->cs = cstring_cat_ch(lexer->tok->cs, ch);
    }

    if (ch != '\"') {
        ERRORF_WITH_TOKEN("unterminated string literal");
    }

    return __lexer_make_token__(lexer, ent2tokt(ent, STRING));
}


static inline 
token_t __lexer_parse_identifier__(lexer_t lexer)
{
    int ch;

    for (;;) {
        ch = reader_get(lexer->reader);
        if (ISIDNUM(ch) || ch == '$' || (0x80 <= ch && ch <= 0xfd)) {
            lexer->tok->cs = cstring_cat_ch(lexer->tok->cs, ch);
            continue;
        }

        if (__lexer_is_universal_char__(lexer, ch)) {
            lexer->tok->cs = cstring_append_utf8(lexer->tok->cs, __lexer_parse_escaped__(lexer));
            continue;
        }

        break;
    }

    reader_unget(lexer->reader, ch);
    return __lexer_make_token__(lexer, TOKEN_IDENTIFIER);
}


static inline
bool __lexer_parse_white_space__(lexer_t lexer)
{
    int ch;

    for (;;) {
        ch = reader_peek(lexer->reader);
        if (!ISSPACE(ch) || reader_is_empty(lexer->reader) || ch == '\n') {
            break;
        }
        reader_get(lexer->reader);
        lexer->tok->spaces++;
    }

    return lexer->tok->spaces > 0;
}


static inline 
void __lexer_parse_comment__(lexer_t lexer)
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
        ERRORF_WITH_TOKEN("unterminated comment");
        return;
    }

    assert(false);
}


static inline 
void __lexer_make_stash__(array_t a)
{
    array_cast_append(array_t, a, array_create_n(sizeof(token_t), 12));
}


static inline 
array_t __lexer_last_snapshot__(lexer_t lexer)
{
    assert(array_length(lexer->snapshots) > 0);
    return array_cast_back(array_t, lexer->snapshots);
}


static inline
void __lexer_mark_loc__(lexer_t lexer)
{
    token_mark_loc(lexer->tok,
                   reader_line(lexer->reader), 
                   reader_column(lexer->reader),
                   reader_linenote(lexer->reader), 
                   reader_name(lexer->reader));
}


static inline
void __lexer_remark_loc__(lexer_t lexer)
{
    token_remark_loc(lexer->tok, 
                     reader_line(lexer->reader),
                     reader_column(lexer->reader), 
                     reader_linenote(lexer->reader));
}


static inline 
token_t __lexer_make_token__(lexer_t lexer, token_type_t type)
{
    token_t tok;

    lexer->tok->type = type;

    tok = token_dup(lexer->tok);

    token_init(lexer->tok);

    return tok;
}
