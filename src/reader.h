

#ifndef __READER__H__
#define __READER__H__


#include "config.h"


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


#define STREAM_TEXT(stream)                                                             \
    (stream)->type == STREAM_TYPE_FILE ? (stream)->fs->ss->text :                       \
        (stream)->type == STREAM_TYPE_STRING ? (stream)->ss->text : NULL

#define STREAM_NAME(stream)                                                             \
    (stream)->type == STREAM_TYPE_FILE ? (const char*)(stream)->fs->fn :                \
        (stream)->type == STREAM_TYPE_STRING ? "<string>" : NULL
    




/* reader interface */

#define reader_line(reader)         stream_line((reader)->last)
#define reader_column(reader)       stream_column((reader)->last)
#define reader_row(reader)          stream_row((reader)->last)
#define reader_name(reader)         stream_name((reader)->last)
#define reader_is_empty(reader)     (reader_peek(reader) == EOF)

reader_t reader_create(diag_t diag, option_t op);
void reader_destroy(reader_t reader);
bool reader_push(reader_t reader, stream_type_t type, const char *s);
int reader_next(reader_t reader);
int reader_peek(reader_t reader);
bool reader_untread(reader_t reader, int ch);
bool reader_try(reader_t reader, int ch);
bool reader_test(reader_t reader, int ch);


#endif
