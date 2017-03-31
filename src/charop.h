

#ifndef __CHAROP__H__
#define __CHAROP__H__


#include "config.h"


static inline
int chtodigit(int ch)
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
int chisoct(int ch)
{
    return '0' <= ch && ch <= '7';
}


static inline
int chishex(int ch)
{
    return isxdigit(ch);
}


static inline
int chisalnum(int ch)
{
    return isalnum(ch);
}


static inline
int chisalpha(int ch)
{
    return isalpha(ch);
}


static inline
int chisspace(int ch)
{
    return isspace(ch);
}


static inline
int chisdigit(int ch)
{
    return isdigit(ch);
}


static inline
int chisidnum(int ch)
{
    return isalnum(ch) || ch == '_';
}


#endif
