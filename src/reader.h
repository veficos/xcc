

#ifndef __READER__H__
#define __READER__H__


#include "config.h"
#include "cstring.h"


typedef struct array_s      array_t;
typedef struct stream_s     stream_t;
typedef struct cspool_s     cspool_t;


typedef enum stream_type_e {
    STREAM_TYPE_FILE,
    STREAM_TYPE_STRING,
} stream_type_t;


typedef const void* linenote_t;


typedef struct reader_s {
    array_t *streams;
    stream_t *last;
    cspool_t *pool;
    int lastch;
} reader_t;


reader_t* reader_create(void);
void reader_destroy(reader_t *reader);
size_t reader_level(reader_t *reader);
bool reader_push(reader_t *reader, stream_type_t type, const unsigned char *s);
time_t reader_mt(reader_t *reader);
int reader_get(reader_t *reader);
int reader_peek(reader_t *reader);
void reader_unget(reader_t *reader, int ch);
bool reader_try(reader_t *reader, int ch);
bool reader_test(reader_t *reader, int ch);
linenote_t reader_linenote(reader_t *reader);
size_t reader_line(reader_t *reader);
size_t reader_column(reader_t *reader);
bool reader_is_empty(reader_t *reader);
bool reader_is_eos(reader_t *reader);
cstring_t reader_name(reader_t *reader);

cstring_t linenote2cs(linenote_t linenote);


#endif
