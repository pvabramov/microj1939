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
static unittest_j1939_claim_msg nodes[8];


TEST_GROUP(j1939_network);


TEST_SETUP(j1939_network) {
    TEST_ASSERT_EQUAL(0, unittest_helpers_setup(CAN_INDEX));

    /* need to be configured each time for one test */
    j1939_configure(CAN_INDEX, CA_ADDR, &CA_name);

    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* process one tick */
    j1939_process(CAN_INDEX);

    /* empty read of "Claim Address" */
    unittest_get_output(NULL);

    /* 250 ms in order to claim address */
    unittest_add_time(250);

    /* we have waited for 250 ms to claim address */
    j1939_process(CAN_INDEX);

    /* next tick */
    unittest_add_time(20);
}


TEST_TEAR_DOWN(j1939_network) {
    unittest_helpers_cleanup();
}


TEST(j1939_network, observe_nodes) {
    /* send request to observe nodes */
    TEST_ASSERT_EQUAL(0, j1939_observenodes(CAN_INDEX));

    /* controller should send Request PGN of Claim Address */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(0xEA00,                   jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,                  jframe.src_address);
    TEST_ASSERT_EQUAL(3,                        jframe.dlc);
    TEST_ASSERT_EQUAL(0x00,                     jframe.payload[0]);
    TEST_ASSERT_EQUAL(0xEE,                     jframe.payload[1]);
    TEST_ASSERT_EQUAL(0x00,                     jframe.payload[2]);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0x01, 8,
        /* NAME */
        01, 02, 00, 00, 00, 00, 00, 00);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0x02, 8,
        /* NAME */
        02, 03, 00, 00, 00, 00, 00, 00);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0x03, 8,
        /* NAME */
        03, 04, 00, 00, 00, 00, 00, 00);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0xFE, 8,
        /* NAME */
        01, 01, 01, 01, 01, 01, 01, 01);

    j1939_process(CAN_INDEX);

    TEST_ASSERT_EQUAL(4, unittest_get_nodes(nodes));

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[0].index);
    TEST_ASSERT_EQUAL(0x01,                         nodes[0].address);
    TEST_ASSERT_EQUAL_UINT64(0x0201,                nodes[0].name.name);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[1].index);
    TEST_ASSERT_EQUAL(0x02,                         nodes[1].address);
    TEST_ASSERT_EQUAL_UINT64(0x0302,                nodes[1].name.name);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[2].index);
    TEST_ASSERT_EQUAL(0x03,                         nodes[2].address);
    TEST_ASSERT_EQUAL_UINT64(0x0403,                nodes[2].name.name);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[3].index);
    TEST_ASSERT_EQUAL(254,                          nodes[3].address);
    TEST_ASSERT_EQUAL_UINT64(0x0101010101010101,    nodes[3].name.name);

    /* next tick */
    unittest_add_time(1000);

    /* observing timeout has been occured, now we should ignore Claim Address messages not addressed to us */
    j1939_process(CAN_INDEX);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0x01, 8,
        /* NAME */
        01, 02, 00, 00, 00, 00, 00, 00);

    TEST_ASSERT_EQUAL(0, unittest_get_nodes(nodes));
}


TEST_GROUP_RUNNER(j1939_network) {
    RUN_TEST_CASE(j1939_network, observe_nodes);
}
