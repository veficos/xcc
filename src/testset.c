

#include "config.h"
#include "unittest.h"
#include "cstring.h"
#include "dict.h"
#include "set.h"


static
void test_set(void)
{
    set_t a, b, c;
    cstring_t cs;
    int i;

    a = set_create();
    b = set_create();

    for (i = 0; i < 10; i++) {
        cs = cstring_from_ll(i);
        set_add(a, cs);
        cstring_destroy(cs);
    }

    for (i = 5; i < 20; i++) {
        cs = cstring_from_ll(i);
        set_add(b, cs);
        cstring_destroy(cs);
    }

    c = set_union(a, b);
    for (i = 0; i < 20; i++) {
        cs = cstring_from_ll(i);
        TEST_COND("set", set_has(c, cs));
        cstring_destroy(cs);
    }
    set_destroy(c);

    c = set_intersection(a, b);
    for (i = 0; i < 20; i++) {
        cs = cstring_from_ll(i);
        if (i >= 5 && i < 10) {
            TEST_COND("set", set_has(c, cs));
        } else {
            TEST_COND("set", !set_has(c, cs));
        }
        cstring_destroy(cs);
    }
    set_destroy(c);

    set_destroy(a);
    set_destroy(b);
}


int main(void)
{

#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_set();
    TEST_REPORT();
    return 0;
}
