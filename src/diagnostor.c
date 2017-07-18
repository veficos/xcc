

#include "config.h"
#include "color.h"
#include "token.h"
#include "option.h"
#include "diagnostor.h"


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


#ifndef MIN_LINE_LIMIT
#define MIN_LINE_LIMIT  12
#endif


diagnostor_t __diagnostor__ = {
    0,
    0,
};

diagnostor_t* diagnostor = &__diagnostor__;


static void __write_linenote__(const unsigned char *linenote, size_t outputed, size_t width);
static void __write_linenote_caution__(diagnostor_level_t level, linenote_t linenote,
                                       size_t start, size_t length, size_t width);


void warningf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf(diagnostor, DIAGNOSTOR_LEVEL_WARNING, fmt, ap);
    va_end(ap);
}


void errorf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf(diagnostor, DIAGNOSTOR_LEVEL_ERROR, fmt, ap);
    va_end(ap);
}


void warningf_with_location(const char *fn, size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_location(diagnostor, DIAGNOSTOR_LEVEL_WARNING, fn, line, column, fmt, ap);
    va_end(ap);
}


void errorf_with_location(const char *fn, size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_location(diagnostor, DIAGNOSTOR_LEVEL_ERROR, fn, line, column, fmt, ap);
    va_end(ap);
}


void warningf_with_linenote(const char *fn, size_t line, size_t column, linenote_t linenote,
                            const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notef_with_linenote(diagnostor, DIAGNOSTOR_LEVEL_WARNING, fn, line, column, linenote, fmt, ap);
    va_end(ap);
}


void errorf_with_linenote(const char *fn, size_t line, size_t column, linenote_t linenote,
                          const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notef_with_linenote(diagnostor, DIAGNOSTOR_LEVEL_ERROR, fn, line, column, linenote, fmt, ap);
    va_end(ap);
}


void warningf_with_linenote_caution(const char *fn, size_t line, size_t column, linenote_t linenote,
                                    linenote_caution_t *linenote_caution, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_linenote_caution(diagnostor, DIAGNOSTOR_LEVEL_WARNING, fn,
                                            line, column, linenote, linenote_caution, fmt, ap);
    va_end(ap);
}


void errorf_with_linenote_caution(const char *fn, size_t line, size_t column, linenote_t linenote,
                                  linenote_caution_t *linenote_caution, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_linenote_caution(diagnostor, DIAGNOSTOR_LEVEL_ERROR, fn,
                                            line, column, linenote, linenote_caution, fmt, ap);
    va_end(ap);
}


void warningf_with_linenote_position(const char *fn, size_t line, size_t column, linenote_t linenote,
                                     size_t start, size_t length, const char *fmt, ...)
{
    va_list ap;
    linenote_caution_t lc;
    va_start(ap, fmt);
    lc.start = start;
    lc.length = length;

    diagnostor_notevf_with_linenote_caution(diagnostor,
                                            DIAGNOSTOR_LEVEL_WARNING,
                                            fn,
                                            line,
                                            column,
                                            linenote,
                                            &lc,
                                            fmt,
                                            ap);
    va_end(ap);
}


void errorf_with_linenote_position(const char *fn,
                                   size_t line,
                                   size_t column,
                                   linenote_t linenote,
                                   size_t start,
                                   size_t length,
                                   const char *fmt, ...)
{
    va_list ap;
    linenote_caution_t lc;
    va_start(ap, fmt);
    lc.start = start;
    lc.length = length;
    diagnostor_notevf_with_linenote_caution(diagnostor,
                                            DIAGNOSTOR_LEVEL_ERROR,
                                            fn,
                                            line,
                                            column,
                                            linenote,
                                            &lc,
                                            fmt,
                                            ap);
    va_end(ap);
}


void warningf_with_token(token_t *token, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diagnostor_notevf_with_linenote_caution(diagnostor,
                                            DIAGNOSTOR_LEVEL_WARNING,
                                            token->location.filename,
                                            token->location.line,
                                            token->location.column,
                                            token->location.linenote,
                                            &token->location.linenote_caution,
                                            fmt,
                                            ap);

    va_end(ap);
}


void errorf_with_token(token_t *token, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    diagnostor_notevf_with_linenote_caution(diagnostor,
                                            DIAGNOSTOR_LEVEL_ERROR,
                                            token->location.filename,
                                            token->location.line,
                                            token->location.column,
                                            token->location.linenote,
                                            &token->location.linenote_caution,
                                            fmt,
                                            ap);

    va_end(ap);
}


void panicf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_panicvf(diagnostor, fmt, ap);
    va_end(ap);
}


void panicf_with_location(const char *fn, size_t line,
                          size_t column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_panicvf_with_location(diagnostor, fn, line, column, fmt, ap);
    va_end(ap);
}


diagnostor_t* diagnostor_create(void)
{
    diagnostor_t *diag = pmalloc(sizeof(diagnostor_t));
    diag->nerrors = 0;
    diag->nwarnings = 0;
    return diag;
}


void diagnostor_destroy(diagnostor_t *diag)
{
    assert(diag != NULL);
    pfree(diag);
}


void diagnostor_notef(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf(diag, level, fmt, ap);
    va_end(ap);
}


void diagnostor_notevf(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, va_list args)
{
    switch (level) {
    case DIAGNOSTOR_LEVEL_NORMAL:
        break;
    case DIAGNOSTOR_LEVEL_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_LEVEL_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_LEVEL_ERROR:
        printf(BRUSH_BOLD_RED("error: "));
        diag->nerrors++;
        break;
    default:
        assert(false);
        break;
    }

    vprintf(fmt, args);
    printf("\n");
}


void diagnostor_notef_with_location(diagnostor_t *diag, diagnostor_level_t level,
                                   const char *fn, size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_location(diag, level, fn, line, column, fmt, ap);
    va_end(ap);
}


void diagnostor_notevf_with_location(diagnostor_t *diag, diagnostor_level_t level,
                                    const char *fn, size_t line, size_t column, const char *fmt, va_list args)
{
    printf("%s:%lu:%lu: ", fn, line, column);

    switch (level) {
    case DIAGNOSTOR_LEVEL_NORMAL:
        break;
    case DIAGNOSTOR_LEVEL_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_LEVEL_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_LEVEL_ERROR:
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

    vprintf(fmt, args);
    printf("\n");
}



void diagnostor_notef_with_linenote(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                    size_t line, size_t column, linenote_t linenote, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    diagnostor_notevf_with_linenote(diag, level, fn, line, column, linenote, fmt, ap);
    va_end(ap);
}


void diagnostor_notevf_with_linenote(diagnostor_t *diag, diagnostor_level_t level, const char *fn,
                                     size_t line, size_t column, linenote_t linenote, const char *fmt, va_list args)
{
    printf("%s:%lu:%lu: ", fn, line, column);

    switch (level) {
    case DIAGNOSTOR_LEVEL_NORMAL:
        break;
    case DIAGNOSTOR_LEVEL_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_LEVEL_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_LEVEL_ERROR:
        printf(BRUSH_BOLD_RED("error: "));
        diag->nerrors++;
        break;
    default:
        assert(false);
        break;
    }

    vprintf(fmt, args);
    printf("\n");

    diagnostor_note_linenote(diag, linenote);

    if (diag->nerrors >= option->ferror_limit) {
        diagnostor_report(diag);
        exit(-1);
    }
}


void diagnostor_notef_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level,
                                           const char *fn, size_t line, size_t column, linenote_t linenote,
                                           linenote_caution_t *linenote_caution, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    diagnostor_notevf_with_linenote_caution(diag, level, fn, line, column, linenote, linenote_caution, fmt, ap);
    va_end(ap);
}


void diagnostor_notevf_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level,
                                            const char *fn, size_t line, size_t column, linenote_t linenote,
                                            linenote_caution_t *linenote_caution, const char *fmt, va_list args)
{
    printf("%s:%lu:%lu: ", fn, line, column);

    switch (level) {
    case DIAGNOSTOR_LEVEL_NORMAL:
        break;
    case DIAGNOSTOR_LEVEL_NOTE:
        printf(BRUSH_BOLD_CYAN("note: "));
        break;
    case DIAGNOSTOR_LEVEL_WARNING:
        printf(BRUSH_BOLD_PURPLE("warning: "));
        diag->nwarnings++;
        break;
    case DIAGNOSTOR_LEVEL_ERROR:
        printf(BRUSH_BOLD_RED("error: "));
        diag->nerrors++;
        break;
    default:
        assert(false);
        break;
    }

    vprintf(fmt, args);
    printf("\n");

    diagnostor_note_linenote_caution(diag, level, linenote, linenote_caution);

    if (diag->nerrors >= option->ferror_limit) {
        diagnostor_report(diag);
        exit(-1);
    }
}


void diagnostor_note_linenote(diagnostor_t *diag, linenote_t linenote)
{
    int width;
    size_t outputed = 0;

    width = get_console_width();
    if (width < MIN_LINE_LIMIT) {
        return;
    }

    outputed += printf("   ");

    __write_linenote__(linenote, outputed, width);
}


void diagnostor_note_linenote_caution(diagnostor_t *diag, diagnostor_level_t level,
                                      linenote_t linenote, linenote_caution_t *linenote_caution)
{
    int width;
    int zoom, zoom_limit;
    size_t start = linenote_caution->start;
    size_t length = linenote_caution->length;
    size_t outputed = 0;

    width = get_console_width();
    if (width < MIN_LINE_LIMIT) {
        return;
    }

    zoom = width - (int)start;
    zoom_limit = (int)length;
    if (zoom <= zoom_limit) {
        outputed += printf("   ...");
        linenote = linenote + start - 1;
        start = outputed + 1;
    } else {
        outputed += printf("   ");
        start += outputed;
    }

    __write_linenote__(linenote, outputed, width);

    __write_linenote_caution__(level, linenote, start, length, width);
}


void diagnostor_panicf(diagnostor_t *diag, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    diagnostor_panicvf(diag, fmt, ap);
    va_end(ap);

}


void diagnostor_panicvf(diagnostor_t *diag, const char *fmt, va_list args)
{
    printf(BRUSH_BOLD_RED("fatal error: "));
    vprintf(fmt, args);
    printf("\n");
    diagnostor_report(diag);
    printf("compilation terminated.");
    exit(-1);
}


void diagnostor_panicf_with_location(diagnostor_t *diag, const char *fn,
                                    size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    diagnostor_panicvf_with_location(diag, fn, line, column, fmt, ap);
    va_end(ap);

}


void diagnostor_panicvf_with_location(diagnostor_t *diag, const char *fn,
                                      size_t line, size_t column, const char *fmt, va_list args)
{
    printf("%s:%lu:%lu: ", fn, line, column);
    printf(BRUSH_BOLD_RED("fatal error: "));
    vprintf(fmt, args);
    printf("\n");
    diagnostor_report(diag);
    printf("compilation terminated.");
    exit(-1);
}


void diagnostor_report(diagnostor_t *diag)
{
    if (diag->nwarnings != 0 && diag->nerrors != 0) {
        printf("%d warning and %d error generated.\n", diag->nwarnings, diag->nerrors);
    } else if (diag->nwarnings != 0) {
        printf("%d warning generated.\n", diag->nwarnings);
    } else if (diag->nerrors != 0) {
        printf("%d error generated.\n", diag->nerrors);
    }

    if (diag->nerrors != 0) {
        exit(-1);
    }
}


static void __write_linenote__(const unsigned char *linenote, size_t outputed, size_t width)
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
void __write_linenote_caution__(diagnostor_level_t level,
                                linenote_t linenote,
                                size_t start,
                                size_t length,
                                size_t width)
{
    size_t i, j;
    const char *tilde;
    const char *caret;

    for (i = 1, j = 1; i < start; i++, j++) {
        putchar(' ');
    }

    if (level == DIAGNOSTOR_LEVEL_WARNING) {
        caret = BRUSH_BOLD_PURPLE("^");
        tilde = BRUSH_BOLD_PURPLE("~");
    } else if (level == DIAGNOSTOR_LEVEL_ERROR) {
        caret = BRUSH_BOLD_RED("^");
        tilde = BRUSH_BOLD_RED("~");
    } else if (level == DIAGNOSTOR_LEVEL_NOTE) {
        caret = BRUSH_BOLD_CYAN("^");
        tilde = BRUSH_BOLD_CYAN("~");
    } else {
        assert(false);
    }

    printf(caret);
    for (i = 1; i < length && j < width; i++, j++) {
        printf(tilde);
    }
    printf("\n");
}
