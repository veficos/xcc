

#include "config.h"
#include "token.h"
#include "diag.h"
#include "encoding.h"


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


static inline void __diag_output__(const char *fmt, va_list ap);
static inline void __diag_output_location__(source_location_t loc);
static inline void __diag_errorvf_location__(diag_t diag, source_location_t loc, const char *fmt, va_list ap);
static inline void __diag_warningvf_location__(diag_t diag, source_location_t loc, const char *fmt, va_list ap);


diag_t diag_create(void)
{
    diag_t diag = pmalloc(sizeof(struct diag_s));
    if (!diag) {
        return NULL;
    }

    diag->nwarnings = 0;
    diag->nerrors = 0;

    return diag;
}


void diag_destroy(diag_t diag)
{
    assert(diag);

    pfree(diag);
}


void diag_panic(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    __diag_output__(fmt, ap);

    va_end(ap);

    exit(-1);
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
    __diag_output__(fmt, ap);
    diag->nerrors++;
}


void diag_errorf(diag_t diag, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diag_errorvf(diag, fmt, ap);

    va_end(ap);
}


void diag_errorvf_location(diag_t diag, source_location_t loc, const char *fmt, va_list ap)
{
    __diag_errorvf_location__(diag, loc, fmt, ap);
}


void diag_errorf_location(diag_t diag, source_location_t loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    __diag_errorvf_location__(diag, loc, fmt, ap);

    va_end(ap);
}


static inline
void __diag_errorvf_location__(diag_t diag, source_location_t loc, const char *fmt, va_list ap)
{
    fprintf(stderr, 
            "%s:%d:%d: " CONSOLE_COLOR_RED "error: " CONSOLE_COLOR_DEFAULT, 
            loc->filename, 
            loc->line, 
            loc->column);

    __diag_output__(fmt, ap);

    __diag_output_location__(loc);

    diag->nerrors++;
}



void diag_warningf_location(diag_t diag, source_location_t loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    __diag_warningvf_location__(diag, loc, fmt, ap);

    va_end(ap);
}


void diag_warningvf_location(diag_t diag, source_location_t loc, const char *fmt, va_list ap)
{
    __diag_warningvf_location__(diag, loc, fmt, ap);
}


static inline 
void __diag_warningvf_location__(diag_t diag, source_location_t loc, const char *fmt, va_list ap)
{
    fprintf(stderr,
            "%s:%d:%d: " CONSOLE_COLOR_MAGENTA "warning: " CONSOLE_COLOR_DEFAULT,
            loc->filename,
            loc->line,
            loc->column);

    __diag_output__(fmt, ap);

    __diag_output_location__(loc);

    diag->nwarnings++;
}


static inline 
void __diag_output__(const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}


static inline 
void __diag_output_location__(source_location_t loc)
{
    const char *p;
    const char *q;
    size_t step;

    if (loc->column >= MAX_COLUMN_HEAD) {
        p = loc->current_line;
        fprintf(stderr, "   ...");
        fprintf(stderr, p + loc->column);
        fprintf(stderr, CONSOLE_COLOR_GREEN "\n      ^\n" CONSOLE_COLOR_DEFAULT);

    } else {
        p = loc->current_line;
        q = p + (loc->column - 1);

        fprintf(stderr, "%s\n", loc->current_line);

        for (; p < q;) {
            step = utf8_rune_size(*p);
            p += step;
            if (step > 1) {
                fputc('  ', stderr);
            } else {
                fputc(' ', stderr);
            }
        }

        fprintf(stderr, CONSOLE_COLOR_GREEN "^\n" CONSOLE_COLOR_DEFAULT);
    }
}