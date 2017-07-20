

#include "config.h"
#include "token.h"
#include "cstring.h"
#include "reader.h"
#include "lexer.h"
#include "dict.h"
#include "unittest.h"


static void test_restore_text(void)
{
    lexer_t *lexer;
    array_t *tokens;
    cstring_t cs;

    lexer = lexer_create();
    tokens = array_create(sizeof(token_t*));

    lexer_push(lexer, STREAM_TYPE_STRING, "#   include < stdio.h > u\"H\" ");

    while (true) {
        token_t *token = lexer_scan(lexer);
        if (token->type == TOKEN_EOF) {
            token_destroy(token);
            break;
        }

        array_cast_append(token_t*, tokens, token);
    }

    cs = tokens_to_text(tokens);

    printf("%s", cs);
    TEST_COND("tokens_to_text()", cstring_compare(cs, "#   include < stdio.h > u\"H\" \n")==0);

    lexer_destroy(lexer);
}

static void test_lexer(void)
{
    lexer_t *lexer;
    array_t *tokens;


    lexer = lexer_create();

    lexer_push(lexer, STREAM_TYPE_STRING, "#include<stdio.h>/*");

    while (true) {
        token_t *token = lexer_scan(lexer);

        if (token->type == TOKEN_END) {
            token_destroy(token);
            break;
        }

        if (token->type == TOKEN_COMMENT) {
            printf("%s\n", token->cs);
        } else {
            printf("%s\n", token_as_name(token));
        }
    }

    lexer_destroy(lexer);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_restore_text();
    //test_lexer();

    TEST_REPORT();
    return 0;
}
