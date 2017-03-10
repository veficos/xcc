

#include "config.h"
#include "diag.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


#ifdef USE_CONSOLE_COLOR
#define CONSOLE_COLOR_RED     "\x1b[31m"
#define CONSOLE_COLOR_GREEN   "\x1b[32m"
#define CONSOLE_COLOR_YELLOW  "\x1b[33m"
#define CONSOLE_COLOR_BLUE    "\x1b[34m"
#define CONSOLE_COLOR_MAGENTA "\x1b[35m"
#define CONSOLE_COLOR_CYAN    "\x1b[36m"
#define CONSOLE_COLOR_DEFAULT "\x1b[0m"
#else 
#define CONSOLE_COLOR_RED     
#define CONSOLE_COLOR_GREEN  
#define CONSOLE_COLOR_YELLOW 
#define CONSOLE_COLOR_BLUE   
#define CONSOLE_COLOR_MAGENTA 
#define CONSOLE_COLOR_CYAN
#define CONSOLE_COLOR_DEFAULT 
#endif 


static inline void __diag_errorvf_with_line__(source_location_t loc, const char *fmt, va_list ap);


void diag_errorvf(const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}


void diag_errorf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    exit(-1);
}


void diag_errorvf_with_line(source_location_t loc, const char *fmt, va_list ap)
{
    __diag_errorvf_with_line__(loc, fmt, ap);
    exit(-1);
}


void diag_errorf_with_line(source_location_t loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    __diag_errorvf_with_line__(loc, fmt, ap);

    va_end(ap);
}


static inline
void __diag_errorvf_with_line__(source_location_t loc, const char *fmt, va_list ap)
{
    bool pass_spaces = false;
    int nspaces = 0;
    const char *p;

    fprintf(stderr, 
            "%s:%d:%d: " CONSOLE_COLOR_RED "error: " CONSOLE_COLOR_DEFAULT, 
            loc->filename, 
            loc->line, 
            loc->column);

    vfprintf(stderr, fmt, ap);

    fprintf(stderr, "\n");

    if (loc->column >= MAX_COLUMN_HEAD) {
        p = loc->current_line;

        fprintf(stderr, "    ...");
        fprintf(stderr, p + loc->column);
        fprintf(stderr, CONSOLE_COLOR_GREEN "\n       ^\n" CONSOLE_COLOR_DEFAULT);

    } else {
        for (p = loc->current_line; *p != '\n' && *p != 0; p++) {
            if (!pass_spaces && (*p == ' ' || *p == '\t')) {
                ++nspaces;
            } else {
                pass_spaces = true;
                fputc(*p, stderr);
            }
        }

        fprintf(stderr, "\n");

        for (unsigned i = 1; i + nspaces < loc->column; ++i)
            fputc(' ', stderr);

        fprintf(stderr, CONSOLE_COLOR_GREEN "^\n" CONSOLE_COLOR_DEFAULT);
    }
}
