

#include "config.h"
#include "token.h"
#include "cstring.h"
#include "reader.h"
#include "diagnostor.h"
#include "option.h"
#include "lexer.h"
#include "dict.h"


static
void test_lexer(void)
{
    reader_t *reader;
    lexer_t *lexer;
    const char *p;
    token_t *tok;

    reader = reader_create();

    lexer = lexer_create(reader);
    lexer_push(lexer, STREAM_TYPE_FILE, "3.c");
    lexer_push(lexer, STREAM_TYPE_STRING, "1.c");

    while (tok = lexer_get(lexer)) {
        if (tok->type == TOKEN_END) {
            token_destroy(tok);
            break;
        }

        p = tok2id(tok);

        printf("token: %s\n"
               "line: %d\n"
               "column: %d\n",
               p ? p : tok->cs, tok->location.line, tok->location.column);
        
        token_destroy(tok);
    }

    lexer_destroy(lexer);
    reader_destroy(reader);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_lexer();

    return 0;
}
