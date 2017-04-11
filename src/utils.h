

#ifndef __UTILS__H__
#define __UTILS__H__


#include "config.h"


static inline
int TODIGIT(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }
    assert(false);
    return ch;
}


static inline
int ISOCT(int ch)
{
    return '0' <= ch && ch <= '7';
}


static inline
int ISHEX(int ch)
{
    return isxdigit(ch);
}


static inline
int ISALNUM(int ch)
{
    return isalnum(ch);
}


static inline
int ISALPHA(int ch)
{
    return isalpha(ch);
}


static inline
int ISSPACE(int ch)
{
    return isspace(ch);
}


static inline
int ISDIGIT(int ch)
{
    return isdigit(ch);
}


static inline
int ISIDNUM(int ch)
{
    return isalnum(ch) || ch == '_';
}


#endif
