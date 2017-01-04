

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void diag_errorf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    exit(-1);
}
