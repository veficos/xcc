

#ifndef __NUMBER__H__
#define __NUMBER__H__


#include "config.h"
#include "cstring.h"


typedef enum number_property_e {
    NUMBER_CATEGORY = 0x000F,
    NUMBER_INVALID  = 0x0000,
    NUMBER_INTEGER  = 0x0001,
    NUMBER_FLOATING = 0x0002,

    NUMBER_WIDTH    = 0x00F0,
    NUMBER_SMALL    = 0x0010,           /* int, float, shrot _Fract/Accum */
    NUMBER_MEDIUM   = 0x0020,           /* long, double, long _Fract/_Accum. */
    NUMBER_LARGE    = 0x0040,           /* long long, long double, long long _Fract/Accum. */

    NUMBER_WIDTH_MD = 0xF0000,	        /* machine defined. */
    NUMBER_MD_W     = 0x10000,
    NUMBER_MD_Q     = 0x20000,

    NUMBER_RADIX    = 0x0F00,
    NUMBER_DECIMAL  = 0x0100,
    NUMBER_HEX      = 0x0200,
    NUMBER_OCTAL    = 0x0400,
    NUMBER_BINARY   = 0x0800,

    NUMBER_UNSIGNED     = 0x1000,       /* Properties. */
    NUMBER_IMAGINARY    = 0x2000,
    NUMBER_DFLOAT       = 0x4000,
    NUMBER_DEFAULT      = 0x8000,

    NUMBER_FRACT    = 0x100000,         /* Fract types. */
    NUMBER_ACCUM    = 0x200000,         /* Accum types. */

    NUMBER_USERDEF  = 0x1000000,        /* C++0x user-defined literal. */
} number_property_t;


typedef struct number_s {
    number_property_t property;
    int radix;
    union {
        long double ld;
        long long ll;
        unsigned long long ul;
    };
} number_t;

typedef struct diag_s*      diag_t;
typedef struct option_s*    option_t;
typedef struct token_s*     token_t;

bool parse_number(diag_t diag, option_t option, token_t tok);

#endif