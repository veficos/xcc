

#include "config.h"
#include "diag.h"
#include "cstring.h"


static 
void test_diag(void)
{
    struct source_location_s loc;
	diag_t diag;

	diag = diag_create();

    loc.line = 8;
    loc.column = 220;
    loc.filename = cstring_create("<string>");
    loc.current_line = cstring_create("                                                                                                                                                                                                                            int val;");

    diag_errorf_with_location(diag, &loc, "unknown identifier");

	cstring_destroy(loc.filename);
	cstring_destroy(loc.current_line);

    loc.line = 8;
    loc.column = 5;
    loc.filename = cstring_create("<string>");
    loc.current_line = cstring_create("int val;");
    
    diag_errorf_with_location(diag, &loc, "unknown identifier");
	cstring_destroy(loc.filename);
	cstring_destroy(loc.current_line);

	diag_destroy(diag);
}



int main(void)
{
    test_diag();
}
