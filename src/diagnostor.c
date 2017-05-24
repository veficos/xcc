

#include "config.h"
#include "token.h"
#include "color.h"
#include "diagnostor.h"


#ifndef MAX_COLUMN_HEAD
#define MAX_COLUMN_HEAD  64
#endif


static void __write_one_line__(FILE *fp, const char *p);


diag_t diag_create(void)
{
    diag_t diag = pmalloc(sizeof(struct diag_s));
    diag->nwarnings = 0;
    diag->nerrors = 0;
    return diag;
}


void diag_destroy(diag_t diag)
{
    assert(diag != NULL);
    pfree(diag);
}


void diag_report(diag_t diag)
{
    if (diag->nwarnings != 0 && diag->nerrors != 0) {
        fprintf(stderr, "%d warning and %d error generated.\n", diag->nwarnings, diag->nerrors);
    }

    if (diag->nwarnings != 0) {
        fprintf(stderr, "%d warning generated.\n", diag->nwarnings);
    }

    if (diag->nerrors != 0) {
        fprintf(stderr, "%d error generated.\n", diag->nerrors);
    }
}


void diag_errorvf(diag_t diag, const char *fmt, va_list ap)
{
    fprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    diag->nerrors++;
}


void diag_errorf(diag_t diag, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");

    va_end(ap);
}


void diag_fmt(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");

    va_end(ap);
}


void diag_panic(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    exit(-1);
}


void debug_linenote(const char* linenote, size_t start, size_t tilde)
{
    const char *p = linenote;

    if (start >= MAX_COLUMN_HEAD) {

        fprintf(stderr, "   ...");

        __write_one_line__(stderr, p + start);

        fprintf(stderr, BRUSH_RED("\n      ^\n"));

    } else {
        const char *q = p + (start - 1);
        size_t step;

        __write_one_line__(stderr, p);

        fprintf(stderr, "\n");

        for (; p < q; ) {
            step = utf8_rune_size(*p);
            p += step;
            if (step > 1) {
                fprintf(stderr, "   ");
            } else {
                fprintf(stderr, " ");
            }
        }

        fprintf(stderr, BRUSH_RED("^"));

        while (tilde && --tilde) {
            fprintf(stderr, BRUSH_GREEN("~"));
        }

        fprintf(stderr, "\n");
    }
}


static void __write_one_line__(FILE *fp, const char *p)
{
    for (;*p != '\r' && *p != '\n' && *p; p++) {
        fputc(*p, fp);
    }
}
