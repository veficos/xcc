

#include "config.h"
#include "unittest.h"
#include "cstring.h"
#include "set.h"


static void reset_sets(set_t *a, set_t *b)
{
    cstring_t cs;
    int i;

    set_clear(a);
    set_clear(b);

    for (i = 0; i < 10; i++) {
        cs = cstring_from_ll(i);
        set_add(a, cs);
        cstring_free(cs);
    }

    for (i = 5; i < 20; i++) {
        cs = cstring_from_ll(i);
        set_add(b, cs);
        cstring_free(cs);
    }
}


static void test_union(set_t *set)
{
    cstring_t cs;
    int i;
    for (i = 0; i < 20; i++) {
        cs = cstring_from_ll(i);
        TEST_COND("set", set_has(set, cs));
        cstring_free(cs);
    }
}


static void test_intersection(set_t *set)
{
    cstring_t cs;
    int i;
    for (i = 0; i < 20; i++) {
        cs = cstring_from_ll(i);
        if (i >= 5 && i < 10) {
            TEST_COND("set", set_has(set, cs));
        } else {
            TEST_COND("set", !set_has(set, cs));
        }
        cstring_free(cs);
    }
}


static void test_set(void)
{
    set_t *a, *b, *c, *d;

    a = set_create();
    b = set_create();

    TEST_COND("set_is_empty", set_is_empty(a) == true);
    TEST_COND("set_is_empty", set_is_empty(b) == true);

    reset_sets(a, b);
    set_concat_union(a, b);
    test_union(a);

    reset_sets(a, b);
    set_concat_intersection(a, b);
    test_intersection(a);

    reset_sets(a, b);

    c = set_union(a, b);
    test_union(c);
    set_destroy(c);

    c = set_intersection(a, b);
    test_intersection(c);

    d = set_dup(c);
    test_intersection(d);

    set_clear(d);

    TEST_COND("set_clear", set_is_empty(d) == true);

    set_destroy(d);
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
