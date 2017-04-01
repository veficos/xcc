

#include "config.h"
#include "array.h"
#include "reader.h"
#include "pmalloc.h"
#include "cstring.h"
#include "charop.h"
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
#define STREAM_STASHED_SIZE    (12)
#endif


#ifndef READER_STREAM_DEPTH
#define READER_STREAM_DEPTH         (8)
#endif

static inline sstream_t __sstream_create__(cstring_t text);
static inline bool __sstream_stash__(sstream_t ss, int ch);
static inline int __sstream_unstash__(sstream_t ss);

static inline int __stream_next__(stream_t stream);
static inline int __stream_peek__(stream_t stream);
static inline int __stream_last__(stream_t stream);
static inline bool __stream_untread__(stream_t stream, int ch);

static inline bool __reader_skip_backslash_space__(reader_t reader);


sstream_t sstream_create(const char *s)
{
    return sstream_create_n(s, strlen(s));
}


sstream_t sstream_create_n(const void *data, size_t n)
{
    cstring_t buf;
    sstream_t ss;

    if ((buf = cstring_create_n(data, n)) == NULL) {
        return NULL;
    }

    if ((ss = __sstream_create__(buf)) == NULL) {
        cstring_destroy(buf);
        return NULL;
    }

    return ss;
}


void sstream_destroy(sstream_t ss)
{
    assert(ss != NULL);

    cstring_destroy(ss->text);

    if (ss->stashed != NULL) {
        cstring_destroy(ss->stashed);
    }

    pfree(ss);
}


int sstream_next(sstream_t ss)
{
    int ch;

    if (ss->lastch == '\r' || ss->lastch == '\n') {
        ss->begin = &ss->text[ss->seek];
    }

    if ((ch = __sstream_unstash__(ss)) != EOF) {
        goto done;
    }

    if ((ch = cstring_length(ss->text) > ss->seek ? ss->text[ss->seek] : '\0') == '\0') {
        return EOF;
    }

    ss->seek++;

    /* "\r\n" or "\r" are canonicalized to "\n" */
    if (ch == '\r') {
        if (cstring_length(ss->text) > ss->seek && ss->text[ss->seek] == '\n') {
            ss->seek++;
        }
        ch = '\n';
    }

done:
    ss->lastch = ch;
    return ch;
}


int sstream_peek(sstream_t sstream)
{
    int ch;

    if ((sstream->stashed != NULL) &&
        (cstring_length(sstream->stashed) > 0)) {
        return sstream->stashed[cstring_length(sstream->stashed) - 1];
    }

    if ((cstring_length(sstream->text) < sstream->seek) ||
        (ch = sstream->text[sstream->seek]) == '\0') {
        return EOF;
    }

    return ch;
}


bool sstream_untread(sstream_t ss, int ch)
{
    return ch == EOF ? false : __sstream_stash__(ss, ch);
}


cstring_t sstream_row(sstream_t ss)
{
    size_t i = ss->seek;

    do {
        if (ss->text[i] == '\r' || ss->text[i] == '\n') {
            break;
        }
    } while (i++ < cstring_length(ss->text));

    return cstring_create_n(ss->begin, ((char *)&ss->text[i] - (char *)ss->begin));
}


const char *sstream_name(sstream_t ss)
{
    (void) ss;
    return "<string>";
}


fstream_t fstream_create(const char *fn)
{
    fstream_t fs;
    void *buf;
    FILE *fp;
    int readn;
    struct stat st;

    if ((fp = fopen(fn, "rb")) == NULL) {
        goto done;
    }

    if (fstat(fileno(fp), &st) != 0) {
        goto clean_fp;
    }

    if ((buf = pmalloc(st.st_size)) == NULL) {
        goto clean_fp;
    }

    if ((readn = fread(buf, sizeof(unsigned char), st.st_size, fp)) != st.st_size) {
        goto clean_buf;
    }

    if ((fs = (fstream_t) pmalloc(sizeof(struct fstream_s))) == NULL) {
        goto clean_buf;
    }

    if ((fs->fn = cstring_create(fn)) == NULL) {
        goto clean_fs;
    }

    if ((fs->ss = sstream_create_n(buf, st.st_size)) == NULL) {
        goto clean_fn;
    }

    fclose(fp);
    pfree(buf);
    return fs;

clean_fn:
    cstring_destroy(fs->fn);

clean_fs:
    pfree(fs);

clean_buf:
    pfree(buf);

clean_fp:
    fclose(fp);

done:
    return NULL;
}


void fstream_destroy(fstream_t fs)
{
    assert(fs != NULL);

    cstring_destroy(fs->fn);
    sstream_destroy(fs->ss);
    pfree(fs);
}


bool stream_init(stream_t stream, stream_type_t type, const char *s)
{
    assert(stream != NULL);

    switch (type) {
    case STREAM_TYPE_FILE:
        if ((stream->fs = fstream_create(s)) == NULL) {
            return false;
        }
        break;
    case STREAM_TYPE_STRING:
        if ((stream->ss = sstream_create(s)) == NULL) {
            return false;
        }
        break;
    default:
        assert(false);
    }

    stream->type = type;
    stream->line = 1;
    stream->column = 1;
    return true;
}


void stream_uninit(stream_t stream)
{
    assert(stream != NULL);
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        fstream_destroy(stream->fs);
        break;
    case STREAM_TYPE_STRING:
        sstream_destroy(stream->ss);
        break;
    default:
        assert(false);
    }
}


int stream_next(stream_t stream)
{
    int ch;

    ch = __stream_next__(stream);
    if (ch == '\n') {
        stream->line++;
        stream->column = 1;
    } else if (ch != EOF) {
        stream->column++;
    }

    return ch;
}


int stream_peek(stream_t stream)
{
    int ch;
    ch = stream_next(stream);
    if (ch != EOF) stream_untread(stream, ch);
    return ch;
}


bool stream_untread(stream_t stream, int ch)
{
    assert(ch != EOF);

    if (__stream_untread__(stream, ch)) {
        if (ch == '\n') {
            stream->column = 1;
            stream->line--;
        } else {
            stream->column--;
        }
        return true;
    }

    return false;
}


cstring_t stream_row(stream_t stream)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_row(stream->fs);
    case STREAM_TYPE_STRING:
        return sstream_row(stream->ss);
    }
    return NULL;
}


const char *stream_name(stream_t stream)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_name(stream->fs);
    case STREAM_TYPE_STRING:
        return sstream_name(stream->ss);
    }
    return NULL;
}


bool stream_try(stream_t stream, int ch)
{
    if (stream_peek(stream) == ch) {
        stream_next(stream);
        return true;
    }
    return false;
}


bool stream_test(stream_t stream, int ch)
{
    return stream_peek(stream);
}


reader_t reader_create(option_t op, diag_t diag)
{
    reader_t reader;

    if ((reader = (reader_t) pmalloc(sizeof(struct reader_s))) == NULL) {
        return NULL;
    }

    if ((reader->streams = array_create_n(sizeof(struct reader_s), READER_STREAM_DEPTH)) == NULL) {
        pfree(reader);
        return NULL;
    }

    reader->option = op;
    reader->diag = diag;
    return reader;
}


void reader_destroy(reader_t reader)
{
    stream_t streams;
    size_t i;

    assert(reader != NULL);

    array_foreach(reader->streams, streams, i) {
        stream_uninit(&streams[i]);
    }

    array_destroy(reader->streams);

    pfree(reader);
}


stream_t reader_push(reader_t reader, stream_type_t type, const char *s)
{
    stream_t stream;

    if ((stream = array_push(reader->streams)) == NULL) {
        return NULL;
    }

    if (!stream_init(stream, type, s)) {
        return NULL;
    }

    reader->last = stream;
    return stream;
}


void reader_pop(reader_t reader)
{
    stream_t streams;

    assert(array_size(reader->streams) > 0);

    streams = array_prototype(reader->streams, struct stream_s);
    
    stream_uninit(&(streams[array_size(reader->streams) - 1]));

    array_pop(reader->streams);

    reader->last = &(streams[array_size(reader->streams) - 1]);
}


int reader_next(reader_t reader)
{
    for (;;) {
        int ch = stream_next(reader->last);

        /* 	compatible with \nEOF */
        if (ch == EOF) {
            if (array_size(reader->streams) == 1) {
                ch = (__stream_last__(reader->last) == '\n' || 
                      __stream_last__(reader->last) == EOF) ? EOF : '\n';
                return ch;
            }
            reader_pop(reader);
            continue;
        }

        /*
         *   Each instance of a backslash character(\) immediately
         *   followed by a newline character is deleted, splicing physical
         *   source lines to form logical source lines
         */
        if (ch == '\\' && __reader_skip_backslash_space__(reader)) {
            continue;
        }

        return ch;
    }

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
    return stream_untread(reader->last, ch);
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
sstream_t __sstream_create__(cstring_t text)
{
    sstream_t ss;

    if ((ss = (sstream_t) pmalloc(sizeof(struct sstream_s))) == NULL) {
        return NULL;
    }

    ss->text = text;
    ss->begin = ss->text;
    ss->stashed = NULL;
    ss->seek = 0;
    ss->lastch = 0;
    return ss;
}


static inline
bool __sstream_stash__(sstream_t ss, int ch)
{
    if (ss->stashed == NULL) {
        if ((ss->stashed = cstring_create_n(NULL, STREAM_STASHED_SIZE)) == NULL) {
            return false;
        }
    }

    if ((ss->stashed = cstring_push_ch(ss->stashed, ch)) == NULL) {
        return false;
    }

    return true;
}


static inline
int __sstream_unstash__(sstream_t ss)
{
    return ss->stashed == NULL || cstring_length(ss->stashed) <= 0 ? \
        EOF : cstring_pop_ch(ss->stashed);
}


static inline
int __stream_last__(stream_t stream)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_last(stream->fs);
    case STREAM_TYPE_STRING:
        return sstream_last(stream->ss);
    default:
        assert(false);
    }
    return EOF;
}


static inline 
int __stream_next__(stream_t stream)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_next(stream->fs);
    case STREAM_TYPE_STRING:
        return sstream_next(stream->ss);
    default:
        assert(false);
    }
    return EOF;
}


static inline 
int __stream_peek__(stream_t stream)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_peek(stream->fs);
    case STREAM_TYPE_STRING:
        return sstream_peek(stream->ss);
    default:
        assert(false);
    }
    return EOF;
}


static inline 
bool __stream_untread__(stream_t stream, int ch)
{
    switch (stream->type) {
    case STREAM_TYPE_FILE:
        return fstream_untread(stream->fs, ch);
    case STREAM_TYPE_STRING:
        return sstream_untread(stream->ss, ch);
    default:
        assert(false);
    }
    return false;
}


static inline
bool __reader_skip_backslash_space__(reader_t reader)
{
    size_t line, column;

    line = stream_line(reader->last);
    column = stream_column(reader->last) - 1;

    if (stream_try(reader->last, '\n')) {
        return true;
    }

    for (;;) {
        int ch = stream_next(reader->last);

        if (ch == EOF) {
            diag_warningf_with_line(reader->diag, line, column,
                stream_name(reader->last), "backslash-newline at end of file");
            return true;
        }
       
        if (ch == '\n') {
            diag_warningf_with_line(reader->diag, line, column,
                stream_name(reader->last), "backslash and newline separated by space");
            return true;
        }

        if (!chisspace(ch)) {
            stream_untread(reader->last, ch);
            break;
        }
    }

    return false;
}
