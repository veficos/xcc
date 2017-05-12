

#include "config.h"
#include "token.h"


struct token_dictionary_s {
    token_type_t type;
    const char *type2str;
    const char *original;
} token_dictionary[] = {
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
    { TOKEN_NEWLINE,             "TOKEN_NEWLINE",             "\\n"      },
    { TOKEN_SPACE,               "TOKEN_SPACE",               "spaces"   },
    { TOKEN_COMMENT,             "TOKEN_COMMENT",             "comment"  },
    { TOKEN_EOF,                 "TOKEN_EOF",                 "eof"      },
};


token_t token_create(void)
{
    token_t tok = (token_t) pmalloc(sizeof(struct token_s));
    tok->type = TOKEN_UNKNOWN;
    tok->cs = cstring_create_n(NULL, DEFUALT_LITERALS_LENGTH);
    tok->loc = source_location_create();
    tok->hideset = NULL;
    tok->spaces = 0;
    tok->is_vararg = false;
    return tok;
}


void token_destroy(token_t token)
{
    assert(token != NULL);

    source_location_destroy(token->loc);

    if (token->hideset != NULL) set_destroy(token->hideset);

    cstring_destroy(token->cs);

    pfree(token);
}


void token_init(token_t token)
{
    cstring_clear(token->cs);

    if (token->hideset != NULL) set_destroy(token->hideset);

    token->type = TOKEN_UNKNOWN;

    token->hideset = NULL;
    
    token->spaces = 0;

    token->begin_of_line = false;

    token->is_vararg = false;

    source_location_mark(token->loc, 0, 0, NULL, NULL);
}


token_t token_dup(token_t tok)
{
    token_t ret;
    cstring_t cs = NULL;

    ret = pmalloc(sizeof(struct token_s));

    ret->type = tok->type;
    ret->hideset = tok->hideset ? set_dup(tok->hideset) : NULL;// set_dup(tok->hideset);
    ret->begin_of_line = tok->begin_of_line;
    ret->spaces = tok->spaces;
    ret->cs = cstring_dup(tok->cs);
    ret->loc = source_location_dup(tok->loc);
    ret->is_vararg = false;
    return ret;
}


const char *tok2id(token_t tok)
{
    size_t i, length;
    length = sizeof(token_dictionary) / sizeof(struct token_dictionary_s);

    for (i = 0; i < length; i++) {
        if (token_dictionary[i].type == tok->type) {
            return token_dictionary[i].type2str;
        }
    }

    return NULL;
}


const char *tok2s(token_t tok)
{
    size_t i, length;
    length = sizeof(token_dictionary) / sizeof(struct token_dictionary_s);

    for (i = 0; i < length; i++) {
        if (token_dictionary[i].type == tok->type) {
            return token_dictionary[i].original;
        }
    }
    return tok->cs;
}


source_location_t source_location_create(void)
{
    source_location_t loc = pmalloc(sizeof(struct source_location_s));
    loc->linenote = NULL;
    loc->fn = NULL;
    loc->line = 0;
    loc->column = 0;
    return loc;
}


void source_location_destroy(source_location_t loc)
{
    assert(loc != NULL);
    pfree(loc);
}


source_location_t source_location_dup(source_location_t loc)
{
    source_location_t ret = pmalloc(sizeof(struct source_location_s));
    ret->linenote = loc->linenote;
    ret->fn = loc->fn;
    ret->line = loc->line;
    ret->column = loc->column;
    return ret;
}
