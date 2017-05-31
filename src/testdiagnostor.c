

#include "config.h"

#include "token.h"
#include "diagnostor.h"
#include "cstring.h"


static
void test_diagnostor(void)
{
    diagnostor_t *diagnostor = diagnostor_create();
    linenote_caution_t linenote_caution;

    linenote_caution.start = 65;
    linenote_caution.length = 7;

    diagnostor_note(diagnostor, DIAGNOSTOR_MSGTYPE_ERROR, "helloworld");
    diagnostor_note_with_line(diagnostor, DIAGNOSTOR_MSGTYPE_NOTE, "a.c", 1,2, "helloworld");

    diagnostor_note_linenote(diagnostor, DIAGNOSTOR_MSGTYPE_ERROR,
                             "if not BillOrderTrackModel.add_order_track(tran.conn, order_id, user_id, status, order_status_description[status].format(operator='承运商'):", &linenote_caution);
    diagnostor_destroy(diagnostor);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_diagnostor();
    return 0;
}
