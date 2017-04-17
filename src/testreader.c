

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
    cstring_t cs;
    struct option_s option;

    const char *s = "Hello World\r"
                    " \n"
                    "\r\n";

    diag = diag_create();

    reader = reader_create(diag, &option);
    reader_push(reader, STREAM_TYPE_STRING, s);

    TEST_COND("reader_create()", reader != NULL);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_get(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_get(reader) == 'e');
    TEST_COND("reader_column()", reader_column(reader) == 3);
    TEST_COND("reader_next()", reader_get(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 4);
    TEST_COND("reader_next()", reader_get(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 5);
    cs = linenode2cs(reader_linenote(reader));
    TEST_COND("line_note", cstring_cmp(cs, "Hello World") == 0);
    TEST_COND("reader_next()", reader_get(reader) == 'o');
    TEST_COND("reader_column()", reader_column(reader) == 6);
    TEST_COND("reader_next()", reader_get(reader) == ' ');
    TEST_COND("reader_column()", reader_column(reader) == 7);
    TEST_COND("reader_next()", reader_get(reader) == 'W');
    TEST_COND("reader_column()", reader_column(reader) == 8);
    TEST_COND("reader_next()", reader_get(reader) == 'o');
    TEST_COND("reader_column()", reader_column(reader) == 9);
    TEST_COND("reader_next()", reader_get(reader) == 'r');
    TEST_COND("reader_column()", reader_column(reader) == 10);
    TEST_COND("reader_next()", reader_get(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 11);
    TEST_COND("reader_next()", reader_get(reader) == 'd');
    TEST_COND("reader_column()", reader_column(reader) == 12);
    TEST_COND("reader_next()", reader_get(reader) == '\n');
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_get(reader) == ' ');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_get(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 3);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_get(reader) == '\n');
    TEST_COND("reader_line()", reader_line(reader) == 4);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_get(reader) == EOF);
    TEST_COND("reader_column()", reader_column(reader) == 1);

    cstring_destroy(cs);
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
        " printf(\"HelloWorld\"); \\\r\n"
        "\n"
        " \\ \n"    
        "\\  \r "                           
        "\n"
        "} Œ“\\   ";                          

    const char *t = "#include<stdio.h>\n"
        "int main(void) \\ { \n"
        " printf(\"HelloWorld\"); \n"
        "  \n"
        "} Œ“\n\xff";

    int i;

    diag = diag_create();
   
    reader = reader_create(diag, &option);
    reader_push(reader, STREAM_TYPE_STRING, s);

    for (i = 0; ; i++) {
        int ch1 = reader_peek(reader);
        int ch2 = reader_get(reader);
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
        int ch = reader_get(reader);
        if (ch == EOF) {
            break;
        }
        cs = cstring_push_ch(cs, ch);
    }

    printf("%s", cs);

    reader_destroy(reader);
    diag_destroy(diag);
    cstring_destroy(cs);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_reader_case1();
    test_reader_case2();
    test_reader_case3();
    TEST_REPORT();
    return 0;
}
