

#include "config.h"
#include "diag.h"
#include "cstring.h"


static 
void test_diag(void)
{
    struct source_location_s loc;

    loc.line = 8;
    loc.column = 220;
    loc.filename = cstring_create("<string>");
    loc.current_line = cstring_create("                                                                                                                                                                                                                            int val;");

    diag_errorf_with_line(&loc, "unknown identifier");

    loc.line = 8;
    loc.column = 5;
    loc.filename = cstring_create("<string>");
    loc.current_line = cstring_create("int val;");
    
    diag_errorf_with_line(&loc, "unknown identifier");
}


int main(void)
{
    test_diag();
}
