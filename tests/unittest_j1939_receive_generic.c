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


TEST_GROUP(j1939_receive_generic);



TEST_SETUP(j1939_receive_generic) {
    memset(&rx_msg, 0xFF, sizeof(unittest_j1939_rx_msg));

    TEST_ASSERT_EQUAL(0, unittest_helpers_setup());

    /* need to be configured each time for one test */
    j1939_configure(CA_ADDR, &CA_name);

    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* empty read of "Claim Address" */
    unittest_get_output(NULL);

    /* process one IDLE tick */
    j1939_process();
    unittest_add_time(20);
}


TEST_TEAR_DOWN(j1939_receive_generic) {
    unittest_helpers_cleanup();
}


TEST(j1939_receive_generic, no_receive) {
    /* for first time there is no any input yet */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_receive_generic, no_receive_PDU1_message_to_another_node) {
    unittest_post_input(0x4200, 0x20, 0x66, 2, 0x31, 0x32);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) < 0);
}


TEST(j1939_receive_generic, receive_PDU1_message_data_len_0) {
    unittest_post_input(0x2F00, CA_ADDR, 0x20, 0);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0x2F00,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x20,     rx_msg.SA);
    TEST_ASSERT_EQUAL(0,        rx_msg.len);
}


TEST(j1939_receive_generic, receive_PDU1_message_data_len_5) {
    unittest_post_input(0x5600, CA_ADDR, 0x66, 5, 0x31, 0x32, 0x33, 0x34, 0x35);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0x5600,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x66,     rx_msg.SA);
    TEST_ASSERT_EQUAL(5,        rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
}


TEST(j1939_receive_generic, receive_PDU1_message_to_all_data_len_7) {
    unittest_post_input(0x7100, 0xFF, 0x31, 7, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0x7100,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.SA);
    TEST_ASSERT_EQUAL(7,        rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
}


TEST(j1939_receive_generic, receive_PDU2_message_data_len_1) {
    unittest_post_input(0xF021, 254, 0x20, 1, 0x31);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xF021,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x20,     rx_msg.SA);
    TEST_ASSERT_EQUAL(1,        rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
}


TEST(j1939_receive_generic, receive_PDU2_message_data_len_3) {
    unittest_post_input(0xFF10, 254, 0x70, 3, 0x31, 0x32, 0x33);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xFF10,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x70,     rx_msg.SA);
    TEST_ASSERT_EQUAL(3,        rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
}


TEST(j1939_receive_generic, receive_PDU2_message_data_len_8) {
    unittest_post_input(0xF433, 254, 0x90, 8, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38);

    /* process tick */
    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xF433,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x90,     rx_msg.SA);
    TEST_ASSERT_EQUAL(8,        rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
    TEST_ASSERT_EQUAL(0x38,     rx_msg.data[7]);
}


TEST_GROUP_RUNNER(j1939_receive_generic) {
    RUN_TEST_CASE(j1939_receive_generic, no_receive);
    RUN_TEST_CASE(j1939_receive_generic, no_receive_PDU1_message_to_another_node);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU1_message_data_len_0);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU1_message_data_len_5);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU1_message_to_all_data_len_7);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU2_message_data_len_1);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU2_message_data_len_3);
    RUN_TEST_CASE(j1939_receive_generic, receive_PDU2_message_data_len_8);
}
