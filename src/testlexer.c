

#include "config.h"
#include "token.h"
#include "cstring.h"
#include "reader.h"
#include "diagnostor.h"
#include "option.h"
#include "lexer.h"
#include "dict.h"


static void test_lexer(void)
{
    lexer_t *lexer;

    lexer = lexer_create();


    lexer_destroy(lexer);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_lexer();

    return 0;
}
