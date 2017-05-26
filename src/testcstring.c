

#include "cstring.h"
#include "unittest.h"


static void test_cstring(void)
{
    cstring_t cs, cs2;
    
    cs = cstring_new("");
    TEST_COND("cstring_length()", cstring_length(cs) == 0);
    cstring_free(cs);

    cs = cstring_from_ll(-1);
    TEST_COND("cstring_from_ll()", memcmp(cs, "-1", 3) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 2);
    cstring_free(cs);
    
    cs = cstring_from_ll(0x7fffffffffffffffl);
    TEST_COND("cstring_from_ll()", memcmp(cs, "9223372036854775807", 20) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 19);
    cstring_free(cs);
    
    cs = cstring_from_ull(-1, 10);
    TEST_COND("cstring_from_ull()", memcmp(cs, "18446744073709551615", 21) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 20);
    cstring_free(cs);


    cs = cstring_new_n(NULL, 0);
    cs = cstring_concat_pf(cs, "%d, %lf, %s", 1024, 1.234, "abcd");
    TEST_COND("cstring_concat_pf()", memcmp(cs, "1024, 1.234000, abcd", 20) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 20);
    cstring_free(cs);

    cs = cstring_new("AA...AA.a.aa.aHelloWorld     :::");
    cs = cstring_trim(cs, "Aa. :");
    TEST_COND("cstring_trim()", memcmp(cs, "HelloWorld", 10) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    cstring_free(cs);

    cs = cstring_new("AA...AA.a.aa.aHe:llo World     :::");
    cs = cstring_trim_all(cs, "Aa. :");
    TEST_COND("cstring_trim_all()", memcmp(cs, "HelloWorld", 10) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    cstring_free(cs);
 
    cs = cstring_new("HelloWorld");
    cs2 = cstring_dup(cs);
    TEST_COND("cstring_dup()", cstring_compare_cs(cs, cs2) == 0);
    TEST_COND("cstring_length()", cstring_length(cs) == 10);
    TEST_COND("cstring_length()", cstring_length(cs2) == 10);
    cstring_tolower(cs);
    TEST_COND("cstring_tolower()", cstring_compare(cs, "helloworld") == 0);
    cstring_toupper(cs);
    TEST_COND("cstring_toupper()", cstring_compare(cs, "HELLOWORLD") == 0);
    cstring_free(cs);
    cstring_free(cs2);

    cs = cstring_new("HelloWorl");
    cs = cstring_push_ch(cs, 'd');
    TEST_COND("cstring_push_ch()", cstring_compare(cs, "HelloWorld") == 0);
    TEST_COND("cstring_pop_ch()", cstring_pop_ch(cs) == 'd');
    TEST_COND("cstring_length()", cstring_length(cs) == 9);
    TEST_COND("cstring_capacity()", cstring_capacity(cs) == 11);
    cstring_free(cs);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_cstring();
    TEST_REPORT();
    return 0;
}
