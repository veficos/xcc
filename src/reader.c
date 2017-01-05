

#include "config.h"
#include "array.h"
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


typedef struct string_reader_s {
    cstring_t text;
    cstring_t stashed;
    long line;
    long column;
    size_t seek;
}* string_reader_t;


static inline void __file_reader_stashed_push__(file_reader_t fr, char ch);
static inline int __file_reader_stashed_pop__(file_reader_t fr);
static inline void __string_reader_stashed_push__(string_reader_t sr, char ch);
static inline int __string_reader_stashed_pop__(string_reader_t sr);


file_reader_t file_reader_create(const char *filename)
{
    file_reader_t fr;
    FILE *file;

    file = fopen(filename, "rb");
    if (!file) {
        goto failed;
    }

    fr = (file_reader_t) pmalloc(sizeof(struct file_reader_s));
    if (!fr) {
        goto clean_fp;
    }

    fr->filename = cstring_create(filename);
    if (!fr->filename) {
        goto clean_fr;
    }

    fr->stashed = NULL;
    fr->file = file;
    fr->line = 1;
    fr->column = 1;
    
    return fr;

clean_fr:
    pfree(fr);

clean_fp:
    fclose(file);

failed:
    return NULL;
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


string_reader_t string_reader_create_n(const void *data, size_t n)
{
    string_reader_t sr;

    sr = (string_reader_t) pmalloc(sizeof(struct string_reader_s));
    if (!sr) {
        goto failed;
    }

    sr->text = cstring_create_n(data, n);
    if (!sr->text) {
        goto clean_tr;
    }

    sr->stashed = NULL;
    sr->line = 1;
    sr->column = 1;
    sr->seek = 0;

    return sr;

    clean_tr:
    pfree(sr);

    failed:
    return NULL;
}


void string_reader_destroy(string_reader_t sr)
{
    if (sr) {
        if (sr->text) {
            cstring_destroy(sr->text);
        }

        if (sr->stashed) {
            cstring_destroy(sr->stashed);
        }

        pfree(sr);
    }
}


int string_reader_get(string_reader_t sr)
{
    int ch;

    ch = __string_reader_stashed_pop__(sr);
    if (ch != EOF) {
        return ch;
    }

    ch = sr->text[sr->seek++];
    if (ch == '\0') {
        return EOF;
    }

    if (ch == '\n') {
        sr->line++;
        sr->column = 1;

    } else {
        sr->column++;
    }

    return ch;
}


int string_reader_peek(string_reader_t sr)
{
    int ch;

    ch = string_reader_get(sr);
    if (ch != EOF) {
        __string_reader_stashed_push__(sr, ch);
    }

    return ch;
}


void string_reader_unget(string_reader_t sr, int ch)
{
    __string_reader_stashed_push__(sr, ch);
}


const char *string_reader_name(string_reader_t sr)
{
    return "<string>";
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


static inline
void __string_reader_stashed_push__(string_reader_t sr, char ch)
{
    if (!sr->stashed) {
        sr->stashed = cstring_create_n(NULL, MAX_STASHED_SIZE);
    }

    sr->stashed = cstring_push_ch(sr->stashed, ch);
}


static inline int __string_reader_stashed_pop__(string_reader_t sr)
{
    if (!sr->stashed) {
        return EOF;
    }

    return cstring_pop_ch(sr->stashed);
}
