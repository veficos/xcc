

#include "reader.h"
#include "unittest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static 
void write2file(const char *filename, const char *s)
{
    FILE *fp = fopen(filename, "w");

    fwrite(s, strlen(s), sizeof(char), fp);

    fclose(fp);
}


static
void removefile(const char *filename)
{
    remove(filename);
}


static 
void test_file_reader()
{
    const char *s = "Do one thing at a time, and do well.";
    size_t slen = strlen(s);
    int i;
    int ch;
    file_reader_t fr;

    {
        write2file("emp1.txt", s);

        fr = file_reader_create("emp1.txt");
        assert(fr != NULL);

        for (i = 0;; i++) {
            if (i < slen) {
                TEST_COND("file_reader_peek()", file_reader_peek(fr) == s[i]);
            }

            ch = file_reader_get(fr);
            if (ch == EOF) {
                break;
            }

            TEST_COND("file_reader_get()", ch == s[i]);

            file_reader_unget(fr, ch);

            TEST_COND("file_reader_unget()", file_reader_peek(fr) == s[i]);

            file_reader_get(fr);
        }

        TEST_COND("file_reader_name()", memcmp("emp1.txt", file_reader_name(fr), 8) == 0);

        file_reader_destroy(fr);

        removefile("emp1.txt");
    }
}


static
void test_string_reader()
{
    const char *s = "Do one thing at a time, and do well.";
    size_t slen = strlen(s);
    int i;
    int ch;
    string_reader_t sr;

    {
        sr = string_reader_create(s);
        assert(sr != NULL);

        for (i = 0;; i++) {
            if (i < slen) {
                TEST_COND("string_reader_peek()", string_reader_peek(sr) == s[i]);
            }

            ch = string_reader_get(sr);
            if (ch == EOF) {
                break;
            }

            TEST_COND("string_reader_get()", ch == s[i]);

            string_reader_unget(sr, ch);

            TEST_COND("string_reader_unget()", string_reader_peek(sr) == s[i]);

            string_reader_get(sr);
        }

        TEST_COND("string_reader_name()", memcmp("<string>", string_reader_name(sr), 8) == 0);

        string_reader_destroy(sr);
    }
}


int main(void)
{
    test_file_reader();
    test_string_reader();
    TEST_REPORT();
    return 0;
}
