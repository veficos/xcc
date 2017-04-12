

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
void test_reader_case1()
{
    diag_t diag;
    reader_t reader;
    struct option_s option;

    const char *s = "Hello World\r"
                    " \n"
                    "\r\n";

    diag = diag_create();

    reader = reader_create(diag, &option);
    reader_push(reader, STREAM_TYPE_STRING, s);

    TEST_COND("reader_create()", reader != NULL);

    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_next(reader) == 'e');
    TEST_COND("reader_column()", reader_column(reader) == 3);
    TEST_COND("reader_next()", reader_next(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 4);
    TEST_COND("reader_next()", reader_next(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 5);
    TEST_COND("reader_row()", cstring_cmp(row2line(reader_row(reader)), "Hello World") == 0);
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
    TEST_COND("reader_next()", reader_next(reader) == '\n');
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == ' ');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_next(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 3);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 4);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_next(reader) == EOF);
    TEST_COND("reader_column()", reader_column(reader) == 1);

    reader_destroy(reader);
    diag_destroy(diag);
}


static 
void test_reader_case2()
{
    diag_t diag;
    struct option_s option;
    reader_t reader;

    const char *s = "#in\\\r"
        "clude<stdio.h>\r"
        "int main(void) \\ { \n"
        " printf(\"HelloWorld\"); \r\n"
        " \\ \n"    
        "\\  \r "                           
        "\n"
        "} \\   ";                          

    const char *t = "#include<stdio.h>\n"
        "int main(void) \\ { \n"
        " printf(\"HelloWorld\"); \n"
        "  \n"
        "} \n\xff";

    int i;

    diag = diag_create();
   
    reader = reader_create(diag, &option);
    reader_push(reader, STREAM_TYPE_STRING, s);

    for (i = 0; ; i++) {
        int ch1 = reader_peek(reader);
        int ch2 = reader_next(reader);
        TEST_COND("reader_peek()", ch1 == ch2);
        TEST_COND("reader_next()", ch2 == t[i]);
        if (ch2 == EOF) {
            break;
        }
        if (ch1 != ch2) {
            printf("");
        }
    }

    reader_destroy(reader);
    diag_destroy(diag);
}


static
void test_reader_case3()
{
    diag_t diag;
    struct option_s option;
    reader_t reader;
    cstring_t cs;
    const char *s =" printf(\"HelloWorld\"); \\ f";

    diag = diag_create();
    cs = cstring_create_n(NULL, 24);
    reader = reader_create(diag, &option);
    reader_push(reader, STREAM_TYPE_STRING, s);

    for (;;) {
        int ch = reader_next(reader);
        if (ch == EOF) {
            break;
        }

        cstring_push_ch(cs, ch);
    }

    printf("%s", cs);

    reader_destroy(reader);
    diag_destroy(diag);
    cstring_destroy(cs);
}


int main(void)
{
    test_reader_case1();
    test_reader_case2();
    test_reader_case3();
    TEST_REPORT();
    return 0;
}
