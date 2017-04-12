

#ifndef __READER__H__
#define __READER__H__


#include "config.h"
#include "cstring.h"

typedef struct array_s*     array_t;
typedef struct option_s*    option_t;
typedef struct diag_s*      diag_t;
typedef struct stream_s*    stream_t;


typedef enum stream_type_e {
    STREAM_TYPE_FILE,
    STREAM_TYPE_STRING,
} stream_type_t;


typedef struct reader_s {
    diag_t diag;
    option_t option;

    array_t streams;
    stream_t last;
} *reader_t;


reader_t reader_create(diag_t diag, option_t op);
void reader_destroy(reader_t reader);
bool reader_push(reader_t reader, stream_type_t type, const char *s);
int reader_next(reader_t reader);
int reader_peek(reader_t reader);
bool reader_untread(reader_t reader, int ch);
bool reader_try(reader_t reader, int ch);
bool reader_test(reader_t reader, int ch);

const char *reader_row(reader_t reader);
size_t reader_line(reader_t reader);
size_t reader_column(reader_t reader);
bool reader_is_empty(reader_t reader);
cstring_t reader_name(reader_t reader);

cstring_t row2line(const char *row);


#endif
