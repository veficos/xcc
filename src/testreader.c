

#include "config.h"
#include "reader.h"
#include "unittest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static void test_reader_case1()
{
    reader_t *reader;
    cstring_t cs;

    const char *s = "Hello World\r"
                    " \n"
                    "\r\n";

    reader = reader_create();
    reader_push(reader, STREAM_TYPE_STRING, s);

    TEST_COND("reader_create()", reader != NULL);
    TEST_COND("reader_depth()", reader_depth(reader) == 1);

    cs = linenote2cs(reader_linenote(reader));

    TEST_COND("line_note", cstring_compare(cs, "Hello World") == 0);
    TEST_COND("reader_modify_time()", reader_modify_time(reader) == 0);
    TEST_COND("reader_change_time()", reader_change_time(reader) == 0);
    TEST_COND("reader_access_time()", reader_access_time(reader) == 0);

    TEST_COND("reader_line()", reader_line(reader) == 1);
    TEST_COND("reader_column()", reader_column(reader) == 1);
    TEST_COND("reader_next()", reader_get(reader) == 'H');
    TEST_COND("reader_column()", reader_column(reader) == 2);
    TEST_COND("reader_next()", reader_get(reader) == 'e');
    TEST_COND("reader_column()", reader_column(reader) == 3);
    TEST_COND("reader_next()", reader_get(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 4);
    TEST_COND("reader_next()", reader_get(reader) == 'l');
    TEST_COND("reader_column()", reader_column(reader) == 5);
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
    TEST_COND("reader_line()", reader_line(reader) == 2);
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

    cstring_free(cs);
    reader_destroy(reader);
}


static void test_reader_case2()
{
    reader_t *reader;
    int i;

    const char *sourcecode = "#in\\\r"
        "clude<stdio.h>\r"
        "int main(void) \\ { \n"
        " printf(\"HelloWorld\"); \\\r\n"
        "\n"
        " \\ \n"                             /* warning */
        "\\  \r "                            /* warning */
        "\n"
        "} 中文\\   ";                        /* warning */

    const char *regularization = "#include<stdio.h>\n"
        "int main(void) \\ { \n"
        " printf(\"HelloWorld\"); \n"
        "  \n"
        "} 中文\n\xff";

    reader = reader_create();
    reader_push(reader, STREAM_TYPE_STRING, sourcecode);

    for (i = 0; i < strlen(regularization);) {
        if (reader_is_empty(reader)) {
            break;
        }

        int ch1 = reader_peek(reader);
        int ch2 = reader_get(reader);
        if (ch1 == EOF && ch2 == EOF) {
            reader_pop(reader);
            continue;
        }

        TEST_COND("reader_peek()", ch1 == ch2);
        TEST_COND("reader_next()", (char)ch2 == regularization[i]);
        i++;
    }

    reader_destroy(reader);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_reader_case1();
    test_reader_case2();
    TEST_REPORT();
    return 0;
}
