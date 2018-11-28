#include "unity/unity_fixture.h"


int main(int argc, const char **argv) {
    void runAllTests(void);
    return UnityMain(argc, argv, runAllTests);
}


void runAllTests(void) {
    RUN_TEST_GROUP(j1939_claim_address);
    RUN_TEST_GROUP(j1939_handle_PGN_request);
    RUN_TEST_GROUP(j1939_sendmsg_generic);
    RUN_TEST_GROUP(j1939_receive_generic);
    RUN_TEST_GROUP(j1939_tp_mgr_receive_bam);
    RUN_TEST_GROUP(j1939_tp_mgr_receive_rts_cts);
    RUN_TEST_GROUP(j1939_tp_mgr_send_bam);
    RUN_TEST_GROUP(j1939_tp_mgr_send_rts);
}
