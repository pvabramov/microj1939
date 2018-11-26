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


TEST_GROUP(j1939_sendmsg_generic);


TEST_SETUP(j1939_sendmsg_generic) {
    TEST_ASSERT_EQUAL(0, unittest_helpers_setup());

    /* need to be configured each time for one test */
    j1939_configure(CA_ADDR, &CA_name);

    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* empty read of "Claim Address" */
    unittest_get_output(NULL);
}


TEST_TEAR_DOWN(j1939_sendmsg_generic) {
    unittest_helpers_cleanup();
}


TEST(j1939_sendmsg_generic, send_frame_data_len_0) {
    j1939_primitive jframe;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xAF00, 20, 0, NULL));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xAF14,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(0,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_1) {
    j1939_primitive jframe;
    uint8_t data[1];

    data[0] = 0x31;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB000, 21, 1, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB015,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(1,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_2) {
    j1939_primitive jframe;
    uint8_t data[2];

    data[0] = 0x31;
    data[1] = 0x32;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB100, 22, 2, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB116,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(2,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_3) {
    j1939_primitive jframe;
    uint8_t data[3];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB200, 23, 3, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB217,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(3,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_4) {
    j1939_primitive jframe;
    uint8_t data[4];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB300, 24, 4, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB318,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(4,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_5) {
    j1939_primitive jframe;
    uint8_t data[5];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB400, 25, 5, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB419,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(5,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_6) {
    j1939_primitive jframe;
    uint8_t data[6];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB500, 26, 6, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB51A,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(6,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_7) {
    j1939_primitive jframe;
    uint8_t data[7];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;
    data[6] = 0x37;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB600, 27, 7, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB61B,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(7,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x37,     jframe.payload[6]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_8) {
    j1939_primitive jframe;
    uint8_t data[8];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;
    data[6] = 0x37;
    data[7] = 0x38;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg(0xB700, 28, 8, data));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB71C,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(8,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x37,     jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x38,     jframe.payload[7]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_0_with_priority) {
    j1939_primitive jframe;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xAF00, 20, 0, NULL, 1));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xAF14,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(0,        jframe.dlc);
    TEST_ASSERT_EQUAL(1,        jframe.priority);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_1_with_priority) {
    j1939_primitive jframe;
    uint8_t data[1];

    data[0] = 0x31;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB000, 21, 1, data, 2));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB015,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(1,        jframe.dlc);
    TEST_ASSERT_EQUAL(2,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_2_with_priority) {
    j1939_primitive jframe;
    uint8_t data[2];

    data[0] = 0x31;
    data[1] = 0x32;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB100, 22, 2, data, 3));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB116,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(2,        jframe.dlc);
    TEST_ASSERT_EQUAL(3,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_3_with_priority) {
    j1939_primitive jframe;
    uint8_t data[3];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB200, 23, 3, data, 4));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB217,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(3,        jframe.dlc);
    TEST_ASSERT_EQUAL(4,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_4_with_priority) {
    j1939_primitive jframe;
    uint8_t data[4];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB300, 24, 4, data, 5));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB318,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(4,        jframe.dlc);
    TEST_ASSERT_EQUAL(5,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_5_with_priority) {
    j1939_primitive jframe;
    uint8_t data[5];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB400, 25, 5, data, 7));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB419,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(5,        jframe.dlc);
    TEST_ASSERT_EQUAL(7,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_6_with_priority) {
    j1939_primitive jframe;
    uint8_t data[6];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB500, 26, 6, data, 6));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB51A,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(6,        jframe.dlc);
    TEST_ASSERT_EQUAL(6,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_7_with_priority) {
    j1939_primitive jframe;
    uint8_t data[7];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;
    data[6] = 0x37;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB600, 27, 7, data, 3));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB61B,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(7,        jframe.dlc);
    TEST_ASSERT_EQUAL(3,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x37,     jframe.payload[6]);
}


TEST(j1939_sendmsg_generic, send_frame_data_len_8_with_priority) {
    j1939_primitive jframe;
    uint8_t data[8];

    data[0] = 0x31;
    data[1] = 0x32;
    data[2] = 0x33;
    data[3] = 0x34;
    data[4] = 0x35;
    data[5] = 0x36;
    data[6] = 0x37;
    data[7] = 0x38;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB700, 28, 8, data, 0));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB71C,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(8,        jframe.dlc);
    TEST_ASSERT_EQUAL(0,        jframe.priority);
    TEST_ASSERT_EQUAL(0x31,     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0x32,     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x33,     jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x34,     jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x35,     jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x36,     jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x37,     jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x38,     jframe.payload[7]);
}


TEST(j1939_sendmsg_generic, send_frame_with_out_of_priority) {
    j1939_primitive jframe;
    uint8_t data[1];

    data[0] = 0x88;

    TEST_ASSERT_EQUAL(0, j1939_sendmsg_p(0xB700, 28, 8, data, 8));

    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(0xB71C,   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,  jframe.src_address);
    TEST_ASSERT_EQUAL(8,        jframe.dlc);
    /* the max priority is 7, so it should be saturated to 7 */
    TEST_ASSERT_EQUAL(7,        jframe.priority);
    TEST_ASSERT_EQUAL(0x88,     jframe.payload[0]);
}


TEST_GROUP_RUNNER(j1939_sendmsg_generic) {
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_0);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_1);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_2);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_3);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_4);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_5);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_6);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_7);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_8);

    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_0_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_1_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_2_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_3_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_4_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_5_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_6_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_7_with_priority);
    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_data_len_8_with_priority);

    RUN_TEST_CASE(j1939_sendmsg_generic, send_frame_with_out_of_priority);
}
