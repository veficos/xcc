

#include "cstring.h"
#include "unittest.h"


static 
void test_cstring(void)
{
    cstring_t cs;
    
    cs = cstring_create("");
    TEST_COND("cstring_length()", cstring_length(cs) == 0);
    cstring_destroy(cs);

    cs = cstring_from_ll(-1);
    TEST_COND("cstring_from_ll()", memcmp(cs, "-1", 3) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 2);
    cstring_destroy(cs);
    
    cs = cstring_from_ll(0x7fffffffffffffffl);
    TEST_COND("cstring_from_ll()", memcmp(cs, "9223372036854775807", 20) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 19);
    cstring_destroy(cs);
    
    cs = cstring_from_ull(-1, 10);
    printf("%s\n", cs);
    TEST_COND("cstring_from_ull()", memcmp(cs, "18446744073709551615", 21) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 20);
    cstring_destroy(cs);

    TEST_REPORT();
}


int main(void)
{
    test_cstring();
    return 0;
}
