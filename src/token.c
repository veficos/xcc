

#include "config.h"
#include "token.h"


struct token_dictionary_s {
    token_type_t type;
    const char *type2str;
    const char *token;
} token_dictionary[] = {
    { TOKEN_L_SQUARE,            "TOKEN_L_SQUARE",            "["        },
    { TOKEN_R_SQUARE,            "TOKEN_R_SQUARE",            "]"        },
    { TOKEN_L_PAREN,             "TOKEN_L_PAREN",             ")"        },
    { TOKEN_R_PAREN,             "TOKEN_R_PAREN",             "("        },
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
    { TOKEN_NEW_LINE,            "TOKEN_NEW_LINE",            "\\n"      },
    { TOKEN_SPACE,               "TOKEN_SPACE",               "spaces"   },
    { TOKEN_COMMENT,             "TOKEN_COMMENT",             "comment"  },
};

token_t token_create(void)
{
    token_t tok;
    cstring_t cs;
    source_location_t loc;

    if ((cs = cstring_create_n(NULL, DEFUALT_LITERALS_LENGTH)) == NULL) {
        goto done;
    }
    
    if ((loc = source_location_create()) == NULL) {
        goto clean_cs;
    }

    if ((tok = (token_t) pmalloc(sizeof(struct token_s))) == NULL) {
        goto clean_loc;
    }

    tok->type = TOKEN_UNKNOWN;
    tok->literals = cs;
    tok->location = loc;
    return tok;

clean_loc:
    source_location_destroy(loc);

clean_cs:
    cstring_destroy(cs);

done:
    return NULL;
}


void token_destroy(token_t tok)
{
    assert(tok);

    if (tok->literals != NULL) {
        cstring_destroy(tok->literals);
    }

    source_location_destroy(tok->location);

    pfree(tok);
}


void token_init(token_t token)
{
    token->type = TOKEN_UNKNOWN;

    cstring_clear(token->literals);

    source_location_init(token->location, 0, 0, NULL, NULL);
}


token_t token_dup(token_t tok)
{
    token_t ret;

    if ((ret = pmalloc(sizeof(struct token_s))) == NULL) {
        goto done;
    }
    
    ret->type = tok->type;
    ret->literals = NULL;

    if ((tok->literals != NULL && cstring_length(tok->literals) != 0) &&
        ((ret->literals = cstring_dup(tok->literals)) == NULL)) {
        goto clean_tok;
    }

    if ((ret->location = source_location_dup(tok->location)) == NULL) {
        goto clean_literals;
    }

    return ret;

clean_literals:
    cstring_destroy(ret->literals);

clean_tok:
    pfree(ret);

done:
    return NULL;
}


const char *token_type2str(token_t tok)
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


source_location_t source_location_create(void)
{
    source_location_t loc;
    if ((loc = (source_location_t) pmalloc(sizeof(struct source_location_s))) == NULL) {
        return NULL;
    }

    loc->current_line = NULL;
    loc->filename = NULL;
    loc->line = 0;
    loc->column = 0;
    return loc;
}


void source_location_destroy(source_location_t loc)
{
    assert(loc != NULL);

    if (loc->filename != NULL) {
        cstring_destroy(loc->filename);
    }

    if (loc->current_line != NULL) {
        cstring_destroy(loc->current_line);
    }

    pfree(loc);
}


source_location_t source_location_dup(source_location_t loc)
{
    source_location_t ret;

    if ((ret = (source_location_t) pmalloc(sizeof(struct source_location_s))) == NULL) {
        return NULL;
    }

    ret->current_line = loc->current_line;
    ret->filename = loc->filename;
    ret->line = loc->line;
    ret->column = loc->column;
    return ret;
}

