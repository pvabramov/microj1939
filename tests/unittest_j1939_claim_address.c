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


TEST_GROUP(j1939_claim_address);


TEST_SETUP(j1939_claim_address) {
    TEST_ASSERT_EQUAL(0, unittest_helpers_setup());

    /* need to be configured each time for one test */
    j1939_configure(CA_ADDR, &CA_name);
}

TEST_TEAR_DOWN(j1939_claim_address) {
    unittest_helpers_cleanup();
}


TEST(j1939_claim_address, claim_address_message_sending) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* j1939 sends "Claim Address" frame into CAN */

    /* check for output */
    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(60928U | 255 /* global address */, jframe.PGN.value);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, successful_address_assigment) {
    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address());
}


TEST(j1939_claim_address, address_loosing) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

    /* skip "Claim Address" message */
    unittest_get_output(&jframe);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address());

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* now we should lose our address */
    TEST_ASSERT_EQUAL(254, j1939_get_address());

    /* check for "Cannot Claim Address" message */
    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(60928U | 255 /* global address */, jframe.PGN.value);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(254 /* null address */, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}

TEST(j1939_claim_address, address_protecting) {
    j1939_primitive jframe;

   /* try to claim address, it should be ok */
   TEST_ASSERT_EQUAL(0, j1939_claim_address(CA_ADDR));

   /* skip "Claim Address" message */
   unittest_get_output(&jframe);

   /* check our address, it should be like we've pointed in j1939_claim_address() */
   TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address());

   /* send "Claim Address" by another node that has same address but has less priority */
   unittest_post_input(60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80);

   /* now we shouldn't lose our address */
   TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address());

   /* check for "Claim Address" message from self */
   unittest_get_output(&jframe);

   TEST_ASSERT_EQUAL(60928U | 255 /* global address */, jframe.PGN.value);
   TEST_ASSERT_EQUAL(8, jframe.dlc);
   TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
   TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST_GROUP_RUNNER(j1939_claim_address) {
    RUN_TEST_CASE(j1939_claim_address, claim_address_message_sending);
    RUN_TEST_CASE(j1939_claim_address, successful_address_assigment);
    RUN_TEST_CASE(j1939_claim_address, address_loosing);
    RUN_TEST_CASE(j1939_claim_address, address_protecting);
}
