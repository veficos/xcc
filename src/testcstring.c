

#include "cstring.h"
#include "unittest.h"


static 
void test_cstring(void)
{
    cstring_t cs;
    
    cs = cstring_create("");
    
    TEST_COND("cstring_length()", cstring_length(cs) == 0);

    TEST_REPORT();
}


int main(void)
{
    test_cstring();
    return 0;
}