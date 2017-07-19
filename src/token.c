

#include "config.h"
#include "token.h"


typedef struct token_dictionary_s {
    token_type_t type;
    const char *text;
    const char *name;
} token_dictionary_t;

token_dictionary_t __token_dictionary__[] = {
    { TOKEN_L_SQUARE,            "TOKEN_L_SQUARE",            "["        },
    { TOKEN_R_SQUARE,            "TOKEN_R_SQUARE",            "]"        },
    { TOKEN_L_PAREN,             "TOKEN_L_PAREN",             "("        },
    { TOKEN_R_PAREN,             "TOKEN_R_PAREN",             ")"        },
    { TOKEN_L_BRACE,             "TOKEN_L_BRACE",             "{"        },
    { TOKEN_R_BRACE,             "TOKEN_R_BRACE",             "}"        },
    { TOKEN_PERIOD,              "TOKEN_PERIOD",              "."        },
    { TOKEN_ELLIPSIS,            "TOKEN_ELLIPSIS",            "..."      },
    { TOKEN_AMP,                 "TOKEN_AMP",                 "&"        },
    { TOKEN_AMPAMP,              "TOKEN_AMPAMP",              "&&"       },
    { TOKEN_AMPEQUAL,            "TOKEN_AMPEQUAL",            "&="       },
    { TOKEN_STAR,                "TOKEN_STAR",                "*"        },
    { TOKEN_STAREQUAL,           "TOKEN_STAREQUAL",           "*="       },
    { TOKEN_PLUS,                "TOKEN_PLUS",                "+"        },
    { TOKEN_PLUSPLUS,            "TOKEN_PLUSPLUS",            "++"       },
    { TOKEN_PLUSEQUAL,           "TOKEN_PLUSEQUAL",           "+="       },
    { TOKEN_MINUS,               "TOKEN_MINUS",               "-"        },
    { TOKEN_MINUSMINUS,          "TOKEN_MINUSMINUS",          "--"       },
    { TOKEN_MINUSEQUAL,          "TOKEN_MINUSEQUAL",          "-="       },
    { TOKEN_ARROW,               "TOKEN_ARROW",               "->"       },
    { TOKEN_TILDE,               "TOKEN_TILDE",               "~"        },
    { TOKEN_EXCLAIM,             "TOKEN_EXCLAIM",             "!"        },
    { TOKEN_EXCLAIMEQUAL,        "TOKEN_EXCLAIMEQUAL",        "!="       },
    { TOKEN_SLASH,               "TOKEN_SLASH",               "/"        },
    { TOKEN_SLASHEQUAL,          "TOKEN_SLASHEQUAL",          "/="       },
    { TOKEN_PERCENT,             "TOKEN_PERCENT",             "%"        },
    { TOKEN_PERCENTEQUAL,        "TOKEN_PERCENTEQUAL",        "%="       },
    { TOKEN_LESS,                "TOKEN_LESS",                "<"        },
    { TOKEN_LESSLESS,            "TOKEN_LESSLESS",            "<<"       },
    { TOKEN_LESSLESSEQUAL,       "TOKEN_LESSLESSEQUAL",       "<<="      },
    { TOKEN_LESSEQUAL,           "TOKEN_LESSEQUAL",           "<="       },
    { TOKEN_GREATER,             "TOKEN_GREATER",             ">"        },
    { TOKEN_GREATERGREATER,      "TOKEN_GREATERGREATER",      ">>"       },
    { TOKEN_GREATEREQUAL,        "TOKEN_GREATEREQUAL",        ">="       },
    { TOKEN_GREATERGREATEREQUAL, "TOKEN_GREATERGREATEREQUAL", ">>="      },
    { TOKEN_CARET,               "TOKEN_CARET",               "^"        },
    { TOKEN_CARETEQUAL,          "TOKEN_CARETEQUAL",          "^="       },
    { TOKEN_PIPE,                "TOKEN_PIPE",                "|"        },
    { TOKEN_PIPEPIPE,            "TOKEN_PIPEPIPE",            "||"       },
    { TOKEN_PIPEEQUAL,           "TOKEN_PIPEEQUAL",           "|="       },
    { TOKEN_QUESTION,            "TOKEN_QUESTION",            "?"        },
    { TOKEN_COLON,               "TOKEN_COLON",               ":"        },
    { TOKEN_SEMI,                "TOKEN_SEMI",                ";"        },
    { TOKEN_EQUAL,               "TOKEN_EQUAL",               "="        },
    { TOKEN_EQUALEQUAL,          "TOKEN_EQUALEQUAL",          "=="       },
    { TOKEN_COMMA,               "TOKEN_COMMA",               ","        },
    { TOKEN_HASH,                "TOKEN_HASH",                "#"        },
    { TOKEN_HASHHASH,            "TOKEN_HASHHASH",            "##"       },
    { TOKEN_BACKSLASH,           "TOKEN_BACKSLASH",           "\\"       },
    { TOKEN_NEWLINE,             "TOKEN_NEWLINE",             "\n"       },
    { TOKEN_SPACE,               "TOKEN_SPACE",               ""         },
    { TOKEN_COMMENT,             "TOKEN_COMMENT",             ""         },
    { TOKEN_EOF,                 "TOKEN_EOF",                 ""         },
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

    for (i = 0; i < length; i++) {
        if (__token_dictionary__[i].type == token->type) {
            return __token_dictionary__[i].name;
        }
    }

    return token->cs;
}


void token_add_linenote_caution(token_t *token, size_t start, size_t length)
{
    token->location.linenote_caution.start = start;
    token->location.linenote_caution.length = length;
}


cstring_t token_restore_text(array_t *tokens)
{
    token_t **toks;
    cstring_t cs;
    int i, j;

    cs = cstring_new_n(NULL, array_length(tokens) * 8);

    array_foreach(tokens, toks, i) {
        int spaces = toks[i]->spaces;
        const char *s;

        while (spaces--) {
            cs = cstring_push_ch(cs, ' ');
        }

        s = token_as_text(toks[i]);
        cs = cstring_concat_n(cs, s, strlen(s));
    }

    return cs;
}