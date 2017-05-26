

#include "config.h"
#include "unittest.h"
#include "cstring.h"
#include "map.h"


void map_scan_f(void *privdata, const void *key, const void *value)
{
    cstring_t cs = cstring_from_ll((long long) value);
    TEST_COND("key", cstring_compare((const cstring_t)key, (const cstring_t)cs) == 0);
    cstring_free(cs);
}


static void testmap(void)
{
    cstring_t cs;
    map_t *map;
    int i;

    map = map_create();

    for (i = 0; i < 10; i++) {
        cs = cstring_from_ll(i);
        map_add(map, cs, (void*) i);
        cstring_free(cs);
    }

    map_scan(map, map_scan_f, NULL);

    cs = cstring_from_ll(1);
    map_del(map, cs);
    TEST_COND("map_del", map_has(map, cs) == false);
    cstring_free(cs);

    map_destroy(map);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    testmap();
    TEST_REPORT();
    return 0;
}
