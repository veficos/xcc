

#include "config.h"
#include "unittest.h"
#include "diag.h"
#include "lexer.h"
#include "option.h"
#include "reader.h"
#include "preprocessor.h"


static
void test_preprocessor(void)
{
    preprocessor_t pp;
    reader_t reader;
    diag_t diag;
    lexer_t lexer;
    struct option_s option;

    diag = diag_create();

    reader = reader_create(&option, diag);

    reader_push(reader, STREAM_TYPE_FILE, "6.h");


    lexer = lexer_create(reader, &option, diag);

    pp = preprocessor_create(lexer, &option, diag);

    preprocessor_add_include_path(pp, "/usr/include");

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
