

#include "config.h"
#include "unittest.h"
#include "preprocessor.h"


static
void test_preprocessor(void)
{
    preprocessor_t pp;

    pp = preprocessor_create();

    preprocessor_add_include_path(pp, "/usr/include");

    preprocessor_destroy(pp);
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
