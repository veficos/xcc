

#include "config.h"
#include "token.h"


typedef struct token_dictionary_s {
    token_type_t type;
    const char *text;
    const char *name;
} token_dictionary_t;

#define TOKEN_DICTIONARY_ITEM(t, s) \
    {t, #t, s}

token_dictionary_t __token_dictionary__[] = {
    TOKEN_DICTIONARY_ITEM(TOKEN_UNKNOWN,            "<UNKNOWN>"),
    TOKEN_DICTIONARY_ITEM(TOKEN_EOF,                "<EOF>"),
    TOKEN_DICTIONARY_ITEM(TOKEN_END,                "<END>"),

    TOKEN_DICTIONARY_ITEM(TOKEN_BACKSLASH,          "\\"),
    TOKEN_DICTIONARY_ITEM(TOKEN_NEWLINE,            "\n"),
    TOKEN_DICTIONARY_ITEM(TOKEN_SPACE,              ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_COMMENT,            ""),

    TOKEN_DICTIONARY_ITEM(TOKEN_L_SQUARE,            "["),
    TOKEN_DICTIONARY_ITEM(TOKEN_R_SQUARE,            "]"),
    TOKEN_DICTIONARY_ITEM(TOKEN_L_PAREN,             "("),
    TOKEN_DICTIONARY_ITEM(TOKEN_R_PAREN,             ")"),
    TOKEN_DICTIONARY_ITEM(TOKEN_L_BRACE,             "{"),
    TOKEN_DICTIONARY_ITEM(TOKEN_R_BRACE,             "}"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PERIOD,              "."),
    TOKEN_DICTIONARY_ITEM(TOKEN_ELLIPSIS,            "..."),
    TOKEN_DICTIONARY_ITEM(TOKEN_AMP,                 "&"),
    TOKEN_DICTIONARY_ITEM(TOKEN_AMPAMP,              "&&"),
    TOKEN_DICTIONARY_ITEM(TOKEN_AMPEQUAL,            "&="),
    TOKEN_DICTIONARY_ITEM(TOKEN_STAR,                "*"),
    TOKEN_DICTIONARY_ITEM(TOKEN_STAREQUAL,           "*="),
    TOKEN_DICTIONARY_ITEM(TOKEN_PLUS,                "+"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PLUSPLUS,            "++"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PLUSEQUAL,           "+="),
    TOKEN_DICTIONARY_ITEM(TOKEN_MINUS,               "-"),
    TOKEN_DICTIONARY_ITEM(TOKEN_MINUSMINUS,          "--"),
    TOKEN_DICTIONARY_ITEM(TOKEN_MINUSEQUAL,          "-="),
    TOKEN_DICTIONARY_ITEM(TOKEN_ARROW,               "->"),
    TOKEN_DICTIONARY_ITEM(TOKEN_TILDE,               "~"),
    TOKEN_DICTIONARY_ITEM(TOKEN_EXCLAIM,             "!"),
    TOKEN_DICTIONARY_ITEM(TOKEN_EXCLAIMEQUAL,        "!="),
    TOKEN_DICTIONARY_ITEM(TOKEN_SLASH,               "/"),
    TOKEN_DICTIONARY_ITEM(TOKEN_SLASHEQUAL,          "/="),
    TOKEN_DICTIONARY_ITEM(TOKEN_PERCENT,             "%"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PERCENTEQUAL,        "%="),
    TOKEN_DICTIONARY_ITEM(TOKEN_LESS,                "<"),
    TOKEN_DICTIONARY_ITEM(TOKEN_LESSLESS,            "<<"),
    TOKEN_DICTIONARY_ITEM(TOKEN_LESSLESSEQUAL,       "<<="),
    TOKEN_DICTIONARY_ITEM(TOKEN_LESSEQUAL,           "<="),
    TOKEN_DICTIONARY_ITEM(TOKEN_GREATER,             ">"),
    TOKEN_DICTIONARY_ITEM(TOKEN_GREATERGREATER,      ">>"),
    TOKEN_DICTIONARY_ITEM(TOKEN_GREATEREQUAL,        ">="),
    TOKEN_DICTIONARY_ITEM(TOKEN_GREATERGREATEREQUAL, ">>="),
    TOKEN_DICTIONARY_ITEM(TOKEN_CARET,               "^"),
    TOKEN_DICTIONARY_ITEM(TOKEN_CARETEQUAL,          "^="),
    TOKEN_DICTIONARY_ITEM(TOKEN_PIPE,                "|"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PIPEPIPE,            "||"),
    TOKEN_DICTIONARY_ITEM(TOKEN_PIPEEQUAL,           "|="),
    TOKEN_DICTIONARY_ITEM(TOKEN_QUESTION,            "?"),
    TOKEN_DICTIONARY_ITEM(TOKEN_COLON,               ":"),
    TOKEN_DICTIONARY_ITEM(TOKEN_SEMI,                ";"),
    TOKEN_DICTIONARY_ITEM(TOKEN_EQUAL,               "="),
    TOKEN_DICTIONARY_ITEM(TOKEN_EQUALEQUAL,          "=="),
    TOKEN_DICTIONARY_ITEM(TOKEN_COMMA,               ","),
    TOKEN_DICTIONARY_ITEM(TOKEN_HASH,                "#"),
    TOKEN_DICTIONARY_ITEM(TOKEN_HASHHASH,            "##"),

    TOKEN_DICTIONARY_ITEM(TOKEN_NUMBER,              ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_IDENTIFIER,          ""),

    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_STRING,     ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_WSTRING,    ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_STRING16,   ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_STRING32,   ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_UTF8STRING, ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_CHAR,       ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_WCHAR,      ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_CHAR16,     ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_CHAR32,     ""),
    TOKEN_DICTIONARY_ITEM(TOKEN_CONSTANT_UTF8CHAR,   ""),
};


token_t* token_create(token_type_t type, cstring_t cs, token_location_t *location)
{
    token_t *token = (token_t*) pmalloc(sizeof(token_t));

    if (location) {
        token->location = *location;

    } else {
        token->location.filename = NULL;
        token->location.linenote = NULL;
        token->location.line = 0;
        token->location.column = 0;
        token->location.linenote_caution.start = 0;
        token->location.linenote_caution.length = 0;
    }

    token->type = type;
    token->cs = cs;

    token->hideset = NULL;
    token->begin_of_line = false;
    token->spaces = 0;
    token->is_vararg = false;

    return token;
}


void token_destroy(token_t *token)
{
    assert(token != NULL);

//    source_location_destroy(token->loc);

    if (token->hideset != NULL) {
        set_destroy(token->hideset);
    }

    if (token->cs) {
        cstring_free(token->cs);
    }

    pfree(token);
}


void token_init(token_t *token)
{
    cstring_clear(token->cs);

    if (token->hideset != NULL) set_destroy(token->hideset);

    token->type = TOKEN_UNKNOWN;

    token->hideset = NULL;
    
    token->spaces = 0;

    token->begin_of_line = false;

    token->is_vararg = false;

    //source_location_mark(token->loc, 0, 0, NULL, NULL);
}


token_t* token_copy(token_t *tok)
{
    token_t* ret;
    cstring_t cs = NULL;

    ret = pmalloc(sizeof(token_t));

    ret->type = tok->type;
    ret->hideset = tok->hideset ? set_dup(tok->hideset) : NULL;// set_dup(tok->hideset);
    ret->begin_of_line = tok->begin_of_line;
    ret->spaces = tok->spaces;
    ret->cs = cstring_dup(tok->cs);
    ret->is_vararg = false;

    return ret;
}


const char* token_as_name(token_t *token)
{
    size_t i, length;
    length = sizeof(__token_dictionary__) / sizeof(struct token_dictionary_s);

    for (i = 0; i < length; i++) {
        if (__token_dictionary__[i].type == token->type) {
            return __token_dictionary__[i].text;
        }
    }

    return token->cs;
}


const char* token_as_text(token_t *token)
{
    size_t i, length;
    length = sizeof(__token_dictionary__) / sizeof(struct token_dictionary_s);

    if (token->cs && cstring_length(token->cs)) {
        return token->cs;
    }

    for (i = 0; i < length; i++) {
        if (__token_dictionary__[i].type == token->type) {
            return __token_dictionary__[i].name;
        }
    }

    return NULL;
}


void token_add_linenote_caution(token_t *token, size_t start, size_t length)
{
    token->location.linenote_caution.start = start;
    token->location.linenote_caution.length = length;
}


cstring_t tokens_to_text(array_t *tokens)
{
    token_t **toks;
    cstring_t cs;
    int i, j;

    cs = cstring_new_n(NULL, array_length(tokens) * 8);

    array_foreach(tokens, toks, i) {
        int spaces;
        const char *s;

        if (toks[i]->type == TOKEN_UNKNOWN ||
                toks[i]->type == TOKEN_EOF ||
                toks[i]->type == TOKEN_END) {
            cstring_free(cs);
            return NULL;
        }

        spaces  = toks[i]->spaces;
        while (spaces--) {
            cs = cstring_push_ch(cs, ' ');
        }

        s = token_as_text(toks[i]);
        cs = cstring_concat_n(cs, s, strlen(s));
    }

    return cs;
}


void tokens_free(array_t *tokens)
{
    token_t **toks;
    int i;

    array_foreach(tokens, toks, i) {
        token_destroy(toks[i]);
    }

    array_destroy(tokens);
}