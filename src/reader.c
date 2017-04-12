

#include "config.h"
#include "array.h"
#include "reader.h"
#include "pmalloc.h"
#include "cstring.h"
#include "utils.h"
#include "diag.h"


/*
*
* Notice
*
* 1. C11 5.1.1: "\r\n" or "\r" are canonicalized to "\n". 
*    [sstream]
*
* 2. C11 5.1.1: Each instance of a backslash character (\) immediately
*       followed by a new-line character is deleted, splicing physical
*       source lines to form logical source lines.Only the last backslash 
*       on any physical source line shall be eligible for being part of 
*       such a splice. A source file that is not empty shall end in a 
*       new-line character, which shall not be immediately preceded by 
*       a backslash character before any such splicing takes place.
*
*     example:
*         |#inc\
*         |lude <stdio.h>
*    [reader]
*
* 3. C11 5.1.1: EOF not immediately following a newline is converted to
*       a sequence of newline and EOF. (The C spec requires source
*       files end in a newline character (5.1.1.2p2). Thus, if all
*       source files are conforming, this step wouldn't be needed.
*    [reader]
*/


#ifndef STREAM_STASHED_SIZE
#define STREAM_STASHED_SIZE     (12)
#endif


#ifndef READER_STREAM_DEPTH
#define READER_STREAM_DEPTH     (8)
#endif


struct stream_s {
    stream_type_t type;

    cstring_t text; 
    cstring_t fn;

    cstring_t stashed;

    char *row;

    char *pc;
    char *pe;
    char *backup;

    size_t line;
    size_t column;

    time_t mt;

    int lastch;
};


#define STREAM_LINE_ADVANCE(stream) \
    do {                            \
        (stream)->line++;           \
        (stream)->column = 1;       \
    } while (false)


static inline bool __stream_init__(stream_t stream, stream_type_t type, const char *s);
static inline void __stream_uninit__(stream_t stream);
static inline void __stream_stash__(stream_t stream, int ch);
static inline int __stream_unstash__(stream_t stream);
static inline int __stream_next__(stream_t stream);
static inline int __stream_peek__(stream_t stream);


reader_t reader_create(diag_t diag, option_t option)
{
    reader_t reader;
    reader = (reader_t)pmalloc(sizeof(struct reader_s));
    reader->diag = diag;
    reader->option = option;
    reader->streams = array_create_n(sizeof(struct reader_s), READER_STREAM_DEPTH);
    reader->last = NULL;
    return reader;
}


void reader_destroy(reader_t reader)
{
    stream_t streams;
    size_t i;

    assert(reader != NULL);

    array_foreach(reader->streams, streams, i) {
        __stream_uninit__(&streams[i]);
    }

    array_destroy(reader->streams);

    pfree(reader);
}


bool reader_push(reader_t reader, stream_type_t type, const char *s)
{
    stream_t stream;

    stream = array_push(reader->streams);

    if (!__stream_init__(stream, type, s)) {
        return false;
    }

    reader->last = stream;
    return true;
}


void reader_pop(reader_t reader)
{
    stream_t streams;

    assert(array_length(reader->streams) > 0);

    streams = array_prototype(reader->streams, struct stream_s);
    
    __stream_uninit__(&(streams[array_length(reader->streams) - 1]));

    array_pop(reader->streams);

    reader->last = &(streams[array_length(reader->streams) - 1]);
}


int reader_next(reader_t reader)
{
    for (;;) {
        int ch = __stream_next__(reader->last);

        if (ch == EOF) {
            if (array_length(reader->streams) == 1) {
                return ch;
            }
            reader_pop(reader);
            continue;
        }

        return ch;
    }

    return EOF;
}


int reader_peek(reader_t reader)
{
    for (;;) {
        int ch = __stream_peek__(reader->last);

        if (ch == EOF) {
            if (array_length(reader->streams) == 1) {
                return ch;
            }
            reader_pop(reader);
            continue;
        }

        return ch;
    }

    return EOF;
}


bool reader_untread(reader_t reader, int ch)
{
    assert(ch != EOF);
    return false;
}


bool reader_try(reader_t reader, int ch)
{
    if (reader_peek(reader) == ch) {
        reader_next(reader);
        return true;
    }
    return false;
}


bool reader_test(reader_t reader, int ch)
{
    return reader_peek(reader) == ch;
}


const char *reader_row(reader_t reader)
{
    return reader->last->row;
}


size_t reader_line(reader_t reader)
{
    return reader->last->line;
}


size_t reader_column(reader_t reader)
{
    return reader->last->column;
}


bool reader_is_empty(reader_t reader)
{
    return reader_peek(reader) == EOF;
}


cstring_t reader_name(reader_t reader)
{
    return reader->last->fn;
}


cstring_t row2line(const char *row)
{
    const char *begin = row;
    for (;*row;) {
        if (*row == '\r' || *row == '\n') {
            break;
        }
        row++;
    }
    return cstring_create_n(begin, row - begin);
}


static inline 
bool __stream_init__(stream_t stream, stream_type_t type, const char *s)
{
    switch (type) {
    case STREAM_TYPE_FILE: {
        void *buf;
        FILE *fp;
        struct stat st;

        if ((fp = fopen(s, "rb")) == NULL) {
            return false;
        }

        if (fstat(fileno(fp), &st) != 0) {
            goto failure;
        }

        buf = pmalloc(st.st_size);
        if (fread(buf, sizeof(unsigned char), st.st_size, fp) != st.st_size) {
            goto failure;
        }

        fclose(fp);
        stream->fn = cstring_create(s);
        stream->mt = st.st_mtime;
        stream->text = cstring_create_n(buf, st.st_size);
        break;
    failure:
        fclose(fp);
        return false;
    }
    case STREAM_TYPE_STRING: {
        stream->fn = cstring_create("<string>");
        stream->mt = 0;
        stream->text = cstring_create_n(s, strlen(s));
        break;
    }
    default:
        assert(false);
    }

    stream->type = type;
    stream->stashed = NULL;
    stream->pc = stream->row = (char *) stream->text;
    stream->pe = (char *) &stream->text[cstring_length(stream->text)];
    stream->backup = NULL;
    stream->line = 1;
    stream->column = 1;
    stream->lastch = EOF;
    return true;
}


static inline
void __stream_uninit__(stream_t stream)
{
    cstring_destroy(stream->fn);
    cstring_destroy(stream->text);
    if (stream->stashed != NULL) {
        cstring_destroy(stream->stashed);
    }
}


static inline
void __stream_stash__(stream_t stream, int ch)
{
    assert(ch != EOF);
    if (stream->stashed == NULL) {
        stream->stashed = cstring_create_n(NULL, STREAM_STASHED_SIZE);
    }
    stream->stashed = cstring_push_ch(stream->stashed, ch);
}


static inline
int __stream_unstash__(stream_t stream)
{
    return stream->stashed == NULL || cstring_length(stream->stashed) <= 0 ? \
        EOF : cstring_pop_ch(stream->stashed);
}


static inline
int __stream_next__(stream_t stream)
{
    int ch;

    if ((ch = __stream_unstash__(stream)) != EOF) {
        goto done;
    }

    if (stream->lastch == '\n') {
        stream->row = stream->pc;
    }
    
nextch:
    if (stream->pc >= stream->pe || (ch = *stream->pc) == '\0') {
        ch = stream->lastch == '\n' || 
            stream->lastch == EOF ? EOF : '\n';
        goto done;
    }

    stream->pc++;

    if (ch == '\r') {
        /* 
         * "\r\n" or "\r" are canonicalized to "\n" 
         */

        if (*stream->pc == '\n') {
            stream->pc++;
        }

        ch = '\n';
        STREAM_LINE_ADVANCE(stream);

    } else if (ch == '\n') {
        STREAM_LINE_ADVANCE(stream);

    } else if (ch == '\\') {
        /*
        * Each instance of a backslash character(\) immediately
        * followed by a newline character is deleted, splicing 
        * physical source lines to form logical source lines
        */

        char *pc = stream->pc;
        while (pc < stream->pe && ISSPACE(*pc)) {
            switch (*pc) {
            case '\r':
                if (*(pc+1) == '\n') {
                    pc++;
                }
            case '\n':
                stream->pc = pc + 1;
                STREAM_LINE_ADVANCE(stream);
                goto nextch;
            }
            pc++;
        }

        if (pc == stream->pe) {
            /* warn... */
            ch = '\n';
            stream->pc = pc;

            /*
                diag_fmt("%s:%d:%d: %s", 
                    stream->fn, 
                    stream->line, 
                    stream->column, 
                    "backslash and newline separated by space");
            */
        }

    } else {
        stream->column++;
    }

done:
    stream->lastch = ch;
    return ch;
}


static inline
int __stream_peek__(stream_t stream)
{
    int ch;
    char *pc;

    if (stream->stashed != NULL &&
        cstring_length(stream->stashed) > 0) {
        return stream->stashed[cstring_length(stream->stashed) - 1];
    }

    pc = stream->pc;
nextch:
    if (pc >= stream->pe || (ch = *pc) == '\0') {
        return stream->lastch == '\n' ||
            stream->lastch == EOF ? EOF : '\n';
    }

    pc++;

    switch (ch) {
    case '\r':
    case '\n':
        return '\n';
    case '\\':
        while (pc < stream->pe && ISSPACE(*pc)) {
            switch (*pc) {
            case '\r':
                if (*(pc + 1) == '\n') {
                    pc++;
                }
            case '\n':
                pc++;
                goto nextch;
            }
            pc++;
        }
        if (pc == stream->pe) {
            return '\n';
        }
    }
    
    return ch;
}
