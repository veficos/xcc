

#include "config.h"
#include "unittest.h"
#include "cstring.h"
#include "cstring_pool.h"


static
void test_cstring_pool(void)
{
    cstring_pool_t pool;
    cstring_t cs;

    pool = cstring_pool_create();

    cs = cstring_pool_push_n(pool, "HelloWorld", 10);

    printf("%s\n", cs);

    cstring_pool_destroy(pool);
}

int main(void)
{
    {
        #ifdef WIN32
        _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
        #endif
    }

    test_cstring_pool();
    return 0;
}
