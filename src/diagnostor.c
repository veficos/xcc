

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


void diagnostor_note(diagnostor_t *diag, diagnostor_level_t level, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

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

    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}


void diagnostor_note_with_location(diagnostor_t *diag, diagnostor_level_t level,
                                   const char *fn, size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

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

    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}


void diagnostor_note_with_linenote_caution(diagnostor_t *diag, diagnostor_level_t level,
                                           const char *fn, size_t line, size_t column, linenote_t linenote,
                                           linenote_caution_t *linenote_caution, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

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

    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);

    diagnostor_note_linenote(diag, level, linenote, linenote_caution);

    if (diag->nerrors >= option->ferror_limit) {
        diagnostor_report(diag);
        exit(-1);
    }
}


void diagnostor_note_linenote(diagnostor_t *diag, diagnostor_level_t level,
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


void diagnostor_panic(diagnostor_t *diag, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf(BRUSH_BOLD_RED("fatal error: "));
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    diagnostor_report(diag);
    exit(-1);
}


void diagnostor_panic_with_location(diagnostor_t *diag, const char *fn,
                                    size_t line, size_t column, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("%s:%lu:%lu: ", fn, line, column);
    printf(BRUSH_BOLD_RED("fatal error: "));
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    diagnostor_report(diag);
    exit(-1);
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


static void __write_linenote_caution__(diagnostor_level_t level, linenote_t linenote,
                                       size_t start, size_t length, size_t width)
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
