

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
    screader_t screader;
    diag_t diag;
    lexer_t lexer;
    struct option_s option;
    const char *p;
    token_t  tok;

    screader = screader_create(STREAM_TYPE_FILE, "3.h");
    diag = diag_create();

    //screader_push(screader, READER_TYPE_FILE, "2.h");
    
    lexer = lexer_create(screader, &option, diag);
    while (tok = lexer_scan(lexer)) {
        if (tok->type == TOKEN_END) {
            token_destroy(tok);
            break;
        }

        // printf("current line: %s\n"
        //        "filename: %s\n"
        //        "line: %d\n"
        //        "column: %d\n"
        //        "token: %s\n", 
        //        tok->location->current_line,
        //        tok->location->filename, 
        //        tok->location->line, 
        //        tok->location->column,
        //        tok->literals ? tok->literals : "unkown");

        p = token_type2str(tok);

        printf("token: %s\n"
               "line: %d\n"
               "column: %d\n",
               p ? p : tok->literals, tok->location->line, tok->location->column);
        
        token_destroy(tok);
    }


    lexer_destroy(lexer); 

    diag_destroy(diag);

    screader_destroy(screader);
}


int main(void)
{

#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_lexer();

    return 0;
}
