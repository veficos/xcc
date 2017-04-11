

#include "config.h"
#include "token.h"
#include "vpfmt.h"
#include "color.h"
#include "diag.h"


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
    vpfmt(stdout, fmt, ap);
    fprintf(stdout, "\n");
    diag->nerrors++;
}


void diag_errorf(diag_t diag, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vpfmt(stdout, fmt, ap);
    fprintf(stdout, "\n");

    va_end(ap);
}


void diag_fmt(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vpfmt(stdout, fmt, ap);
    fprintf(stdout, "\n");

    va_end(ap);
}


void diag_panic(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vpfmt(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    exit(-1);
}
