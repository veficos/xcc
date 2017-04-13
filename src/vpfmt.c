

#include "config.h"
#include "token.h"
#include "vpfmt.h"


#ifndef MAX_COLUMN_HEAD
#define MAX_COLUMN_HEAD  64
#endif


static inline void __output_with_loc__(FILE *fp, source_location_t loc);


void pfmt(FILE *fp, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vpfmt(fp, fmt, ap);

    va_end(ap);
}


void vpfmt(FILE *fp, const char *fmt, va_list ap)
{
    source_location_t loc = NULL;
    token_t tok = NULL;

    for (; *fmt; fmt++) {
        if (*fmt == '%') {
            switch (*++fmt) {
            case 'c':
                fprintf(fp, "%c", (char) va_arg(ap, int));
                break;
            case 'i':
            case 'd':
                fprintf(fp, "%d", va_arg(ap, int));
                break;
            case 'u':
                fprintf(fp, "%u", va_arg(ap, unsigned int));
                break;
            case 'x':
                fprintf(fp, "%x", va_arg(ap, int));
                break;
            case 'X':
                fprintf(fp, "%X", va_arg(ap, int));
                break;
            case 'o':
                fprintf(fp, "%o", va_arg(ap, int));
                break;
            case 's':
                fputs(va_arg(ap, char *), fp);
                break;
            case 'p':
                fprintf(fp, "%p", va_arg(ap, void *));
                break;
            case 'f':
                fprintf(fp, "%f", va_arg(ap, double));
                break;
            case 'l':
                if (fmt[1] == 'd') {
                    fmt++;
                    fprintf(fp, "%ld", va_arg(ap, long));
                    break;
                } else if (fmt[1] == 'u') {
                    fmt++;
                    fprintf(fp, "%lu", va_arg(ap, unsigned long));
                    break;
                } else if (fmt[1] == 'l' && fmt[2] == 'd') {
                    fmt += 2;
                    fprintf(fp, "%lld", va_arg(ap, long long));
                    break;
                } else if (fmt[1] == 'l' && fmt[2] == 'u') {
                    fmt += 2;
                    fprintf(fp, "%llu", va_arg(ap, unsigned long long));
                    break;
                } 
                putc(*fmt, fp);
                break;
            case 'L':
                if (fmt[1] == 'f') {
                    fmt++;
                    fprintf(fp, "%Lf", va_arg(ap, long double));
                } else {
                    putc(*fmt, fp);
                }
                break;
            case 'S':
                loc = va_arg(ap, source_location_t);
                fprintf(fp, "%s:%u:%u", loc->fn, loc->line, loc->column);
                break;
            case 'T':
                tok = va_arg(ap, token_t);
                fprintf(fp, "%s:%u:%u", tok->loc->fn, tok->loc->line, tok->loc->column);
                break;
            default:
                putc(*fmt, fp);
                break;
            }
        } else {
            putc(*fmt, fp);
        }
    }
}
