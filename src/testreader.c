

#include "reader.h"
#include "unittest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static 
void test_screader()
{
    screader_t screader;
    const char *s = "Hello World\r \n\r\n";

    screader = screader_create(STREAM_TYPE_STRING, s);
    TEST_COND("screader_create()", screader != NULL);
    TEST_COND("screader_peek()", screader_peek(screader) == 'H');
    TEST_COND("screader_column()", screader_column(screader) == 1);
    TEST_COND("screader_next()", screader_next(screader) == 'H');
    TEST_COND("screader_column()", screader_column(screader) == 2);
    TEST_COND("screader_next()", screader_next(screader) == 'e');
    TEST_COND("screader_column()", screader_column(screader) == 3);
    TEST_COND("screader_row()", cstring_cmp(screader_row(screader), "Hello World\r ") == 0);
    TEST_COND("screader_untread()", screader_untread(screader, 'e'));
    TEST_COND("screader_untread()", screader_untread(screader, 'H'));
    TEST_COND("screader_next()", screader_next(screader) == 'H');
    TEST_COND("screader_column()", screader_column(screader) == 2);
    TEST_COND("screader_next()", screader_next(screader) == 'e');
    TEST_COND("screader_column()", screader_column(screader) == 3);
    TEST_COND("screader_next()", screader_next(screader) == 'l');
    TEST_COND("screader_column()", screader_column(screader) == 4);
    TEST_COND("screader_next()", screader_next(screader) == 'l');
    TEST_COND("screader_column()", screader_column(screader) == 5);
    TEST_COND("screader_next()", screader_next(screader) == 'o');
    TEST_COND("screader_column()", screader_column(screader) == 6);
    TEST_COND("screader_next()", screader_next(screader) == ' ');
    TEST_COND("screader_column()", screader_column(screader) == 7);
    TEST_COND("screader_next()", screader_next(screader) == 'W');
    TEST_COND("screader_column()", screader_column(screader) == 8);
    TEST_COND("screader_next()", screader_next(screader) == 'o');
    TEST_COND("screader_column()", screader_column(screader) == 9);
    TEST_COND("screader_next()", screader_next(screader) == 'r');
    TEST_COND("screader_column()", screader_column(screader) == 10);
    TEST_COND("screader_next()", screader_next(screader) == 'l');
    TEST_COND("screader_column()", screader_column(screader) == 11);
    TEST_COND("screader_next()", screader_next(screader) == 'd');
    TEST_COND("screader_column()", screader_column(screader) == 12);
    TEST_COND("screader_next()", screader_next(screader) == '\r');
    TEST_COND("screader_column()", screader_column(screader) == 13);
    TEST_COND("screader_next()", screader_next(screader) == ' ');
    TEST_COND("screader_column()", screader_column(screader) == 14);
    TEST_COND("screader_next()", screader_next(screader) == '\n');
    TEST_COND("screader_line()", screader_line(screader) == 2);
    TEST_COND("screader_column()", screader_column(screader) == 1);
    TEST_COND("screader_next()", screader_next(screader) == '\n');
    TEST_COND("screader_line()", screader_line(screader) == 3);
    TEST_COND("screader_column()", screader_column(screader) == 1);
    TEST_COND("screader_next()", screader_next(screader) == EOF);
    TEST_COND("screader_column()", screader_column(screader) == 1);
    screader_destroy(screader);
}


int main(void)
{
    test_screader();
    TEST_REPORT();
    return 0;
}
