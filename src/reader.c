

#include "config.h"
#include "array.h"
#include "reader.h"
#include "pmalloc.h"
#include "cstring.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifndef MAX_STASHED_SIZE
#define MAX_STASHED_SIZE (12)
#endif


#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE (120)
#endif



static inline string_reader_t __string_reader_create_n__(cstring_t text);
static inline void __string_reader_stashed_push__(string_reader_t sr, char ch);
static inline int __string_reader_stashed_pop__(string_reader_t sr);


string_reader_t string_reader_create_n(const void *data, size_t n)
{
    cstring_t *buf;

    buf = cstring_create_n(data, n);
    if (!buf) {
        return NULL;
    }

    return __string_reader_create_n__(buf);
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

    ch = cstring_length(sr->text) > sr->seek ? sr->text[sr->seek++] : '\0';
    if (ch == '\0') {
        return EOF;
    }

    if (ch == '\n') {
        sr->line++;
        sr->column = 1;
        sr->begin = &sr->text[sr->seek];

    } else {
        sr->column++;
    }

    return ch;
}


int string_reader_peek(string_reader_t sr)
{
    int ch;

    if (sr->stashed && cstring_length(sr->stashed)) {
        return sr->stashed[cstring_length(sr->stashed) - 1];
    }

    ch = sr->text[sr->seek];
    if (ch == '\0') {
        return EOF;
    }

    return ch;
}


void string_reader_unget(string_reader_t sr, int ch)
{
    __string_reader_stashed_push__(sr, ch);
}


cstring_t string_reader_current_line(string_reader_t sr)
{
    int i = sr->seek;

    do {
        if (sr->text[i] == '\n') {
            break;
        }
    } while(i < cstring_length(sr->text) && i++);

    return cstring_create_n(sr->begin, (&sr->text[i] - (char *)sr->begin));
}


file_reader_t file_reader_create(const char *filename)
{
    file_reader_t fr;
    cstring_t buf;
    FILE *fp;
    int readn;
    struct stat st;

    fp = fopen(filename, "rb");
    if (!fp) {
        goto done;
    }

    if (fstat(fileno(fp), &st) != 0) {
        goto clean_fp;
    }

    buf = cstring_create_n(NULL, st.st_size);
    if (!buf) {
        goto clean_fp;
    }

    readn = fread(buf, sizeof(char), st.st_size, fp);
    if (readn != st.st_size) {
        goto clean_fp;
    }

    fclose(fp);

    cstring_of(buf)->length = st.st_size;

    fr = (file_reader_t) pmalloc(sizeof(struct file_reader_s));
    if (!fr) {
        goto done;
    }

    fr->filename = cstring_create(filename);
    if (!fr->filename) {
        goto clean_fr;
    }

    return fr;

clean_fr:
    pfree(fr);

clean_fp:
    fclose(fp);

done:
    return NULL;
}


static inline
string_reader_t __string_reader_create_n__(cstring_t text)
{
    string_reader_t sr;

    sr = (string_reader_t) pmalloc(sizeof(struct string_reader_s));
    if (!sr) {
        return NULL;
    }

    sr->text = text;
    sr->begin = sr->text;
    sr->stashed = NULL;
    sr->line = 1;
    sr->column = 1;
    sr->seek = 0;

    return sr;
}


static inline
void __string_reader_stashed_push__(string_reader_t sr, char ch)
{
    if (!sr->stashed) {
        sr->stashed = cstring_create_n(NULL, MAX_STASHED_SIZE);
    }

    sr->stashed = cstring_push_ch(sr->stashed, ch);
}


static inline
int __string_reader_stashed_pop__(string_reader_t sr)
{
    if (!sr->stashed) {
        return EOF;
    }

    return cstring_pop_ch(sr->stashed);
}
