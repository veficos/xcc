

#ifndef __READER__H__
#define __READER__H__


#include "config.h"
#include "cstring.h"
#include "pmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct string_reader_s {
    cstring_t text;
    cstring_t stashed;
    void *begin;
    size_t line;
    size_t column;
    size_t seek;
    int lastch;
} *string_reader_t;


typedef struct file_reader_s {
    cstring_t filename;
    string_reader_t sreader;
} *file_reader_t;


typedef enum reader_type_e {
    READER_TYPE_FILE,
    READER_TYPE_STRING,
} reader_type_t;


typedef struct reader_s {
    reader_type_t type;
    union {
        file_reader_t fr;
        string_reader_t sr;
    } u;
} *reader_t;


#define file_reader_get(fr)         string_reader_get((fr)->sreader)
#define file_reader_peek(fr)        string_reader_peek((fr)->sreader)
#define file_reader_unget(fr, ch)   string_reader_unget((fr)->sreader, (ch))
#define file_reader_readsome(fr, n) string_reader_readsome((fr)->sreader, n)
#define file_reader_peeksome(fr, n) string_reader_peeksome((fr)->sreader, n)
#define file_reader_line(fr)        string_reader_line((fr)->sreader)
#define file_reader_column(fr)      string_reader_column((fr)->sreader)
#define file_reader_name(fr)        ((const char *)(fr)->filename)
#define file_reader_current_line(fr) string_reader_current_line((fr)->sreader)

file_reader_t file_reader_create(const char *filename);
void file_reader_destroy(file_reader_t fr);


string_reader_t string_reader_create_n(const void *data, size_t n);
void string_reader_destroy(string_reader_t sr);
int string_reader_get(string_reader_t sr);
int string_reader_peek(string_reader_t sr);
cstring_t string_reader_peeksome(string_reader_t sr, size_t n);
cstring_t string_reader_readsome(string_reader_t sr, size_t n);
bool string_reader_unget(string_reader_t sr, int ch);
cstring_t string_reader_current_line(string_reader_t sr);


static inline
string_reader_t string_reader_create(const char *s)
{
    return string_reader_create_n(s, strlen(s));
}


static inline
const char *string_reader_name(string_reader_t sr)
{
    (void) sr;
    return "<string>";
}


static inline
size_t string_reader_line(string_reader_t sr)
{
    return sr->line;
}


static inline
size_t string_reader_column(string_reader_t sr)
{
    return sr->column;
}


static inline
reader_t reader_create(reader_type_t type, const char *s)
{
    reader_t reader = (reader_t) pmalloc(sizeof(struct reader_s));
    if (!reader) {
        return NULL;
    }

    reader->type = type;

    switch (type) {
    case READER_TYPE_STRING:
        reader->u.sr = string_reader_create(s);
        if (!reader->u.sr) {
            pfree(reader);
            return NULL;
        }
        return reader;
    case READER_TYPE_FILE:
        reader->u.fr = file_reader_create(s);
        if (!reader->u.fr) {
            pfree(reader);
            return NULL;
        }
        return reader;
    }

    return NULL;
}


static inline
void reader_destroy(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        file_reader_destroy(reader->u.fr);
        break;
    case READER_TYPE_STRING:
        string_reader_destroy(reader->u.sr);
        break;
    }
    pfree(reader);
}


static inline
const char* reader_name(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_name(reader->u.fr);
    case READER_TYPE_STRING:
        return string_reader_name(reader->u.sr);
    }
    return NULL;
}


static inline
int reader_get(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_get(reader->u.fr);
    case READER_TYPE_STRING:
        return string_reader_get(reader->u.sr);
    }
    return EOF;
}


static inline
int reader_peek(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_peek(reader->u.fr);
    case READER_TYPE_STRING:
        return string_reader_peek(reader->u.sr);
    }
    return EOF;
}


static inline
cstring_t reader_peeksome(reader_t reader, size_t n)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_peeksome(reader->u.fr, n);
    case READER_TYPE_STRING:
        return string_reader_peeksome(reader->u.sr, n);
    }
    return NULL;
}


static inline
cstring_t reader_readsome(reader_t reader, size_t n)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_readsome(reader->u.fr, n);
    case READER_TYPE_STRING:
        return string_reader_readsome(reader->u.sr, n);
    }
    return NULL;
}


static inline
void reader_unget(reader_t reader, int ch)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        file_reader_unget(reader->u.fr, ch);
        break;
    case READER_TYPE_STRING:
        string_reader_unget(reader->u.sr, ch);
        break;
    }
}


static inline
size_t reader_line(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_line(reader->u.fr);
    case READER_TYPE_STRING:
        return string_reader_line(reader->u.sr);
    }
    return 0;
}


static inline
size_t reader_column(reader_t reader)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_column(reader->u.fr);
    case READER_TYPE_STRING:
        return string_reader_column(reader->u.sr);
    }
    return 0;
}


static inline
cstring_t reader_current_line(reader_t reader)
{ 
    switch (reader->type) {
    case READER_TYPE_FILE:
        file_reader_current_line(reader->u.fr);
        break;
    case READER_TYPE_STRING:
        string_reader_current_line(reader->u.sr);
        break;
    }
    return NULL;
} 


static inline
bool reader_try(reader_t reader, int ch)
{
    bool ret;
    switch (reader->type) {
    case READER_TYPE_FILE:
         ret = file_reader_peek(reader->u.fr) == ch;
         file_reader_get(reader->u.fr);
         return ret;
    case READER_TYPE_STRING:
        ret = string_reader_peek(reader->u.sr) == ch;
        string_reader_get(reader->u.sr);
        return ret;
    }
    return false;
}


static inline
bool reader_test(reader_t reader, int ch)
{
    switch (reader->type) {
    case READER_TYPE_FILE:
        return file_reader_peek(reader->u.fr) == ch;
    case READER_TYPE_STRING:
        return string_reader_peek(reader->u.sr) == ch;
    }
    return false;
}


#endif
