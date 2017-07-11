

#include "config.h"
#include "cspool.h"
#include "unittest.h"


static void test_cspool(void)
{
    cspool_t *pool;
    cstring_t cs;

    pool = cspool_create();

    cs = cspool_push(pool, "HelloWorld");

    TEST_COND("cspool_push()", cspool_push(pool, "HelloWorld") == cs);
    cspool_pop(pool, "HelloWorld");
    TEST_COND("cspool_push()", cspool_push(pool, "HelloWorld") == cs);

    cspool_destroy(pool);
}


int main(void)
{

#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_cspool();
    TEST_REPORT();
    return 0;
}
