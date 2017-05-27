

#include "config.h"
#include "diagnostor.h"
#include "token.h"
#include "cstring.h"


static 
void test_diagnostor(void)
{

}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_diagnostor();
    return 0;
}
