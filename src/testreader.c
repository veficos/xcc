

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


int main(void)
{
    TEST_REPORT();
    return 0;
}
