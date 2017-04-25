

#include "config.h"
#include "unittest.h"
#include "diag.h"
#include "lexer.h"
#include "option.h"
#include "token.h"
#include "reader.h"
#include "preprocessor.h"


static 
void print_pp(preprocessor_t pp)
{
    unsigned int spaces;
    for (;;) {
        token_t tok = preprocessor_next(pp);
        if (tok->type == TOKEN_END) {
            token_destroy(tok);
            break;
        }

        if (tok->type == TOKEN_NEWLINE) {
            token_destroy(tok);
            printf("\n");
            continue;
        }

        spaces = tok->spaces;
        while (spaces--)
            printf(" ");

        printf("%s", tok2s(tok));
        token_destroy(tok);
    }

    printf("\n");
}


static
void test_preprocessor(void)
{
    preprocessor_t pp;
    reader_t reader;
    diag_t diag;
    lexer_t lexer;
    struct option_s option;

    diag = diag_create();

    reader = reader_create(diag, &option);

    reader_push(reader, STREAM_TYPE_FILE, "1.c");


    lexer = lexer_create(reader, &option, diag);

    pp = preprocessor_create(lexer, &option, diag);

    preprocessor_add_include_path(pp, "/usr/include");

    print_pp(pp);

    preprocessor_destroy(pp);

    lexer_destroy(lexer);
    reader_destroy(reader);
    diag_destroy(diag);
}




int main(void)
{
{
    #ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    #endif
}

    test_preprocessor();
    return 0;
}
