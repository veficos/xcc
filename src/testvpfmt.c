

#include "config.h"
#include "vpfmt.h"
#include "token.h"
#include "unittest.h"


static 
void test_vpfmt()
{
    struct source_location_s loc;

    pfmt(stderr, "%s", "HelloWorld\n");
    pfmt(stderr, "%d\n", 1024);
    pfmt(stderr, "%c\n", 'H');

    loc.line = 8;
    loc.column = 220;
    loc.fn = cstring_create("<string>");
    loc.row = cstring_create("                                                                                                                                                                                                                            int val;xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

    pfmt(stderr, "%S: error: out of memory\n", &loc);
    cstring_destroy(loc.fn);
    cstring_destroy(loc.row);

    loc.line = 8;
    loc.column = 5;
    loc.fn = cstring_create("<string>");
    loc.row = cstring_create("int val;");

    pfmt(stderr, "%S: error: out of memory\n", &loc);

    cstring_destroy(loc.fn);
    cstring_destroy(loc.row);
}


int main(void)
{
    test_vpfmt();
    return 0;
}
