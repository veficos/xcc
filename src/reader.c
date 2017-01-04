

#include "array.h"
#include "reader.h"
#include "pmalloc.h"
#include "cstring.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#ifndef MAX_STASHED_SIZE
#define MAX_STASHED_SIZE (12)
#endif


typedef struct file_reader_s {
    cstring_t filename;
    cstring_t stashed;
    FILE *file;
    long line;
    long column;
}* file_reader_t;


static inline void __file_reader_stashed_push__(file_reader_t fr, char ch);
static inline int __file_reader_stashed_pop__(file_reader_t fr);


file_reader_t file_reader_create(const char *filename)
{
    file_reader_t fr = NULL;
    FILE *file;

    file = fopen(filename, "rb");
    if (!file) {
        goto done;
    }

    fr = (file_reader_t *) pmalloc(sizeof(file_reader_t));
    if (!fr) {
        goto close_fp;
    }

    fr->filename = cstring_create(filename);
    if (!fr->filename) {
        goto close_fr;
    }

    fr->stashed = NULL;
    fr->file = file;
    fr->line = 1;
    fr->column = 1;

close_fr:
    pfree(fr);

close_fp:
    fclose(file);

done:
    return file_reader;
}


void file_reader_destroy(file_reader_t fr)
{
    if (fr) {
        if (fr->filename) {
            cstring_destroy(fr->filename);
        }

        if (fr->stashed) {
            cstring_destroy(fr->stashed);
        }

        fclose(fr->file);
        pfree(fr);
    }
}


int file_reader_get(file_reader_t fr)
{
    int ch;
    
    ch = __file_reader_stashed_pop__(fr);
    if (ch != EOF) {
        return ch;
    }

    ch = fgetc(fr->file);
    if (ch == EOF) {
        return EOF;
    }

    if (ch == '\n') {
        fr->line++;
        fr->column = 1;

    } else {
        fr->column++;
    }

    return ch;
}


int file_reader_peek(file_reader_t fr)
{
    int ch;
    
    ch = file_reader_get(fr);
    if (ch != EOF) {
        __file_reader_stashed_push__(fr, ch);
    }

    return ch;
}


void file_reader_unget(file_reader_t fr, char ch)
{
    __file_reader_stashed_push__(fr, ch);
}


const char *file_reader_name(file_reader_t fr)
{
    return fr->filename;
}


static inline
void __file_reader_stashed_push__(file_reader_t fr, char ch)
{
    if (!fr->stashed) {
        fr->stashed = cstring_create_n(NULL, MAX_STASHED_SIZE);
    }

    fr->stashed = cstring_push_ch(fr->stashed, ch);
}


static inline
int __file_reader_stashed_pop__(file_reader_t fr)
{
    if (!fr->stashed) {
        return EOF;
    }

    return cstring_pop_ch(fr->stashed);
}

