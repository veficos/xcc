

#include "config.h"
#include "reader.h"
#include "unittest.h"
#include "diag.h"
#include "option.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static 
void test_screader_case1()
{
    diag_t diag;
    reader_t reader;
    struct option_s option;

    const char *s = "Hello World\r \n\r\n";
    
    diag = diag_create();

    reader = reader_create(&option, diag);
    reader_push(reader, STREAM_TYPE_STRING, s);

    TEST_COND("reader_create()", reader != NULL);
    TEST_COND("reader_peek()", reader_peek(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_next(reader) == 'e');
    TEST_COND("reader_column()", reader_column(reader) == 3);
    TEST_COND("reader_row()", cstring_cmp(reader_row(reader), "Hello World\r ") == 0);
    TEST_COND("reader_untread()", reader_untread(reader, 'e'));
    TEST_COND("reader_untread()", reader_untread(reader, 'H'));
    TEST_COND("reader_next()", reader_next(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_next(reader) == 'e');
    TEST_COND("reader_column()", reader_column(reader) == 3);
    TEST_COND("reader_next()", reader_next(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 4);
    TEST_COND("reader_next()", reader_next(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 5);
    TEST_COND("reader_next()", reader_next(reader) == 'o');
    TEST_COND("reader_column()", reader_column(reader) == 6);
    TEST_COND("reader_next()", reader_next(reader) == ' ');
    TEST_COND("reader_column()", reader_column(reader) == 7);
    TEST_COND("reader_next()", reader_next(reader) == 'W');
    TEST_COND("reader_column()", reader_column(reader) == 8);
    TEST_COND("reader_next()", reader_next(reader) == 'o');
    TEST_COND("reader_column()", reader_column(reader) == 9);
    TEST_COND("reader_next()", reader_next(reader) == 'r');
    TEST_COND("reader_column()", reader_column(reader) == 10);
    TEST_COND("reader_next()", reader_next(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 11);
    TEST_COND("reader_next()", reader_next(reader) == 'd');
    TEST_COND("reader_column()", reader_column(reader) == 12);
    TEST_COND("reader_next()", reader_next(reader) == '\r');
    TEST_COND("reader_column()", reader_column(reader) == 13);
    TEST_COND("reader_next()", reader_next(reader) == ' ');
    TEST_COND("reader_column()", reader_column(reader) == 14);
    TEST_COND("reader_next()", reader_next(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 2);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 3);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == EOF);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    reader_destroy(reader);
    diag_destroy(diag);
}


static 
void test_screader_case2()
{
    diag_t diag;
    struct option_s option;
    reader_t reader;

    const char *s = "#in\\\n"
        "clude<stdio.h>\n"
        "int main(void) { \n"
        " printf(\"HelloWorld\"); \\ \n"
        "} \\";

    const char *d = "#include<stdio.h>\n"
        "int main(void) { \n"
        " printf(\"HelloWorld\"); } \n\xff";

    int i;

    diag = diag_create();
   
    reader = reader_create(&option, diag);
    reader_push(reader, STREAM_TYPE_STRING, s);
    for (i = 0; ; i++) {
        int ch1 = reader_peek(reader);
        int ch2 = reader_next(reader);
        TEST_COND("reader_peek()", ch1 == ch2);
        TEST_COND("reader_next()", ch2 == d[i]);
        if (ch2 == '\n') {
            printf("");
        }
        if (ch2 == EOF) {
            break;
        }
    }

    reader_destroy(reader);
    diag_destroy(diag);
}


int main(void)
{
    //test_screader_case1();
    test_screader_case2();
    TEST_REPORT();
    return 0;
}
