

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

    token_t  tok;

    reader = reader_create(READER_TYPE_FILE, "C:\\Users\\sshtc\\Desktop\\occ_vs\\1.c");
    diag = diag_create();
    
    lexer = lexer_create(reader, &option, diag);
    while (tok = lexer_scan(lexer)) {
        if (tok->type == TOKEN_END) {
            token_destroy(tok);
            break;
        }

        printf("current line: %s\n"
               "filename: %s\n"
               "line: %d\n"
               "column: %d\n"
               "token: %s\n", 
               tok->location->current_line,
               tok->location->filename, 
               tok->location->line, 
               tok->location->column,
               tok->literals ? tok->literals : "unkown");

        token_destroy(tok);
    }


    lexer_destroy(lexer); 

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
