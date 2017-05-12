

#include "config.h"
#include "array.h"
#include "pmalloc.h"
#include "cstring.h"
#include "cspool.h"
#include "utils.h"
#include "reader.h"
#include "diag.h"


/**
* Notices
*
* 1. C11 5.1.1: "\r\n" or "\r" are canonicalized to "\n". 
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
*
* 3. C11 5.1.1: EOF not immediately following a newline is converted to
*       a sequence of newline and EOF. (The C spec requires source
*       files end in a newline character (5.1.1.2p2). Thus, if all
*       source files are conforming, this step wouldn't be needed.
*/


#ifndef STREAM_STASHED_SIZE
#define STREAM_STASHED_SIZE     (12)
#endif


#ifndef READER_STREAM_DEPTH
#define READER_STREAM_DEPTH     (8)
#endif


struct stream_s {
    stream_type_t type;

    cstring_t fn;

    cstring_t stashed;

    linenote_t line_note;

    unsigned char *pc;
    unsigned char *pe;

    size_t line;
    size_t column;

    time_t mt;

    int lastch;
};


#define __WARNINGF__(fmt, ...)                                                              \
    diag_warningf_with_line(reader->diag, stream->fn, stream->line, stream->column,    \
        stream->line_note, stream->column, 1, __VA_ARGS__);


#define STREAM_LINE_ADVANCE(stream)         \
    do {                                    \
        (stream)->line++;                   \
        (stream)->column = 1;               \
        (stream)->line_note = (stream)->pc; \
    } while (false)


static inline bool __stream_init__(reader_t reader, stream_t stream, stream_type_t type, const unsigned char *s);
static inline void __stream_uninit__(stream_t stream);
static inline void __stream_stash__(stream_t stream, int ch);
static inline int __stream_unstash__(stream_t stream);
static inline int __stream_next__(reader_t reader, stream_t stream);
static inline int __stream_peek__(stream_t stream);


reader_t reader_create(diag_t diag, option_t option)
{
    reader_t reader = (reader_t) pmalloc(sizeof(struct reader_s));
    reader->diag = diag;
    reader->option = option;
    reader->pool = cspool_create();
    reader->streams = array_create_n(sizeof(struct stream_s), READER_STREAM_DEPTH);
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

    cspool_destroy(reader->pool);

    pfree(reader);
}


size_t reader_level(reader_t reader)
{
    return array_length(reader->streams);
}


bool reader_push(reader_t reader, stream_type_t type, const unsigned char *s)
{
    stream_t stream;

    stream = array_push_back(reader->streams);

    if (!__stream_init__(reader, stream, type, s)) {
        return false;
    }

    reader->last = stream;
    return true;
}


time_t reader_mt(reader_t reader)
{
    if (reader->last == NULL) return 0;
    return reader->last->mt;
}


void reader_pop(reader_t reader)
{
    assert(!array_is_empty(reader->streams));

    __stream_uninit__(reader->last);

    array_pop_back(reader->streams);

    if (array_is_empty(reader->streams)) {
        reader->last = NULL;
    } else {
        reader->last = &(array_cast_back(struct stream_s, reader->streams));
    }
}


int reader_get(reader_t reader)
{
    if (reader->last != NULL) {
        int ch = __stream_next__(reader, reader->last);

        if (ch == EOF) {
            reader_pop(reader);
        }

        return ch;
    }
    return EOF;
}


int reader_peek(reader_t reader)
{
    int ch = EOF;
    if (reader->last != NULL) {
        ch = __stream_peek__(reader->last);
    }
    return ch;
}


void reader_unget(reader_t reader, int ch)
{
    assert(ch != EOF);
    __stream_stash__(reader->last, ch);
}


bool reader_try(reader_t reader, int ch)
{
    if (reader_peek(reader) == ch) {
        reader_get(reader);
        return true;
    }
    return false;
}


bool reader_test(reader_t reader, int ch)
{
    return reader_peek(reader) == ch;
}


linenote_t reader_linenote(reader_t reader)
{
    if (reader->last == NULL) return NULL;
    return reader->last->line_note;
}


size_t reader_line(reader_t reader)
{
    if (reader->last == NULL) return 0;
    return reader->last->line;
}


size_t reader_column(reader_t reader)
{
    if (reader->last == NULL) return 0;
    return reader->last->column;
}


cstring_t reader_name(reader_t reader)
{
    if (reader->last == NULL) return NULL;
    return reader->last->fn;
}


bool reader_is_empty(reader_t reader)
{
    return (reader_peek(reader) == EOF) && (reader_level(reader) == 0);
}


bool reader_is_eos(reader_t reader)
{
    return (reader_peek(reader) == EOF);
}


cstring_t linenote2cs(linenote_t linenote)
{
    const unsigned char *p = (const unsigned char *)linenote;

    for (;*p;) {
        if (*p == '\r' || *p == '\n') {
            break;
        }
        p++;
    }

    return cstring_create_n((const unsigned char*)linenote, p - (const unsigned char *)linenote);
}


static inline 
bool __stream_init__(reader_t reader, stream_t stream, stream_type_t type, const unsigned char *s)
{
    cstring_t text = NULL;

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
       
        stream->fn = cspool_push_cs(reader->pool, cstring_create(s));
        stream->mt = st.st_mtime;
        text = cspool_push_cs(reader->pool, cstring_create_n(buf, st.st_size));

        pfree(buf);
        fclose(fp);
        break;
    failure:
        fclose(fp);
        return false;
    }
    case STREAM_TYPE_STRING: {
        stream->fn = cspool_push(reader->pool, "<string>");
        stream->mt = 0;
        text = cspool_push(reader->pool, s);
        break;
    }
    default:
        assert(false);
    }

    stream->type = type;
    stream->stashed = NULL;
    stream->line_note = stream->pc = (unsigned char *)text;
    stream->pe = (unsigned char *)&text[cstring_length(text)];
    stream->line = 1;
    stream->column = 1;
    stream->lastch = EOF;
    return true;
}


static inline
void __stream_uninit__(stream_t stream)
{
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
int __stream_next__(reader_t reader, stream_t stream)
{
    int ch;

    if ((ch = __stream_unstash__(stream)) != EOF) {
        goto done;
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

        unsigned char *pc = stream->pc;
        uintptr_t step = 0;
        while (pc < stream->pe && ISSPACE(*pc)) {
            switch (*pc) {
            case '\r':
                if (*(pc + 1) == '\n') {
                    pc++;
                    step++;
                }
            case '\n':
                if (pc > stream->pc + step) {
                    __WARNINGF__("backslash and newline separated by space");
                }

                stream->pc = pc + 1;
                STREAM_LINE_ADVANCE(stream);
                goto nextch;
            }
            pc++;
        }

        if (pc == stream->pe) {
            __WARNINGF__("backslash-newline at end of file");

            ch = '\n';
            stream->pc = pc;
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
    unsigned char *pc;

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
