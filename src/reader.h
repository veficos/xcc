

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


typedef const unsigned char* linenote_t;


typedef struct reader_s {
    array_t *streams;
    stream_t *last;
    cspool_t *cspool;
    bool clean_csp;
} reader_t;


reader_t* reader_create(void);
reader_t* reader_create_csp(cspool_t *csp);
void reader_destroy(reader_t *reader);
size_t reader_depth(reader_t *reader);
bool reader_is_empty(reader_t *reader);
bool reader_push(reader_t *reader, stream_type_t type, const unsigned char *s);
void reader_pop(reader_t *reader);
int reader_get(reader_t *reader);
int reader_peek(reader_t *reader);
void reader_unget(reader_t *reader, int ch);
bool reader_try(reader_t *reader, int ch);
bool reader_test(reader_t *reader, int ch);
size_t reader_line(reader_t *reader);
size_t reader_column(reader_t *reader);
cstring_t reader_filename(reader_t *reader);
time_t reader_modify_time(reader_t *reader);
time_t reader_change_time(reader_t *reader);
time_t reader_access_time(reader_t *reader);

linenote_t reader_linenote(reader_t *reader);
cstring_t linenote2cs(linenote_t linenote);


#endif
