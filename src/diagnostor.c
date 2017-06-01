

#include "config.h"
#include "token.h"
#include "color.h"
#include "diagnostor.h"
#include "option.h"

#ifndef MAX_COLUMN_HEAD
#define MAX_COLUMN_HEAD  64
#endif


static void __write_one_line__(FILE *fp, const char *p);


#if defined(UNIX)
#   include <sys/ioctl.h>
#   include <unistd.h>

#elif defined(WINDOWS)
#   include <Windows.h>
#else

#endif


int get_console_width() {
#if defined(WINDOWS)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
	return csbi.srWindow.Right - csbi.srWindow.Left;
#elif defined(UNIX)
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
    return 80;
}


int get_console_height() {
#if defined(WINDOWS)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
	return csbi.srWindow.Bottom - csbi.srWindow.Top;
#elif defined(UNIX)
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
    return 25;
}


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


#define MIN_LINE_LIMIT  12

diagnostor_t* diagnostor_create(void)
{
    diagnostor_t *diagnostor = pmalloc(sizeof(diagnostor_t));
    diagnostor->nerrors = 0;
    diagnostor->nwarnings = 0;
    return diagnostor;
}


void diagnostor_destroy(diagnostor_t *diagnostor)
{
    assert(diagnostor != NULL);
    pfree(diagnostor);
}


void diagnostor_note(diagnostor_t *diag, diagnostor_msgtype_t msgtype, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    switch (msgtype) {
    case DIAGNOSTOR_MSGTYPE_NORMAL:
        break;
    case DIAGNOSTOR_MSGTYPE_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_MSGTYPE_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_MSGTYPE_ERROR:
        printf(BRUSH_BOLD_RED("error: "));
        diag->nerrors++;
        break;
    default:
        assert(false);
        break;
    }

    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}


void diagnostor_note_with_line(diagnostor_t *diag, diagnostor_msgtype_t msgtype,
                               const char *fn, size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    printf("%s:%lu:%lu: ", fn, line, column);

    switch (msgtype) {
    case DIAGNOSTOR_MSGTYPE_NORMAL:
        break;
    case DIAGNOSTOR_MSGTYPE_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_MSGTYPE_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_MSGTYPE_ERROR:
        printf(BRUSH_BOLD_RED("error: "));
        diag->nerrors++;
        if (diag->nerrors >= option->ferror_limit) {
            diagnostor_report(diag);
            exit(-1);
        }
        break;
    default:
        assert(false);
        break;
    }

    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

static void __write_linenote__(const unsigned char *linenote, size_t outputed, size_t width);
static void __write_linenote_caution__(diagnostor_msgtype_t msgtype,
                                       linenote_t linenote,
                                       linenote_caution_t *linenote_caution,
                                       size_t width_limit);


void diagnostor_note_linenote(diagnostor_t *diag, diagnostor_msgtype_t msgtype, linenote_t linenote,
                              linenote_caution_t *linenote_caution)
{
    int width;
    int zoom, zoom_limit;
    size_t outputed = 0;

    width = get_console_width();
    if (width < MIN_LINE_LIMIT) {
        return;
    }

    zoom = width - (int)linenote_caution->start;
    zoom_limit = (int)linenote_caution->length;
    if (zoom <= zoom_limit) {
        outputed += printf("   ...");
        linenote = linenote + (linenote_caution->start - 1);
        linenote_caution->start = outputed + 1;
    } else {
        outputed += printf("   ");
        linenote_caution->start += outputed;
    }

    __write_linenote__(linenote, outputed, width);

    __write_linenote_caution__(msgtype, linenote, linenote_caution, width);
}


void diagnostor_report(diagnostor_t *diag)
{
    if (diag->nwarnings != 0 && diag->nerrors != 0) {
        printf("%d warning and %d error generated.\n", diag->nwarnings, diag->nerrors);
    }

    if (diag->nwarnings != 0) {
        printf("%d warning generated.\n", diag->nwarnings);
    }

    if (diag->nerrors != 0) {
        printf("%d error generated.\n", diag->nerrors);
    }
}


static
void __write_linenote__(const unsigned char *linenote, size_t outputed, size_t width)
{
    size_t i;

    width -= 3;
    for (i = outputed; *linenote != '\r' &&
            *linenote != '\n' &&
            *linenote &&
            (width != 0 && i < width);
         linenote++, i++) {
        putchar(*linenote);
    }

    if (i == width && *linenote != '\r' && *linenote != '\n' && *linenote) {
        printf("...\n");
    } else {
        printf("\n");
    }
}


static
void __write_linenote_caution__(diagnostor_msgtype_t msgtype,
                                linenote_t linenote,
                                linenote_caution_t *linenote_caution,
                                size_t width_limit)
{
    size_t i, j;
    const char *tilde;
    const char *caret;

    for (i = 1, j = 1; i < linenote_caution->start; i++, j++) {
        putchar(' ');
    }

    if (msgtype == DIAGNOSTOR_MSGTYPE_WARNING) {
        caret = BRUSH_BOLD_PURPLE("^");
        tilde = BRUSH_BOLD_PURPLE("~");
    } else if (msgtype == DIAGNOSTOR_MSGTYPE_ERROR) {
        caret = BRUSH_BOLD_RED("^");
        tilde = BRUSH_BOLD_RED("~");
    } else {
        assert(false);
    }

    printf(caret);
    for (i = 1; i < linenote_caution->length && j < width_limit; i++, j++) {
        printf(tilde);
    }
    printf("\n");
}
