

#include "config.h"

#include "token.h"
#include "diagnostor.h"


static void test_diagnostor()
{
    linenote_caution_t linenote_caution;

    linenote_caution.start = 8;
    linenote_caution.length = 16;

    warningf("error...");
    warningf_with_location("<string>", 0, 0, "errorf_with_location...");
    warningf_with_linenote("<string>", 0, 0, "hdr = (cstring_header_t *) pmalloc(sizeof(cstring_header_t) + size);", "errorf_with_linenote...");
    warningf_with_linenote_caution("<string>", 0, 0, "hdr = (cstring_header_t *) pmalloc(sizeof(cstring_header_t) + size);", &linenote_caution, "errorf_with_linenote_caution...");

    errorf("error...");
    errorf_with_location("<string>", 0, 0, "errorf_with_location...");
    errorf_with_linenote("<string>", 0, 0, "hdr = (cstring_header_t *) pmalloc(sizeof(cstring_header_t) + size);", "errorf_with_linenote...");
    errorf_with_linenote_caution("<string>", 0, 0, "hdr = (cstring_header_t *) pmalloc(sizeof(cstring_header_t) + size);", &linenote_caution, "errorf_with_linenote_caution...");

    report();
}


static void test_panic(void)
{
    panicf("panicf...");
    panicf_with_location("<string>", 0, 0, "panicf_with_location...");
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_diagnostor();
    test_panic();
    return 0;
}
