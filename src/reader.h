

#ifndef __READER__H__
#define __READER__H__


#include "config.h"
#include "array.h"
#include "cstring.h"
#include "pmalloc.h"


typedef struct array_s* array_t;


typedef struct string_reader_s {
    cstring_t text;
    cstring_t stashed;
    void *begin;
    size_t line;
    size_t column;
    size_t seek;
    int lastch;
} *string_reader_t;


string_reader_t string_reader_create_n(const void *data, size_t n);
void string_reader_destroy(string_reader_t sr);
int string_reader_next(string_reader_t sr);
int string_reader_peek(string_reader_t sr);
bool string_reader_untread(string_reader_t sr, int ch);
cstring_t string_reader_row(string_reader_t sr);


static inline
string_reader_t string_reader_create(const char *s)
{
    return string_reader_create_n(s, strlen(s));
}


static inline
const char *string_reader_name(string_reader_t sr)
{
    (void)sr;
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


typedef struct file_reader_s {
    cstring_t filename;
    string_reader_t sreader;
} *file_reader_t;


#define file_reader_next(fr)            string_reader_next((fr)->sreader)
#define file_reader_peek(fr)            string_reader_peek((fr)->sreader)
#define file_reader_untread(fr, ch)     string_reader_untread((fr)->sreader, (ch))
#define file_reader_line(fr)            string_reader_line((fr)->sreader)
#define file_reader_column(fr)          string_reader_column((fr)->sreader)
#define file_reader_name(fr)            ((const char *)(fr)->filename)
#define file_reader_row(fr)             string_reader_row((fr)->sreader)


file_reader_t file_reader_create(const char *filename);
void file_reader_destroy(file_reader_t fr);



typedef enum stream_type_e {
    STREAM_TYPE_FILE,
    STREAM_TYPE_STRING,
} stream_type_t;


typedef struct reader_s {
    stream_type_t type;
    union {
        file_reader_t fr;
        string_reader_t sr;
    } u;
} *reader_t;


static inline
reader_t reader_init(reader_t reader, stream_type_t type, const char *s)
{
    assert(reader != NULL);

    reader->type = type;
    switch (type) {
    case STREAM_TYPE_STRING:
        reader->u.sr = string_reader_create(s);
        if (!reader->u.sr) {
            pfree(reader);
            return NULL;
        }
        return reader;
    case STREAM_TYPE_FILE:
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
void reader_uninit(reader_t reader)
{
    assert(reader != NULL);

    switch (reader->type) {
    case STREAM_TYPE_FILE:
        file_reader_destroy(reader->u.fr);
        break;
    case STREAM_TYPE_STRING:
        string_reader_destroy(reader->u.sr);
        break;
    }
}


static inline
reader_t reader_create(stream_type_t type, const char *s)
{
    reader_t reader;;
    if ((reader = (reader_t)pmalloc(sizeof(struct reader_s))) == NULL) {
        return NULL;
    }
    return reader_init(reader, type, s);
}


static inline
void reader_destroy(reader_t reader)
{
    assert(reader != NULL);

    reader_uninit(reader);

    pfree(reader);
}


static inline
const char* reader_name(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_name(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_name(reader->u.sr);
    }
    return NULL;
}


static inline
int reader_next(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_next(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_next(reader->u.sr);
    }
    return EOF;
}


static inline
int reader_peek(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_peek(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_peek(reader->u.sr);
    }
    return EOF;
}


static inline
bool reader_is_empty(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_peek(reader->u.fr) == EOF;
    case STREAM_TYPE_STRING:
        return string_reader_peek(reader->u.sr) == EOF;
    }
    return true;
}


static inline
bool reader_untread(reader_t reader, int ch)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_untread(reader->u.fr, ch);
    case STREAM_TYPE_STRING:
        return string_reader_untread(reader->u.sr, ch);
    }
    return false;
}


static inline
size_t reader_line(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_line(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_line(reader->u.sr);
    }
    return 0;
}


static inline
size_t reader_column(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_column(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_column(reader->u.sr);
    }
    return 0;
}


static inline
cstring_t reader_row(reader_t reader)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_row(reader->u.fr);
    case STREAM_TYPE_STRING:
        return string_reader_row(reader->u.sr);
    }
    return NULL;
}


static inline
bool reader_try(reader_t reader, int ch)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        if (file_reader_peek(reader->u.fr) == ch) {
            file_reader_next(reader->u.fr);
            return true;
        }
        break;
    case STREAM_TYPE_STRING:
        if (string_reader_peek(reader->u.sr) == ch) {
            string_reader_next(reader->u.sr);
            return true;
        }
        break;
    }
    return false;
}


static inline
bool reader_test(reader_t reader, int ch)
{
    switch (reader->type) {
    case STREAM_TYPE_FILE:
        return file_reader_peek(reader->u.fr) == ch;
    case STREAM_TYPE_STRING:
        return string_reader_peek(reader->u.sr) == ch;
    }
    return false;
}


/* source code reader */
typedef struct screader_s {
    array_t files;
    reader_t last;
} *screader_t;


screader_t screader_create(stream_type_t type, const char *s);
void screader_destroy(screader_t screader);
reader_t screader_push(screader_t screader, stream_type_t type, const char *s);
void screader_pop(screader_t screader);
int screader_next(screader_t screader);
int screader_peek(screader_t screader);


static inline
bool screader_untread(screader_t screader, int ch)
{
    return reader_untread(screader->last, ch);
}


static inline
int screader_depth(screader_t sreader)
{
    return array_size(sreader->files);
}


static inline
bool screader_try(screader_t screader, int ch)
{
    return reader_try(screader->last, ch);
}


static inline
bool screader_test(screader_t screader, int ch)
{
    return reader_test(screader->last, ch);
}


static inline
bool screader_is_empty(screader_t screader)
{
    return screader_peek(screader) == EOF;
}


static inline
const char *screader_name(screader_t screader)
{
    return reader_name(screader->last);
}


static inline
size_t screader_line(screader_t screader)
{
    return reader_line(screader->last);
}


static inline
size_t screader_column(screader_t screader)
{
    return reader_column(screader->last);
}


static inline
cstring_t screader_row(screader_t screader)
{
    return reader_row(screader->last);
}


#endif
