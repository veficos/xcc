

#include "cstring.h"
#include "unittest.h"


static 
void test_cstring(void)
{
    cstring_t cs, cs2;
    
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
    TEST_COND("cstring_from_ull()", memcmp(cs, "18446744073709551615", 21) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 20);
    cstring_destroy(cs);


    cs = cstring_create_n(NULL, 0);
    cs = cstring_cat_pf(cs, "%d, %lf, %s", 1024, 1.234, "abcd");
    TEST_COND("cstring_cat_pf()", memcmp(cs, "1024, 1.234000, abcd", 20) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 20);
    cstring_destroy(cs);

    cs = cstring_create("AA...AA.a.aa.aHelloWorld     :::");
    cs = cstring_trim(cs, "Aa. :");
    TEST_COND("cstring_trim()", memcmp(cs, "HelloWorld", 10) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    cstring_destroy(cs);

    cs = cstring_create("AA...AA.a.aa.aHe:llo World     :::");
    cs = cstring_trim_all(cs, "Aa. :");
    TEST_COND("cstring_trim_all()", memcmp(cs, "HelloWorld", 10) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    cstring_destroy(cs);
 
    cs = cstring_create("HelloWorld");
    cs2 = cstring_dup(cs);
    TEST_COND("cstring_dup()", cstring_cmp_cs(cs, cs2) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    TEST_COND("cstring_length()", cstring_length(cs2) == 10);
    cstring_tolower(cs);
    TEST_COND("cstring_tolower()", cstring_cmp(cs, "helloworld") == 0);
    cstring_toupper(cs);
    TEST_COND("cstring_toupper()", cstring_cmp(cs, "HELLOWORLD") == 0);
    cstring_destroy(cs);
    cstring_destroy(cs2);

    cs = cstring_create("HelloWorl");
    cs = cstring_push_ch(cs, 'd');
    TEST_COND("cstring_push_ch()", cstring_cmp(cs, "HelloWorld") == 0);
    TEST_COND("cstring_pop_ch()", cstring_pop_ch(cs) == 'd');
    TEST_COND("cstring_length()", cstring_length(cs) == 9);
    TEST_COND("cstring_capacity()", cstring_capacity(cs) == 11);
    cstring_destroy(cs);
}


int main(void)
{
    test_cstring();
    TEST_REPORT();
    return 0;
}
