

#include "config.h"
#include "token.h"
#include "cstring.h"
#include "reader.h"
#include "diag.h"
#include "option.h"
#include "lexer.h"
#include "dict.h"


static
void test_lexer(void)
{
    reader_t reader;
    diag_t diag;
    lexer_t lexer;
    struct option_s option;
    const char *p;
    token_t  tok;


    diag = diag_create();

    reader = reader_create(&option, diag);

    reader_push(reader, STREAM_TYPE_FILE, "5.h");

    lexer = lexer_create(reader, &option, diag);

    lexer_peek(lexer);
    while (tok = lexer_next(lexer)) {
        if (tok->type == TOKEN_END) {
            token_destroy(tok);
            break;
        }

        p = token_type2str(tok);

        printf("token: %s\n"
               "line: %d\n"
               "column: %d\n",
               p ? p : tok->literals, tok->loc->line, tok->loc->column);
        
        token_destroy(tok);
    }

    lexer_destroy(lexer); 

    diag_report(diag);
    diag_destroy(diag);

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
