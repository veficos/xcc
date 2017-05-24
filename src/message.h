

#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include "config.h"
#include "color.h"


#define warning_message(fn, line, column, fmt) \
     "%s:%u:%u " BRUSH_PURPLE("warning: ") fmt "\n", fn, line, column


#define warn_backslash_newline_space(fn, line, column) \
    warning_message(fn, line, column, "backslash and newline separated by space")


#define warn_no_newline_eof \
    "no newline at end of file"

#define warn_cxx98_compat_no_newline_eof \
    "C++98 requires newline at end of file"

#endif
