

#ifndef __ENCODING__H__
#define __ENCODING__H__


#include "config.h"
#include "cstring.h"


typedef enum encoding_type_e {
    ENCODING_NONE,
    ENCODING_CHAR16,
    ENCODING_CHAR32,
    ENCODING_UTF8,
    ENCODING_WCHAR,
} encoding_type_t;

cstring_t cstring_append_utf8(cstring_t cs, uint32_t rune);
cstring_t cstring_cast_to_utf16(cstring_t cs);
cstring_t cstring_cast_to_utf32(cstring_t cs);

size_t utf8_rune_size(int ch);

#endif
