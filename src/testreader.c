

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
    size_t i;
    int ch;
    cstring_t cs;
    file_reader_t fr;

    {
        write2file("emp1.txt", s);

        fr = file_reader_create("emp1.txt");
        assert(fr != NULL);

        cs = file_reader_peeksome(fr, 2);
        TEST_COND("file_reader_peeksome", cstring_cmp(cs, "Do") == 0);
        cstring_destroy(cs);
        
        file_reader_unget(fr, 'a');

        TEST_COND("file_reader_column()", file_reader_column(fr) == 1);
        TEST_COND("file_reader_peek()", file_reader_peek(fr) == 'a');
        TEST_COND("file_reader_get()", file_reader_get(fr) == 'a');
        TEST_COND("file_reader_column()", file_reader_column(fr) == 1);


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
    cstring_t cs;

    {
        sr = string_reader_create(s);
        assert(sr != NULL);
        TEST_COND("string_reader_line()", string_reader_line(sr));
        
        cs = string_reader_peeksome(sr, 2);
        TEST_COND("string_reader_peeksome", cstring_cmp(cs, "Do") == 0);
        cstring_destroy(cs);
        
        string_reader_unget(sr, 'a');

        TEST_COND("string_reader_column()", string_reader_column(sr) == 1);
        TEST_COND("string_reader_peek()", string_reader_peek(sr) == 'a');
        TEST_COND("string_reader_get()", string_reader_get(sr) == 'a');
        TEST_COND("string_reader_column()", string_reader_column(sr) == 1);

        for (i = 0;; i++) {
            if (i < slen) {
                TEST_COND("string_reader_column()", string_reader_column(sr) == i+1);
                TEST_COND("string_reader_peek()", string_reader_peek(sr) == s[i]);
                TEST_COND("string_reader_column()", string_reader_column(sr) == i+1);
            }

            ch = string_reader_get(sr);
            if (ch == EOF) {
                break;
            }

            TEST_COND("string_reader_column()", string_reader_column(sr) == i+2);
            TEST_COND("string_reader_get()", ch == s[i]);

            string_reader_unget(sr, ch);

            TEST_COND("string_reader_column()", string_reader_column(sr) == i+2);

            TEST_COND("string_reader_unget()", string_reader_peek(sr) == s[i]);

            string_reader_get(sr);

            TEST_COND("string_reader_column()", string_reader_column(sr) == i+2);
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
