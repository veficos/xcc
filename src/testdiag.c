

#include "config.h"
#include "diagnostor.h"
#include "token.h"
#include "cstring.h"


static 
void test_diag(void)
{
    struct source_location_s loc;
	diag_t diag;

	diag = diag_create();

    loc.line = 8;
    loc.column = 220;
    loc.fn = cstring_create("<string>");
    loc.row = cstring_create("                                                                                                                                                                                                                            int val;xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

    diag_errorf_with_loc(diag, &loc, "unknown identifier");
	cstring_destroy(loc.fn);
	cstring_destroy(loc.row);

    loc.line = 8;
    loc.column = 5;
    loc.fn = cstring_create("<string>");
    loc.row = cstring_create("int val;");
    
    diag_errorf_with_loc(diag, &loc, "unknown identifier");
	cstring_destroy(loc.fn);
	cstring_destroy(loc.row);

    diag_report(diag);

	diag_destroy(diag);
}


int main(void)
{
    test_diag();
    return 0;
}
