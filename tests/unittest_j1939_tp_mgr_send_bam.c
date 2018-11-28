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


static j1939_primitive jframe;


TEST_GROUP(j1939_tp_mgr_send_bam);


TEST_SETUP(j1939_tp_mgr_send_bam) {
    memset(&jframe, 0xFF, sizeof(j1939_primitive));

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


TEST_TEAR_DOWN(j1939_tp_mgr_send_bam) {
    unittest_helpers_cleanup();
}


TEST(j1939_tp_mgr_send_bam, send_BAM_message) {
    uint8_t data[15] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

    /* try to send data to all by BAM method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0x1300, J1939_GLOBAL_ADDRESS, 15, data));

    /* controller should send TP_CM with BAM control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(32,               jframe.payload[0]); /* Control byte = Broadcast Announce Message */
    TEST_ASSERT_EQUAL(15 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]); /* Reserved for assignment by SAE, this byte should be filled with FF */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x13,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    /* now controller should send 3 TP_DT frames to completely transfer data */

    /*
     * THE FIRST TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /*
     * THE THIRD (the last) TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* there shouldn't be any data anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST(j1939_tp_mgr_send_bam, sendmsg_returns_error_on_already_managed_BAM_transmition) {
    uint8_t data[9] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

    /* try to send data to all by BAM method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0x4500, J1939_GLOBAL_ADDRESS, 9, data));

    /* get failure on already sending a BAM message */
    TEST_ASSERT(j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 15, data) < 0);

    /* checking the first sendmsg call was not interrupted */
    /* controller should send TP_CM with BAM control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(32,               jframe.payload[0]); /* Control byte = Broadcast Announce Message */
    TEST_ASSERT_EQUAL(9  /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(2,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]); /* Reserved for assignment by SAE, this byte should be filled with FF */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x45,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    /* now controller should send 2 TP_DT frames to completely transfer data */

    /*
     * THE FIRST TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);


    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* there shouldn't be any data anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST(j1939_tp_mgr_send_bam, send_two_BAM_messages_in_sequence) {
    uint8_t data[9] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

    /* try to send data to all by BAM method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0x4500, J1939_GLOBAL_ADDRESS, 9, data));

    /* get failure on already sending a BAM message */
    TEST_ASSERT(j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 9, data) < 0);

    /* controller should send TP_CM with BAM control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(32,               jframe.payload[0]); /* Control byte = Broadcast Announce Message */
    TEST_ASSERT_EQUAL(9  /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(2,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]); /* Reserved for assignment by SAE, this byte should be filled with FF */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x45,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    /* get failure on already sending a BAM message */
    TEST_ASSERT(j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 9, data) < 0);

    /* now controller should send 2 TP_DT frames to completely transfer data */

    /*
     * THE FIRST TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    /* get failure on already sending a BAM message */
    TEST_ASSERT(j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 9, data) < 0);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /* get failure on already sending a BAM message */
    TEST_ASSERT(j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 9, data) < 0);

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* now we can send a new BAM with another PGN */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0x6400, J1939_GLOBAL_ADDRESS, 9, data));

    /* controller should send TP_CM with BAM control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(32,               jframe.payload[0]); /* Control byte = Broadcast Announce Message */
    TEST_ASSERT_EQUAL(9  /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(2,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]); /* Reserved for assignment by SAE, this byte should be filled with FF */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x64,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    /* now controller should send 2 TP_DT frames to completely transfer data */

    /*
     * THE FIRST TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(j1939_bsp_get_time());
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 255,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* there shouldn't be any data anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST_GROUP_RUNNER(j1939_tp_mgr_send_bam) {
    RUN_TEST_CASE(j1939_tp_mgr_send_bam, send_BAM_message);
    RUN_TEST_CASE(j1939_tp_mgr_send_bam, sendmsg_returns_error_on_already_managed_BAM_transmition);
    RUN_TEST_CASE(j1939_tp_mgr_send_bam, send_two_BAM_messages_in_sequence);
}
