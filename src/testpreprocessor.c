

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
    for (;;) {
        token_t tok = preprocessor_next(pp);
        if (tok->type == TOKEN_END)
            break;

        if (tok->type == TOKEN_NEWLINE) {
            printf("\n");
            continue;
        }
        
        if (tok->spaces)
            while (tok->spaces--)
                printf(" ");

        printf("%s", tok2s(tok));
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
