#include <string.h>

#include "unity/unity_fixture.h"
#include "helpers/unittest_helpers.h"


#ifndef CA_ADDR
#define CA_ADDR 40
#endif


// this is a just test sample, never mind
static const j1939_CA_name CA_name = {
    .identity_number = 0,               //
    .manufacturer_code = 0,             // Reserved
    .ECU_instance = 0,                  // First ECU (only one controller)
    .function = 45,                     // Water Pump Control (see SAE J1939 Appendix (Table B11))
    .function_instance = 0,             // First Instance (only one device)
    .vehicle_instance = 0,              // First Instance (only one device)
    .vehicle_system = 6,                // Sprayers
    .industry_group = 2,                // Agricultural and Forestry Equipment
    .arbitrary_address_capable = 0,     // Unsupport self-configuration
};


static unittest_j1939_rx_msg rx_msg;


TEST_GROUP(j1939_tp_mgr_receive_bam);


TEST_SETUP(j1939_tp_mgr_receive_bam) {
    memset(&rx_msg, 0xFF, sizeof(unittest_j1939_rx_msg));

    TEST_ASSERT_EQUAL(0, unittest_helpers_setup());

    /* need to be configured each time for one test */
    j1939_configure(CA_ADDR, &CA_name);

    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* empty read of "Claim Address" */
    unittest_get_output(NULL);

    /* process one IDLE tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);
}


TEST_TEAR_DOWN(j1939_tp_mgr_receive_bam) {
    unittest_helpers_cleanup();
}


TEST(j1939_tp_mgr_receive_bam, receive_BAM_message) {
    /* TP.CM = BAM */
    unittest_post_input(236 << 8, 255 /* global address */, 0x45, 8,
            0x20                /* Control Byte = BAM */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);
    /* TP.DT, 1*/
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);
    /* TP.DT, 2 */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);
    /* TP.DT, 3 (the last) */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    /* process tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    /* there are data */
    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xAD00,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x45,     rx_msg.SA);
    TEST_ASSERT_EQUAL(15,       rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
    TEST_ASSERT_EQUAL(0x38,     rx_msg.data[7]);
    TEST_ASSERT_EQUAL(0x39,     rx_msg.data[8]);
    TEST_ASSERT_EQUAL(0x3A,     rx_msg.data[9]);
    TEST_ASSERT_EQUAL(0x3B,     rx_msg.data[10]);
    TEST_ASSERT_EQUAL(0x3C,     rx_msg.data[11]);
    TEST_ASSERT_EQUAL(0x3D,     rx_msg.data[12]);
    TEST_ASSERT_EQUAL(0x3E,     rx_msg.data[13]);
    TEST_ASSERT_EQUAL(0x3F,     rx_msg.data[14]);
}


TEST(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_without_TP_CM) {
    /* TP.DT, 1*/
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);
    /* TP.DT, 2 */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);
    /* TP.DT, 3 (the last) */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    /* process tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    /* there are no data */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_on_wrong_seq_number) {
    /* TP.CM = BAM */
    unittest_post_input(236 << 8, 255 /* global address */, 0x45, 8,
            0x20                /* Control Byte = BAM */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);
    /* TP.DT, 1*/
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);
    /* TP.DT, 2 has been lost or next seq. number is wrong */
    /* TP.DT, 3 (the last) */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    /* process tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    /* there are no data, session should be closed */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_on_timedout) {
    /* TP.CM = BAM */
    unittest_post_input(236 << 8, 255 /* global address */, 0x45, 8,
            0x20                /* Control Byte = BAM */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);
    /* TP.DT, 1*/
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);
    /* TP.DT, 2 */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);

    /* process tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(750 /* T1 */);

    /* there are no data */
    TEST_ASSERT(unittest_get_input(NULL) < 0);

    /* process tick */
    j1939_process(j1939_bsp_get_time());

    /* there are no data */
    TEST_ASSERT(unittest_get_input(NULL) < 0);

    /* TP.DT, 3 (the last) */
    unittest_post_input(235 << 8, 255 /* global address */, 0x45, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    /* process tick */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    /* there are no data */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST_GROUP_RUNNER(j1939_tp_mgr_receive_bam) {
    RUN_TEST_CASE(j1939_tp_mgr_receive_bam, receive_BAM_message);
    RUN_TEST_CASE(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_without_TP_CM);
    RUN_TEST_CASE(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_on_wrong_seq_number);
    RUN_TEST_CASE(j1939_tp_mgr_receive_bam, dont_receive_BAM_message_on_timedout);
}
