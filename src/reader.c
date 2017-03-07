

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
static inline bool __string_reader_stashed_push__(string_reader_t sr, char ch);
static inline int __string_reader_stashed_pop__(string_reader_t sr);


string_reader_t string_reader_create_n(const void *data, size_t n)
{
    cstring_t buf;

    buf = cstring_create_n(data, n);
    if (!buf) {
        return NULL;
    }

    return __string_reader_create_n__(buf);
}


void string_reader_destroy(string_reader_t sr)
{
    if (sr) {
        cstring_destroy(sr->text);

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


bool string_reader_unget(string_reader_t sr, int ch)
{
    return __string_reader_stashed_push__(sr, ch);
}


cstring_t string_reader_peeksome(string_reader_t sr, size_t n)
{
    cstring_t cs = cstring_create_n(NULL, n);
    if (!cs) {
        return NULL;
    }

    if (n > (cstring_length(sr->text) - sr->seek)) {
        n = (cstring_length(sr->text) - sr->seek);
    }

    cs = cstring_cpy_n(cs, &sr->text[sr->seek], n);

    return cs;
}


cstring_t string_reader_readsome(string_reader_t sr, size_t n)
{
    cstring_t cs = cstring_create_n(NULL, n);
    if (!cs) {
        return NULL;
    }

    if (n > (cstring_length(sr->text) - sr->seek)) {
        n = (cstring_length(sr->text) - sr->seek);
    }

    cs = cstring_cpy_n(cs, &sr->text[sr->seek], n);

    sr->seek += n;

    return cs;
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
        goto clean_cs;
    }

    cstring_of(buf)->length = st.st_size;

    fr = (file_reader_t) pmalloc(sizeof(struct file_reader_s));
    if (!fr) {
        goto clean_cs;
    }

    fr->filename = cstring_create(filename);
    if (!fr->filename) {
        goto clean_fr;
    }

    fr->sreader = __string_reader_create_n__(buf);
    if (!fr->sreader) {
        goto clean_fr;
    }
    
    fclose(fp);
    return fr;

clean_fr:
    pfree(fr);

clean_cs:
    cstring_destroy(buf);

clean_fp:
    fclose(fp);

done:
    return NULL;
}


void file_reader_destroy(file_reader_t fr)
{
    if (fr) {
        cstring_destroy(fr->filename);

        string_reader_destroy(fr->sreader);

        pfree(fr);
    }
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
bool __string_reader_stashed_push__(string_reader_t sr, char ch)
{
    if (!sr->stashed) {
        sr->stashed = cstring_create_n(NULL, MAX_STASHED_SIZE);
        if (!sr->stashed) {
            return false;
        }
    }

    if (!(sr->stashed = cstring_push_ch(sr->stashed, ch))){
        return false;
    }

    return true;
}


static inline
int __string_reader_stashed_pop__(string_reader_t sr)
{
    if (!sr->stashed) {
        return EOF;
    }

    return cstring_pop_ch(sr->stashed);
}
