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


TEST_GROUP(j1939_handle_PGN_request);


TEST_SETUP(j1939_handle_PGN_request) {
    memset(&jframe, 0xFF, sizeof(j1939_primitive));

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


TEST_TEAR_DOWN(j1939_handle_PGN_request) {
    unittest_helpers_cleanup();
}


TEST(j1939_handle_PGN_request, Claim_Address_specific_request) {
    unittest_post_input(234 << 8, CA_ADDR, 251, 3,
        0x00, 0xEE, 0x00                                /* Parameter Group Number being requested = Claim_Address */
    );

    /* request to "Claim Address" should be supported by all controllers */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    /*
     * SAE J1939-21:
     *     The Address Claim PGN is sent to the global destination address even though the request for it
     *     may have been to a specific destination address (see J1939-81).
     */

    TEST_ASSERT_EQUAL(0xEEFF,                   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,                  jframe.src_address);
    TEST_ASSERT_EQUAL(8,                        jframe.dlc);
    TEST_ASSERT_EQUAL_HEX64(CA_name.name,       *(uint64_t*)jframe.payload);
}


TEST(j1939_handle_PGN_request, Claim_Address_global_request) {
    unittest_post_input(234 << 8, 255 /* global address */, 251, 3,
        0x00, 0xEE, 0x00                                /* Parameter Group Number being requested = Claim_Address */
    );

    /* request to "Claim Address" should be supported by all controllers */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    /*
     * SAE J1939-21:
     *     The Address Claim PGN is sent to the global destination address even though the request for it
     *     may have been to a specific destination address (see J1939-81).
     */

    TEST_ASSERT_EQUAL(0xEEFF,                   jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,                  jframe.src_address);
    TEST_ASSERT_EQUAL(8,                        jframe.dlc);
    TEST_ASSERT_EQUAL_HEX64(CA_name.name,       *(uint64_t*)jframe.payload);
}


TEST(j1939_handle_PGN_request, NACK_on_unsupported_PGN_specific_request) {
    unittest_post_input(234 << 8, CA_ADDR, 251, 3,
        0x00, 0xAF, 0x00                                /* Parameter Group Number being requested */
    );

    /* NACK */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    /*
     * SAE J1939-21:
     *     The Acknowledgment PGN response uses a global destination address even though the PGN that
     *     causes Acknowledgment was sent to a specific destination address.
     */

    TEST_ASSERT_EQUAL(232 << 8 | 255,           jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,                  jframe.src_address);
    TEST_ASSERT_EQUAL(8,                        jframe.dlc);
    TEST_ASSERT_EQUAL(1,                        jframe.payload[0]);     /* Control byte = 1, Negative Acknowledgment (NACK) */
    TEST_ASSERT_EQUAL(251 /* originator */,     jframe.payload[4]);     /* Address Negative Acknowledgement */
    TEST_ASSERT_EQUAL(0x00,                     jframe.payload[5]);     /* Parameter Group Number of requested information */
    TEST_ASSERT_EQUAL(0xAF,                     jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                     jframe.payload[7]);
}


TEST(j1939_handle_PGN_request, no_response_on_unsupported_PGN_global_request) {
    unittest_post_input(234 << 8, 255 /* global request */, 251, 3,
        0x00, 0xAF, 0x00                                /* Parameter Group Number being requested */
    );

    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST_GROUP_RUNNER(j1939_handle_PGN_request) {
    RUN_TEST_CASE(j1939_handle_PGN_request, Claim_Address_specific_request);
    RUN_TEST_CASE(j1939_handle_PGN_request, Claim_Address_global_request);
    RUN_TEST_CASE(j1939_handle_PGN_request, NACK_on_unsupported_PGN_specific_request);
    RUN_TEST_CASE(j1939_handle_PGN_request, no_response_on_unsupported_PGN_global_request);
}
