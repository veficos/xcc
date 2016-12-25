

#include "array.h"
#include "reader.h"
#include "pmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct file_reader_s {
    const char *filename;
    FILE *file;
    int line;
    int column;
    array_t *stashed;
};

typedef struct file_reader_s file_reader_t;


file_reader_t *file_reader_create(const char *filename)
{
    file_reader_t *file_reader;
    FILE *file;

    file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    file_reader = (file_reader_t *) pmalloc(sizeof(file_reader_t));
    if (!file_reader) {
        fclose(file);
        return NULL;
    }
}
