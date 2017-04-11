

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


static inline bool __stream_init__(stream_t stream, stream_type_t type, const char *s);
static inline void __stream_uninit__(stream_t stream);


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
    return EOF;
}


int reader_peek(reader_t reader)
{
    int ch = reader_next(reader);
    if (ch != EOF) reader_untread(reader, ch);
    return ch;
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
        ch = EOF;
        goto done;
    }

    stream->pc++;

    if (ch == '\r') {
        /* "\r\n" or "\r" are canonicalized to "\n" */

        if (*stream->pc == '\n') {
            stream->pc++;
        }

        ch = '\n';
        stream->line++;
        stream->column = 1;

    } else if (ch == '\\') {
        /*
        *   Each instance of a backslash character(\) immediately
        *   followed by a newline character is deleted, splicing physical
        *   source lines to form logical source lines
        */

        char *pc = stream->pc + 1;
        while (pc < stream->pe) {
            if (*pc == '\r') {
                if (*++pc == '\n') {
                    pc++;
                }
                stream->pc = pc;
                goto nextch;
            } else if (*pc == '\n') {
                stream->pc = pc + 1;
                goto nextch;
            } else if (!ISSPACE(*pc)) {
                break;
            }
            stream->column++;
        }

        if (pc == stream->pe) {
            /* error... */
            ch = EOF;
        }

    } else {
        stream->column++;
    }

done:
    stream->lastch = ch;
    return ch;
}


int __stream_peek__(stream_t stream)
{
    int ch;

    ch = stream->stashed == NULL || cstring_length(stream->stashed) <= 0 ? 
        EOF : cstring_pop_ch(stream->stashed);
}


static inline
void __stream__(stream_t stream)
{
    assert(stream->backup == NULL);
    stream->backup = stream->pc;
}


static inline
void __stream_recover__(stream_t stream)
{
    assert(stream->backup != NULL);
    stream->pc = stream->backup;
    stream->backup = NULL;
}


static inline
cstring_t __stream_commit__(stream_t stream)
{
    cstring_t cs;

    assert(stream->backup != NULL);

    cs = cstring_create_n(stream->backup, (size_t)((char*)stream->pc - (char*)stream->backup));
    stream->backup = NULL;
    return cs;
}
