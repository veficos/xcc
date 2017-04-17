

#include "config.h"
#include "unittest.h"
#include "diag.h"
#include "option.h"
#include "number.h"
#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static
void test_number()
{
    diag_t diag;
    struct option_s option;
    token_t tok;

    diag = diag_create();
    tok = token_create();

    tok->cs = cstring_cpy_n(tok->cs, "0xbbbb", 6);
    tok->type = TOKEN_NUMBER;

    TEST_COND("0xbbbb", parse_number(diag, &option, tok));
    TEST_COND("0xbbbb", tok->number.ul == 0xbbbb);

    tok->cs = cstring_cpy_n(tok->cs, "0b1234", 6);
    TEST_COND("0b1234", !parse_number(diag, &option, tok));

    tok->cs = cstring_cpy_n(tok->cs, "0b1111", 6);
    TEST_COND("0b1111", parse_number(diag, &option, tok));
    TEST_COND("0b1111", tok->number.ul == 0xf);
 
    token_destroy(tok);
    diag_destroy(diag);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_number();
    TEST_REPORT();
    return 0;
}
