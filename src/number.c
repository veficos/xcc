

#include "config.h"
#include "number.h"
#include "option.h"
#include "diagnostor.h"
#include "utils.h"
#include "token.h"


#undef  ERRORF
#define ERRORF_WITH_TOKEN(diag, tok, fmt, ...) \
    diag_errorf_with_tok((diag), (tok), fmt, __VA_ARGS__)


#undef  WARNINGF
#define WARNINGF_WITH_TOKEN(diag, tok, fmt, ...) \
    diag_warningf_with_tok((diag), (tok), fmt, __VA_ARGS__)


#undef  CONFIG_DIGIT_SEPARATOR
#define CONFIG_DIGIT_SEPARATOR(option)  true


#undef  CONFIG_EXT_NUMERIC_LITERALS
#define CONFIG_EXT_NUMERIC_LITERALS(option) true


#undef  CONFIG_PEDANTIC
#define CONFIG_PEDANTIC(option) true


#undef  CONFIG_EXTENDED_NUMBERS
#define CONFIG_EXTENDED_NUMBERS(option) true

#undef  CONFIG_CPLUSPLUS
#define CONFIG_CPLUSPLUS(option) true

#undef  CONFIG_C99
#define CONFIG_C99(option) true


#undef  CONFIG_WTRADITIONAL
#define CONFIG_WTRADITIONAL(option) false


#undef  CONFIG_WARNLONGLONG
#define CONFIG_WARNLONGLONG(option) true


#undef  CONFIG_BINARY_CONSTANT
#define CONFIG_BINARY_CONSTANT(option) true


#undef  DIGIT_SEPARATOR
#define DIGIT_SEPARATOR(ch, option) \
    ((ch) == '\'' && CONFIG_DIGIT_SEPARATOR(option))


static number_property_t 
__interpret_float_suffix__(option_t option, const unsigned char *p, size_t len);
static number_property_t
__interpret_int_suffix__(option_t option, const unsigned char *p, size_t len);
static void __init_number__(number_t *number);
static bool __to_number__(diag_t diag, cstring_t cs, int radix, number_property_t property, token_t tok, number_t *number);


bool parse_number(diag_t diag, option_t option, token_t tok, number_t *number)
{
    unsigned int max_digit, radix;
    bool seen_digit;
    bool seen_digit_sep;
    number_property_t property;

    enum
    {
        NOT_FLOAT = 0,
        AFTER_POINT,
        AFTER_EXPON,
    } float_flag;

    const unsigned char *p = (unsigned char*)tok->cs;
    const unsigned char *q = (unsigned char*)tok->cs + cstring_length(tok->cs);
    cstring_t cs = cstring_new_n(NULL, 128);

    assert(tok->type == TOKEN_NUMBER);
     
    radix = 10;
    float_flag = NOT_FLOAT;
    seen_digit = false;
    seen_digit_sep = false;
    max_digit = 0;
    
    __init_number__(number);

    if (*p == '0') {
        radix = 8;
        p++;
        if (*p == 'x' || *p == 'X') {
            if (p[1] == '.' || ISHEX(p[1])) {
                radix = 16;
                p++;
            } else if (DIGIT_SEPARATOR(p[1], option)) {
                ERRORF_WITH_TOKEN(diag, tok, "digit separator after base indicator");
                goto syntax_error;
            }
            cs = cstring_concat_n(cs, "0x", 2);
        } else if (*p == 'b' || *p == 'B') {
            if (p[1] == '0' || p[1] == '1') {
                radix = 2;
                p++;
            } else if (DIGIT_SEPARATOR(p[1], option)) {
                ERRORF_WITH_TOKEN(diag, tok, "digit separator after base indicator");
                goto syntax_error;
            }
        }
    }

    for (;;) {
        unsigned int ch = *p++;

        cs = cstring_concat_ch(cs, (unsigned char)ch);
        if (ISDIGIT(ch) || (ISHEX(ch) && radix == 16)) {
            seen_digit_sep = false;
            seen_digit = true;
            ch = TODIGIT(ch);
            if (ch > max_digit) {
                max_digit = ch;
            }
        } else if (DIGIT_SEPARATOR(ch, option)) {
            if (seen_digit_sep) {
                ERRORF_WITH_TOKEN(diag, tok, "adjacent digit separators");
                goto syntax_error;
            }
            seen_digit_sep = true;
            cstring_pop_ch(cs);
        } else if (ch == '.') {
            if (seen_digit_sep) {
                ERRORF_WITH_TOKEN(diag, tok, "adjacent digit separators");
                goto syntax_error;
            }

            seen_digit_sep = false;

            if (float_flag == NOT_FLOAT) {
                float_flag = AFTER_POINT;
            } else {
                ERRORF_WITH_TOKEN(diag, tok, "too many decimal points in number");
                goto syntax_error;
            }
        } else if ((radix <= 10 && (ch == 'e' || ch == 'E')) || 
                   (radix == 16 && (ch == 'p' || ch == 'P'))) {
            if (seen_digit_sep || DIGIT_SEPARATOR(*p, option)) {
                ERRORF_WITH_TOKEN(diag, tok, "digit separator adjacent to exponent");
                goto syntax_error;
            }
            float_flag = AFTER_EXPON;
            break;
        } else {
            p--;
            cstring_pop_ch(cs);
            break;
        }
    }

    if (seen_digit_sep && float_flag != AFTER_EXPON) {
        ERRORF_WITH_TOKEN(diag, tok, "digit separator outside digit sequence");
        goto syntax_error;
    }

    if (radix != 16 && float_flag == NOT_FLOAT) {
        property = __interpret_float_suffix__(option, p, q - p);

        if ((property & NUMBER_FRACT) || (property & NUMBER_ACCUM)) {
            property |= NUMBER_FLOATING;

            /* We need to restore the radix to 10, if the radix is 8. */
            if (radix == 8) {
                radix = 10;
            }

            if (CONFIG_PEDANTIC(option)) {
                ERRORF_WITH_TOKEN(diag, tok, "fixed-point constants are a GCC extension");
            }

            goto syntax_ok;
        } else {
            property = NUMBER_INVALID;
        }
    }

    if (float_flag != NOT_FLOAT && radix == 8) {
        radix = 10;
    }

    if (max_digit >= radix) {
        if (radix == 2) {
            ERRORF_WITH_TOKEN(diag, tok, "invalid digit \"%c\" in binary constant", '0' + max_digit);
            goto syntax_error;
        } else {
            ERRORF_WITH_TOKEN(diag, tok, "invalid digit \"%c\" in octal constant", '0' + max_digit);
            goto syntax_error;
        }
    }

    if (float_flag != NOT_FLOAT) {
        if (radix == 2) {
            ERRORF_WITH_TOKEN(diag, tok, "invalid prefix \"0b\" for floating constant");
            goto syntax_error;
        }

        if (radix == 16 && !seen_digit) {
            ERRORF_WITH_TOKEN(diag, tok, "no digits in hexadecimal floating constant");
            goto syntax_error;
        }

        if (radix == 16 && CONFIG_PEDANTIC(option) && 
            !CONFIG_EXTENDED_NUMBERS(option)) {
            if (CONFIG_CPLUSPLUS(option)) {
                ERRORF_WITH_TOKEN(diag, tok, "use of C++1z hexadecimal floating constant");
            } else {
                ERRORF_WITH_TOKEN(diag, tok, "use of C99 hexadecimal floating constant");
            }
        }

        if (float_flag == AFTER_EXPON) {
            if (*p == '+' || *p == '-') {
                p++;
            }

            /* Exponent is decimal, even if string is a hex float.  */
            if (!ISDIGIT(*p)) {
                if (DIGIT_SEPARATOR(*p, option)) {
                    ERRORF_WITH_TOKEN(diag, tok, "digit separator adjacent to exponent");
                    goto syntax_error;
                } else {
                    ERRORF_WITH_TOKEN(diag, tok, "exponent has no digits");
                    goto syntax_error;
                }
            }

            do {
                seen_digit_sep = DIGIT_SEPARATOR(*p, option);
                p++;
            } while (ISDIGIT(*p) || DIGIT_SEPARATOR(*p, option));
        } else if (radix == 16) {
            ERRORF_WITH_TOKEN(diag, tok, "hexadecimal floating constants require an exponent");
            goto syntax_error;
        }

        if (seen_digit_sep) {
            ERRORF_WITH_TOKEN(diag, tok, "digit separator outside digit sequence");
            goto syntax_error;
        }

        property = __interpret_float_suffix__(option, p, q - p);
        if (property == NUMBER_INVALID) {
            ERRORF_WITH_TOKEN(diag, tok, "invalid suffix \"%.*s\" on floating constant", (intptr_t)(p-q), p);
            goto syntax_error;
        }

        /* Traditional C didn't accept any floating suffixes. */
        if (q != p && CONFIG_WTRADITIONAL(option)) {
            WARNINGF_WITH_TOKEN(diag, tok, "traditional C rejects the \"%.*s\" suffix", (intptr_t)(p - q), p);
        }

        /* 
         * A suffix for double is a GCC extension via decimal float support.
         * If the suffix also specifies an imaginary value we'll catch that
         * later.  
         */
        if ((property == NUMBER_MEDIUM) && CONFIG_PEDANTIC(option)) {
            ERRORF_WITH_TOKEN(diag, tok, "suffix for double constant is a GCC extension");
        }

        /* Radix must be 10 for decimal floats.  */
        if ((property & NUMBER_DFLOAT) && radix != 10) {
            ERRORF_WITH_TOKEN(diag, tok, 
                "invalid suffix \"%.*s\" with hexadecimal floating constant", 
                (intptr_t)(p - q), p);
            goto syntax_error;
        }

        if ((property & (NUMBER_FRACT | NUMBER_ACCUM)) && CONFIG_PEDANTIC(option)) {
            ERRORF_WITH_TOKEN(diag, tok, "fixed-point constants are a GCC extension");
        }

        if ((property & NUMBER_DFLOAT) && CONFIG_PEDANTIC(option)) {
            ERRORF_WITH_TOKEN(diag, tok, "decimal float constants are a GCC extension");
        }

        property |= NUMBER_FLOATING;
    } else {
        property = __interpret_int_suffix__(option, p, q - p);
        if (property == NUMBER_INVALID) {
            ERRORF_WITH_TOKEN(diag, tok, "invalid suffix \"%.*s\" on integer constant", (intptr_t)(p - q), p);
            goto syntax_error;
        }

        /* 
         * Traditional C only accepted the 'L' suffix.
         * Suppress warning about 'LL' with -Wno-long-long.
         */
        if (CONFIG_WTRADITIONAL(option))
        {
            int u_or_i = (property & (NUMBER_UNSIGNED | NUMBER_IMAGINARY));
            int large = (property & NUMBER_WIDTH) == NUMBER_LARGE && CONFIG_WARNLONGLONG(option);

            if (u_or_i || large) {
                WARNINGF_WITH_TOKEN(diag, tok, "traditional C rejects the \"%.*s\" suffix", (intptr_t)(p - q), p);
            }
        }

        if ((property & NUMBER_WIDTH) == NUMBER_LARGE && CONFIG_WARNLONGLONG(option)) {
            if (CONFIG_C99(option)) {
                WARNINGF_WITH_TOKEN(diag, tok, "use of C99 long long integer constant");
            } else {
                WARNINGF_WITH_TOKEN(diag, tok, "use of C++11 long long integer constant");
            }
        }

        property |= NUMBER_INTEGER;
    }

syntax_ok:
    if ((property & NUMBER_IMAGINARY) && CONFIG_PEDANTIC(option)) {
        ERRORF_WITH_TOKEN(diag, tok, "imaginary constants are a GCC extension");
    }
  
    if (radix == 2 && !CONFIG_BINARY_CONSTANT(option) && CONFIG_PEDANTIC(pfile)) {
        ERRORF_WITH_TOKEN(diag, tok, "binary constants are a C++14 feature or GCC extension");
    }

    if (radix == 10) {
        property |= NUMBER_DECIMAL;
    } else if (radix == 16) {
        property |= NUMBER_HEX;
    } else if (radix == 2) {
        property |= NUMBER_BINARY;
    } else if (radix == 8) {
        property |= NUMBER_OCTAL;
    } else {
        assert(false);
    }
    
    __to_number__(diag, cs, radix, property, tok, number);
    cstring_free(cs);
    return true;

syntax_error:
    cstring_free(cs);
    return false;
}
    

bool parse_char(diag_t diag, option_t option, token_t tok, number_t *number)
{
    cstring_t utf;
    uint32_t ch;

    assert(tok->type == TOKEN_CONSTANT_CHAR ||
           tok->type == TOKEN_CONSTANT_WCHAR ||
           tok->type == TOKEN_CONSTANT_CHAR16 ||
           tok->type == TOKEN_CONSTANT_CHAR32);
    
    __init_number__(number);

    if (cstring_length(tok->cs) <= 0) {
        return false;
    }

    switch(tok->type) {
    case TOKEN_CONSTANT_CHAR:
        ch = (uint32_t)tok->cs[0];
        if (cstring_length(tok->cs) > sizeof(uint8_t)) {
            WARNINGF_WITH_TOKEN(diag, tok, "multi-character character constant");
        }
        break;
    case TOKEN_CONSTANT_CHAR16:
        utf = cstring_cast_to_utf16(tok->cs);
        ch = *(uint16_t*)utf;
        if (cstring_length(utf) > sizeof(uint16_t)) {
            WARNINGF_WITH_TOKEN(diag, tok, "multi-character character constant");
        }
        break;
    case TOKEN_CONSTANT_CHAR32:
    case TOKEN_CONSTANT_WCHAR:
        utf = cstring_cast_to_utf32(tok->cs);
        ch = *(uint32_t*)utf;
        if (cstring_length(utf) > sizeof(uint32_t)) {
            WARNINGF_WITH_TOKEN(diag, tok, "multi-character character constant");
        }
        break;
    default:
        assert(false);
        return false;
    }

    number->radix = 10;
    number->property = NUMBER_INTEGER | NUMBER_MEDIUM | NUMBER_DECIMAL | NUMBER_UNSIGNED;
    number->ul = ch;
    return true;
}


static number_property_t
__interpret_float_suffix__(option_t option, const unsigned char *p, size_t len)
{
    size_t flags;
    size_t f, d, l, w, q, i;

    flags = 0;
    f = d = l = w = q = i = 0;

    /* 
     * Process decimal float suffixes, which are two letters starting
     * with d or D.  Order and case are significant.  
     */
    if (len == 2 && (*p == 'd' || *p == 'D'))
    {
        bool uppercase = (*p == 'D');
        switch (p[1]) {
        case 'f':
            return (!uppercase ? (NUMBER_DFLOAT | NUMBER_SMALL) : 0);
        case 'F':
            return (uppercase ? (NUMBER_DFLOAT | NUMBER_SMALL) : 0);
        case 'd':
            return (!uppercase ? (NUMBER_DFLOAT | NUMBER_MEDIUM) : 0);
        case 'D':
            return (uppercase ? (NUMBER_DFLOAT | NUMBER_MEDIUM) : 0);
        case 'l':
            return (!uppercase ? (NUMBER_DFLOAT | NUMBER_LARGE) : 0);
        case 'L':
            return (uppercase ? (NUMBER_DFLOAT | NUMBER_LARGE) : 0);
        default:
            /*
             * Additional two-character suffixes beginning 
             * with D are not for decimal float constants.  
             */
            break;
        }
    }

    if (CONFIG_EXT_NUMERIC_LITERALS(option)) {
        if (len != 0) {
            switch (p[len - 1]) {
            case 'k':
            case 'K':
                flags = NUMBER_ACCUM;
                break;
            case 'r':
            case 'R':
                flags = NUMBER_FRACT;
                break;
            default:
                break;
            }
        }

        /* Continue processing a fixed-point suffix.  The suffix is case
        insensitive except for ll or LL.  Order is significant.  */
        if (flags) {
            if (len == 1) {
                return flags;
            }

            len--;

            if (*p == 'u' || *p == 'U') {
                flags |= NUMBER_UNSIGNED;
                if (len == 1) {
                    return flags;
                }
                len--;
                p++;
            }

            switch (*p) {
            case 'h':
            case 'H':
                if (len == 1) {
                    return flags |= NUMBER_SMALL;
                }
                break;
            case 'l':
                if (len == 1) {
                    return flags |= NUMBER_MEDIUM;
                }
                if (len == 2 && p[1] == 'l') {
                    return flags |= NUMBER_LARGE;
                }
                break;
            case 'L':
                if (len == 1) {
                    return flags |= NUMBER_MEDIUM;
                }
                if (len == 2 && p[1] == 'L') {
                    return flags |= NUMBER_LARGE;
                }
                break;
            default:
                break;
            }
            /* Anything left at this point is invalid.  */
            return NUMBER_INVALID;
        }
    }

    while (len--) {
        switch (p[len]){
        case 'f':
        case 'F':
            f++;
            break;
        case 'd':
        case 'D':
            d++;
            break;
        case 'l':
        case 'L':
            l++;
            break;
        case 'w':
        case 'W':
            w++;
            break;
        case 'q':
        case 'Q':
            q++;
            break;
        case 'i':
        case 'I':
        case 'j':
        case 'J':
            i++;
            break;
        default:
            return NUMBER_INVALID;
        }
    }

    if (f + d + l + w + q > 1 || i > 1) {
        return NUMBER_INVALID;
    }

    if (i && !CONFIG_EXT_NUMERIC_LITERALS(option)) {
        return NUMBER_INVALID;
    }

    if ((w || q) && !CONFIG_EXT_NUMERIC_LITERALS(option)) {
        return NUMBER_INVALID;
    }

    return ((i ? NUMBER_IMAGINARY : 0) | 
            (f ? NUMBER_SMALL : d ? NUMBER_MEDIUM : l ? NUMBER_LARGE : w ? NUMBER_MD_W : q ? NUMBER_MD_Q : NUMBER_DEFAULT));
}


static number_property_t
__interpret_int_suffix__(option_t option, const unsigned char *p, size_t len)
{
    size_t u, l, i;

    u = l = i = 0;

    while (len--) {
        switch (p[len]) {
        case 'u':
        case 'U':
            u++;
            break;
        case 'i':
        case 'I':
        case 'j':
        case 'J':
            i++;
            break;
        case 'l':
        case 'L':
            l++;
            /* 
             * If there are two Ls, they must be adjacent and the same case.
             */
            if (l == 2 && p[len] != p[len + 1])
                return NUMBER_INVALID;
            break;
        default:
            return NUMBER_INVALID;
        }
    }

    if (l > 2 || u > 1 || i > 1) {
        return NUMBER_INVALID;
    }

    if (i && !CONFIG_EXT_NUMERIC_LITERALS(option)) {
        return NUMBER_INVALID;
    }

    return ((i ? NUMBER_IMAGINARY : 0) | (u ? NUMBER_UNSIGNED : 0) | ((l == 0) ? NUMBER_SMALL
        : (l == 1) ? NUMBER_MEDIUM : NUMBER_LARGE));
}


static
void __init_number__(number_t *number)
{
    number->property = NUMBER_INVALID;
    number->radix = 0;
    number->ul = 0;
}


static bool 
__to_number__(diag_t diag, cstring_t cs, int radix, number_property_t property, token_t tok, number_t *number)
{
    char *end;

    if (property & NUMBER_INTEGER) {
        number->ul = strtoull(cs, &end, radix);
    } else if (property & NUMBER_FLOATING) {
        number->ld = strtold(cs, &end);
    } else {
        assert(false);
        return false;
    }

    number->radix = radix;
    number->property = property;
    return true;
}


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef bool
#define bool unsigned char
enum {
    false,
    true,
};
#endif
#define BN_SIZE 2
#define STRING_MAX_SIZE     1024
#define CONFIG_BINARY_CONSTANT true

#define error printf
#define expect printf

static inline
int ISDIGIT(int ch)
{
    return isdigit(ch);
}

/* bn = (bn << shift) | or_val */
void bn_lshift(unsigned long *bn, int shift, int or_val)
{
    int i;
    unsigned int v;
    for(i = 0; i < BN_SIZE; i++) {
        v = bn[i];
        bn[i] = (v << shift) | or_val;
        or_val = v >> (32 - shift);
    }
}

void bn_zero(unsigned long *bn)
{
    int i;
    for(i = 0; i < BN_SIZE; i++) {
        bn[i] = 0;
    }
}


typedef enum {
    TOK_CLDOUBLE,
    TOK_CFLOAT,
    TOK_CDOUBLE,
    TOK_CLLONG,
    TOK_CULLONG,
    TOK_CUINT,
    TOK_CINT,
}token_type_t;

typedef union number_u {
    long double ld;
    double d;
    float f;
    int i;
    unsigned int ui;
    unsigned int ul; /* address (should be unsigned long on 64 bit cpu) */
    long long ll;
    unsigned long long ull;
} number_t;

int errno = 0;


token_type_t tok;
number_t tokc;

void parse_number(char *p,  char *token_buf)
{
    int b, t, shift, frac_bits, s, exp_val, ch;
    char *q;
    unsigned int bn[BN_SIZE];
    double d;

    /* number */
    q = token_buf;
    ch = *p++;
    t = ch;
    ch = *p++;
    *q++ = t;
    b = 10;
    if (t == '.') {
        goto float_frac_parse;
    } else if (t == '0') {
        if (ch == 'x' || ch == 'X') {
            q--;
            ch = *p++;
            b = 16;
        } else if (CONFIG_BINARY_CONSTANT && (ch == 'b' || ch == 'B')) {
            q--;
            ch = *p++;
            b = 2;
        }
    }
    /* parse all digits. cannot check octal numbers at this stage
       because of floating point constants */
    while (1) {
        if (ch >= 'a' && ch <= 'f')
            t = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            t = ch - 'A' + 10;
        else if (ISDIGIT(ch))
            t = ch - '0';
        else
            break;
        if (t >= b)
            break;
        if (q >= token_buf + STRING_MAX_SIZE) {
            num_too_long:
            error("number too long");
        }
        *q++ = ch;
        ch = *p++;
    }
    if (ch == '.' ||
        ((ch == 'e' || ch == 'E') && b == 10) ||
        ((ch == 'p' || ch == 'P') && (b == 16 || b == 2))) {
        if (b != 10) {
            /* NOTE: strtox should support that for hexa numbers, but
               non ISOC99 libcs do not support it, so we prefer to do
               it by hand */
            /* hexadecimal or binary floats */
            /* XXX: handle overflows */
            *q = '\0';
            if (b == 16)
                shift = 4;
            else
                shift = 2;
            bn_zero(bn);
            q = token_buf;
            while (1) {
                t = *q++;
                if (t == '\0') {
                    break;
                } else if (t >= 'a') {
                    t = t - 'a' + 10;
                } else if (t >= 'A') {
                    t = t - 'A' + 10;
                } else {
                    t = t - '0';
                }
                bn_lshift(bn, shift, t);
            }
            frac_bits = 0;
            if (ch == '.') {
                ch = *p++;
                while (1) {
                    t = ch;
                    if (t >= 'a' && t <= 'f') {
                        t = t - 'a' + 10;
                    } else if (t >= 'A' && t <= 'F') {
                        t = t - 'A' + 10;
                    } else if (t >= '0' && t <= '9') {
                        t = t - '0';
                    } else {
                        break;
                    }
                    if (t >= b)
                        error("invalid digit");
                    bn_lshift(bn, shift, t);
                    frac_bits += shift;
                    ch = *p++;
                }
            }
            if (ch != 'p' && ch != 'P')
                expect("exponent");
            ch = *p++;
            s = 1;
            exp_val = 0;
            if (ch == '+') {
                ch = *p++;
            } else if (ch == '-') {
                s = -1;
                ch = *p++;
            }
            if (ch < '0' || ch > '9')
                expect("exponent digits");
            while (ch >= '0' && ch <= '9') {
                exp_val = exp_val * 10 + ch - '0';
                ch = *p++;
            }
            exp_val = exp_val * s;

            /* now we can generate the number */
            /* XXX: should patch directly float number */
            d = (double)bn[1] * 4294967296.0 + (double)bn[0];
            d = ldexp(d, exp_val - frac_bits);
            t = toupper(ch);
            if (t == 'F') {
                ch = *p++;
                tok = TOK_CFLOAT;
                /* float : should handle overflow */
                tokc.f = (float)d;
            } else if (t == 'L') {
                ch = *p++;
                tok = TOK_CLDOUBLE;
                /* XXX: not large enough */
                tokc.ld = (long double)d;
            } else {
                tok = TOK_CDOUBLE;
                tokc.d = d;
            }
        } else {
            /* decimal floats */
            if (ch == '.') {
                if (q >= token_buf + STRING_MAX_SIZE)
                    goto num_too_long;
                *q++ = ch;
                ch = *p++;
                float_frac_parse:
                while (ch >= '0' && ch <= '9') {
                    if (q >= token_buf + STRING_MAX_SIZE)
                        goto num_too_long;
                    *q++ = ch;
                    ch = *p++;
                }
            }
            if (ch == 'e' || ch == 'E') {
                if (q >= token_buf + STRING_MAX_SIZE)
                    goto num_too_long;
                *q++ = ch;
                ch = *p++;
                if (ch == '-' || ch == '+') {
                    if (q >= token_buf + STRING_MAX_SIZE)
                        goto num_too_long;
                    *q++ = ch;
                    ch = *p++;
                }
                if (ch < '0' || ch > '9')
                    expect("exponent digits");
                while (ch >= '0' && ch <= '9') {
                    if (q >= token_buf + STRING_MAX_SIZE)
                        goto num_too_long;
                    *q++ = ch;
                    ch = *p++;
                }
            }
            *q = '\0';
            t = toupper(ch);
            errno = 0;
            if (t == 'F') {
                ch = *p++;
                tok = TOK_CFLOAT;
                tokc.f = strtof(token_buf, NULL);
            } else if (t == 'L') {
                ch = *p++;
                tok = TOK_CLDOUBLE;
                tokc.ld = strtold(token_buf, NULL);
            } else {
                tok = TOK_CDOUBLE;
                tokc.d = strtod(token_buf, NULL);
            }
        }
    } else {
        unsigned long long n, n1;
        int lcount, ucount;

        /* integer number */
        *q = '\0';
        q = token_buf;
        if (b == 10 && *q == '0') {
            b = 8;
            q++;
        }
        n = 0;
        while(1) {
            t = *q++;
            /* no need for checks except for base 10 / 8 errors */
            if (t == '\0') {
                break;
            } else if (t >= 'a') {
                t = t - 'a' + 10;
            } else if (t >= 'A') {
                t = t - 'A' + 10;
            } else {
                t = t - '0';
                if (t >= b)
                    error("invalid digit");
            }
            n1 = n;
            n = n * b + t;
            /* detect overflow */
            /* XXX: this test is not reliable */
            if (n < n1)
                error("integer constant overflow");
        }

        /* XXX: not exactly ANSI compliant */
        if ((n & 0xffffffff00000000LL) != 0) {
            if ((n >> 63) != 0)
                tok = TOK_CULLONG;
            else
                tok = TOK_CLLONG;
        } else if (n > 0x7fffffff) {
            tok = TOK_CUINT;
        } else {
            tok = TOK_CINT;
        }
        lcount = 0;
        ucount = 0;
        for(;;) {
            t = toupper(ch);
            if (t == 'L') {
                if (lcount >= 2)
                    error("three 'l's in integer constant");
                lcount++;
                if (lcount == 2) {
                    if (tok == TOK_CINT)
                        tok = TOK_CLLONG;
                    else if (tok == TOK_CUINT)
                        tok = TOK_CULLONG;
                }
                ch = *p++;
            } else if (t == 'U') {
                if (ucount >= 1)
                    error("two 'u's in integer constant");
                ucount++;
                if (tok == TOK_CINT)
                    tok = TOK_CUINT;
                else if (tok == TOK_CLLONG)
                    tok = TOK_CULLONG;
                ch = *p++;
            } else {
                break;
            }
        }
        if (tok == TOK_CINT || tok == TOK_CUINT)
            tokc.ui = n;
        else
            tokc.ull = n;
    }
    if (ch)
        error("invalid number\n");
}


int main() {
    const char p[] = "0x123";
    parse_number(p, p);
    return 0;
}