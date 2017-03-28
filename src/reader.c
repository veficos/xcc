

#include "config.h"
#include "array.h"
#include "reader.h"
#include "pmalloc.h"
#include "cstring.h"


/*
*
* Notice
*
* 1. C11 5.1.1: "\r\n" or "\r" are canonicalized to "\n".
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
*       source files are comforming, this step wouldn't be needed.)
*/


#ifndef MAX_STASHED_SIZE
#define MAX_STASHED_SIZE    (12)
#endif


static inline string_reader_t __string_reader_create__(cstring_t text);
static inline int __string_reader_next__(string_reader_t sr);
static inline int __string_reader_peek__(string_reader_t sr);
static inline bool __string_reader_stash__(string_reader_t sr, int ch);
static inline int __string_reader_unstash__(string_reader_t sr);
static inline bool __screader_skip_backslash_space__(screader_t screader);

string_reader_t string_reader_create_n(const void *data, size_t n)
{
    cstring_t buf;

    if ((buf = cstring_create_n(data, n)) == NULL) {
        return NULL;
    }

    return __string_reader_create__(buf);
}


void string_reader_destroy(string_reader_t sr)
{
    assert(sr != NULL);

    cstring_destroy(sr->text);

    if (sr->stashed != NULL) {
        cstring_destroy(sr->stashed);
    }

    pfree(sr);
}


int string_reader_next(string_reader_t sr)
{
    int ch;

    if (((ch = __string_reader_next__(sr)) == '\r') &&
        __string_reader_peek__(sr) == '\n') {
        ch = __string_reader_next__(sr);
    }

    if (ch != EOF) {
        if (ch == '\n') {
            sr->line++;
            sr->column = 1;
        }
        else {
            sr->column++;
        }
    }
         
    sr->lastch = ch;
    return ch;
}


int string_reader_peek(string_reader_t sr)
{
    int ch;

    if (((ch = __string_reader_peek__(sr)) == '\r')) {
        string_reader_next(sr);
        if (__string_reader_peek__(sr) == '\n') {
            return '\n';
        }
        string_reader_untread(sr, ch);
    }

    return ch;
}


bool string_reader_untread(string_reader_t sr, int ch)
{
    if (ch == EOF) {
        return false;
    }

    if (__string_reader_stash__(sr, ch)) {
        if (ch == '\n') {
            sr->column = 1;
            sr->line--;
        } else {
            sr->column--;
        }
        return true;
    }
    return false;
}


cstring_t string_reader_row(string_reader_t sr)
{
    size_t i = sr->seek;

    do {
        if (sr->text[i] == '\n') {
            break;
        }
    } while(i++ < cstring_length(sr->text));

    return cstring_create_n(sr->begin, ((char *)&sr->text[sr->text[i - 1] == '\r' ? --i : i] - (char *)sr->begin));
}


file_reader_t file_reader_create(const char *filename)
{
    file_reader_t fr;
    void *buf;
    FILE *fp;
    int readn;
    struct stat st;

    if ((fp = fopen(filename, "rb")) == NULL) {
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

    if ((fr = (file_reader_t) pmalloc(sizeof(struct file_reader_s))) == NULL) {
        goto clean_buf;
    }

    if ((fr->filename = cstring_create(filename)) == NULL) {
        goto clean_fr;
    }

    if ((fr->sreader = string_reader_create_n(buf, st.st_size)) == NULL) {
        goto clean_fn;
    }
    
    fclose(fp);
    pfree(buf);
    return fr;

clean_fn:
    cstring_destroy(fr->filename);

clean_fr:
    pfree(fr);

clean_buf:
    pfree(buf);

clean_fp:
    fclose(fp);

done:
    return NULL;
}


void file_reader_destroy(file_reader_t fr)
{
    assert(fr != NULL);

    cstring_destroy(fr->filename);

    string_reader_destroy(fr->sreader);

    pfree(fr);
}


screader_t screader_create(stream_type_t type, const char *s)
{
    reader_t reader;
    screader_t screader;

    if ((screader = (screader_t) pmalloc(sizeof(struct screader_s))) == NULL) {
        goto done;
    }

    if ((screader->files = array_create_n(sizeof(struct reader_s), 8)) == NULL) {
        goto clean_scr;
    }
    
    if ((reader = array_push(screader->files)) == NULL) {
        goto clean_scr;
    }

    if (reader_init(reader, type, s) == NULL) {
        goto clean_array;
    }

    screader->last = reader;
    return screader;

clean_array:
    array_destroy(screader->files);

clean_scr:
    pfree(screader);

done:
    return NULL;
}


void screader_destroy(screader_t screader)
{
    reader_t readers;
    size_t i;

    assert(screader != NULL);

    array_foreach(screader->files, readers, i) {
        reader_uninit(&readers[i]);
    }

    array_destroy(screader->files);

    pfree(screader);
}


reader_t screader_push(screader_t screader, stream_type_t type, const char *s)
{
    reader_t reader;

    if ((reader = array_push(screader->files)) == NULL) {
        return NULL;
    }

    if (reader_init(reader, type, s) == NULL) {
        return NULL;
    }

    screader->last = reader;
    return reader;
}


void screader_pop(screader_t screader)
{
    if (array_size(screader->files) > 1) {
        reader_t readers = array_prototype(screader->files, struct reader_s);
        reader_uninit(&(readers[array_size(screader->files) - 1]));
        array_pop(screader->files);
        screader->last = &(readers[array_size(screader->files) - 1]);
    }
}


int screader_next(screader_t screader)
{
    for (;;) {
        int ch = reader_next(screader->last);
        if (ch == EOF) {
            screader_pop(screader);
            if (array_size(screader->files) == 1) {
                return ch;
            }
            continue;
        }

        if (ch == '\\' && __screader_skip_backslash_space__(screader)) {
            continue;
        }

        return ch;
    }

    return EOF;
}


int screader_peek(screader_t screader)
{
    for (;;) {
        int ch = reader_next(screader->last);
        if (ch == EOF) {
            screader_pop(screader);
            if (array_size(screader->files) == 1) {
                return EOF;
            }
            continue;
        }

        if (ch == '\\' && __screader_skip_backslash_space__(screader)) {
            continue;
        }

        reader_untread(screader->last, ch);
        return ch;
    }

    return EOF;
}


static inline
bool __screader_skip_backslash_space__(screader_t screader)
{
    if (reader_try(screader->last, '\n')) {
        return true;
    }
    
    for (;;) {
        int ch = reader_next(screader->last);
        if (!isspace(ch)) {
            break;
        }

        if (ch == EOF) {
            /* TODO: backslash-newline at end of file */
            return true;
        }

        if (ch == '\n') {
            /* TODO: backslash and newline separated by space */
            return true;
        }
    }

    return false;
}


static inline
string_reader_t __string_reader_create__(cstring_t text)
{
    string_reader_t sr;

    if ((sr = (string_reader_t) pmalloc(sizeof(struct string_reader_s))) == NULL) {
        return NULL;
    }

    sr->text = text;
    sr->begin = sr->text;
    sr->stashed = NULL;
    sr->line = 1;
    sr->column = 1;
    sr->seek = 0;
    sr->lastch = EOF;
    return sr;
}


static inline 
int __string_reader_next__(string_reader_t sr)
{
    int ch;

    if (sr->lastch == '\n') {
        sr->begin = &sr->text[sr->seek];
    }

    if ((ch = __string_reader_unstash__(sr)) != EOF) {
        goto done;
    }

    if ((ch = cstring_length(sr->text) > sr->seek ? sr->text[sr->seek] : '\0') == '\0') {
        return EOF;
    }

    sr->seek++;

done:
    sr->lastch = ch;
    return ch;
}


static inline 
int __string_reader_peek__(string_reader_t sr)
{
    int ch;

    if (sr->stashed != NULL && cstring_length(sr->stashed) > 0) {
        return sr->stashed[cstring_length(sr->stashed) - 1];
    }

    if (cstring_length(sr->text) < sr->seek || (ch = sr->text[sr->seek]) == '\0') {
        return EOF;
    }

    return ch;
}


static inline
bool __string_reader_stash__(string_reader_t sr, int ch)
{
    if (sr->stashed == NULL) {
        if ((sr->stashed = cstring_create_n(NULL, MAX_STASHED_SIZE)) == NULL) {
            return false;
        }
    }

    if ((sr->stashed = cstring_push_ch(sr->stashed, ch)) == NULL){
        return false;
    }

    return true;
}


static inline
int __string_reader_unstash__(string_reader_t sr)
{
    if (sr->stashed == NULL) {
        return EOF;
    }
    return cstring_pop_ch(sr->stashed);
}
