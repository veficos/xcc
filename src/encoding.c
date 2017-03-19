

#include "config.h"
#include "encoding.h"
#include "diag.h"


static inline int __count_leading_ones__(char ch);
static inline bool __read_rune__(uint32_t *rune, size_t *rune_size, char *s, int n);
static inline cstring_t __write8__(cstring_t cs, uint8_t u);
static inline cstring_t __write16__(cstring_t cs, uint16_t u);
static inline cstring_t __write32__(cstring_t cs, uint32_t u);


cstring_t cstring_append_utf8(cstring_t cs, uint32_t rune)
{
    if (rune < 0x80) {
        return __write8__(cs, rune);
    }

    if (rune < 0x800) {
        cs = __write8__(cs, 0xC0 | (rune >> 6));
        cs = __write8__(cs, 0x80 | (rune & 0x3F));
        return cs;
    }

    if (rune < 0x10000) {
        cs = __write8__(cs, 0xE0 | (rune >> 12));
        cs = __write8__(cs, 0x80 | ((rune >> 6) & 0x3F));
        cs = __write8__(cs, 0x80 | (rune & 0x3F));
        return cs;
    }

    if (rune < 0x200000) {
        cs = __write8__(cs, 0xF0 | (rune >> 18));
        cs = __write8__(cs, 0x80 | ((rune >> 12) & 0x3F));
        cs = __write8__(cs, 0x80 | ((rune >> 6) & 0x3F));
        cs = __write8__(cs, 0x80 | (rune & 0x3F));
        return cs;
    }

    return NULL;
}


cstring_t cstring_cast_to_utf16(cstring_t cs)
{
    cstring_t to;
    uint32_t rune;
    size_t rune_size;
    size_t i, length;

    length = cstring_length(cs);
    to = cstring_create_n(NULL, length * sizeof(uint16_t));
    if (!to) {
        return NULL;
    }

    for (i = 0; i < length;) {
        if (!__read_rune__(&rune, &rune_size, &cs[i], length - i)) {
            cstring_destroy(to);
            return NULL;
        }

        if (rune < 0x10000) {
            to = __write16__(to, rune);
        } else {
            to = __write16__(to, (rune >> 10) + 0xD7C0);
            to = __write16__(to, (rune & 0x3FF) + 0xDC00);
        }

        i += rune_size;
    }

    return to;
}


cstring_t cstring_cast_to_utf32(cstring_t cs)
{
    cstring_t to;
    uint32_t rune;
    size_t rune_size;
    size_t i, length;

    length = cstring_length(cs);
    to = cstring_create_n(NULL, length * sizeof(uint32_t));
    if (!to) {
        return NULL;
    }

    for (i = 0; i < length; ) {
        if (!__read_rune__(&rune, &rune_size, cs, length - i)) {
            return NULL;
        }

        to = __write32__(to, rune);

        i += rune_size;
    }

    return to;
}


static inline
int __count_leading_ones__(char ch)
{
    for (int i = 7; i >= 0; i--) {
        if ((ch & (1 << i)) == 0) {
            return 7 - i;
        }
    }
    return 8;
}


static inline
bool __read_rune__(uint32_t *rune, size_t *rune_size, char *s, int n) {

    int len = __count_leading_ones__(s[0]);

    if (len == 0) {
        *rune = s[0];
        *rune_size = 1;
        return true;
    }

    if (len > n) {
        return false;
    }

    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            return false;
        }
    }

    switch (len) {
    case 2:
        *rune = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        *rune_size = 2;
        return true;
    case 3:
        *rune = ((s[0] & 0xF) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        *rune_size = 3;
        return true;
    case 4:
        *rune = ((s[0] & 0x7) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        *rune_size = 4;
        return true;
    }

    return false;
}


static inline 
cstring_t __write8__(cstring_t cs, uint8_t u)
{
    return cstring_cat_n(cs, &u, sizeof(uint8_t));
}


static inline
cstring_t __write16__(cstring_t cs, uint16_t u)
{
    cs = cstring_push_ch(cs, u & 0xFF);
    cs = cstring_push_ch(cs, u >> 8);
    return cs;
}


static inline
cstring_t __write32__(cstring_t cs, uint32_t u)
{
    cs = __write16__(cs, u & 0xFFFF);
    cs = __write16__(cs, u >> 16);
    return cs;
}


size_t utf8_rune_size(int ch)
{
    size_t step = __count_leading_ones__(ch);
    return step == 0 ? 1 : step;
}
