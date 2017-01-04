

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
void test_reader()
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
        
        for (i = 0; ; i++) {
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

        file_reader_destroy(fr);
    }

    TEST_REPORT();
}


int main(void)
{
    test_reader();
    return 0;
}
