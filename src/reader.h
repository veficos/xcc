

#ifndef __READER__H__
#define __READER__H__


#include "config.h"
#include "array.h"
#include "cstring.h"
#include "pmalloc.h"


typedef struct array_s*     array_t;
typedef struct option_s*    option_t;
typedef struct diag_s*      diag_t;


typedef struct sstream_s {
    cstring_t text;
    cstring_t stashed;
    void *begin;
    size_t seek;
    int lastch;
} *sstream_t;


typedef struct fstream_s {
    cstring_t fn;
    sstream_t ss;
} *fstream_t;


typedef enum stream_type_e {
    STREAM_TYPE_FILE,
    STREAM_TYPE_STRING,
} stream_type_t;


typedef struct stream_s {
    union {
        sstream_t ss;
        fstream_t fs;
    };
    stream_type_t type;
    size_t line;
    size_t column;
} *stream_t;


typedef struct reader_s {
    array_t streams;
    stream_t last;
    option_t option;
    diag_t diag;
} *reader_t;


#define sstream_last(ss)            ((ss)->lastch)

sstream_t sstream_create(const char *s);
sstream_t sstream_create_n(const void *data, size_t n);
void sstream_destroy(sstream_t ss);
int sstream_next(sstream_t ss);
int sstream_peek(sstream_t sstream);
bool sstream_untread(sstream_t ss, int ch);
cstring_t sstream_row(sstream_t ss);
const char *sstream_name(sstream_t ss);


#define fstream_next(fs)            sstream_next((fs)->ss)
#define fstream_peek(fs)            sstream_peek((fs)->ss)
#define fstream_last(fs)            sstream_last((fs)->ss)
#define fstream_untread(fs, ch)     sstream_untread((fs)->ss, (ch))
#define fstream_name(fs)            ((const char *)(fs)->fn)
#define fstream_row(fs)             sstream_row((fs)->ss)

fstream_t fstream_create(const char *fn);
void fstream_destroy(fstream_t fs);


#define stream_line(stream)         ((stream)->line)
#define stream_column(stream)       ((stream)->column)

bool stream_init(stream_t stream, stream_type_t type, const char *s);
void stream_uninit(stream_t stream);
int stream_next(stream_t stream);
int stream_peek(stream_t stream);
bool stream_untread(stream_t stream, int ch);
cstring_t stream_row(stream_t stream);
const char *stream_name(stream_t stream);
bool stream_try(stream_t stream, int ch);
bool stream_test(stream_t stream, int ch);


/* reader interface */

#define reader_line(reader)         stream_line((reader)->last)
#define reader_column(reader)       stream_column((reader)->last)
#define reader_row(reader)          stream_row((reader)->last)
#define reader_name(reader)         stream_name((reader)->last)
#define reader_is_empty(reader)     (reader_peek(reader) == EOF)

reader_t reader_create(option_t op, diag_t diag);
stream_t reader_push(reader_t reader, stream_type_t type, const char *s);
void reader_destroy(reader_t reader);
int reader_next(reader_t reader);
int reader_peek(reader_t reader);
bool reader_untread(reader_t reader, int ch);
bool reader_try(reader_t reader, int ch);
bool reader_test(reader_t reader, int ch);


#endif
