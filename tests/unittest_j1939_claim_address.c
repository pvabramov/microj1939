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
    TEST_ASSERT_EQUAL(0, unittest_helpers_setup(CAN_INDEX));

    /* need to be configured each time for one test */
    j1939_configure(CAN_INDEX, CA_ADDR, &CA_name);
}

TEST_TEAR_DOWN(j1939_claim_address) {
    unittest_helpers_cleanup();
}


TEST(j1939_claim_address, claim_address_message_sending) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

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
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));
}


static void __receive_claim_address(void) {
    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}


TEST(j1939_claim_address, abort_address_assigment) {
    j1939_primitive jframe;

    /* receiving message from another node while waiting */
    unittest_set_callback_on_delay(__receive_claim_address);

    /* try to claim address, it should be not ok */
    TEST_ASSERT(j1939_claim_address(CAN_INDEX, CA_ADDR) < 0);

    /* skip our "Claim Address" message */
    unittest_get_output(&jframe);

    /* check our address, it should be the null address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* check for "Cannot Claim Address" message */
    unittest_get_output(&jframe);

    TEST_ASSERT_EQUAL(60928U | 255 /* global address */, jframe.PGN.value);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(254 /* null address */, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, address_loosing) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

    /* skip "Claim Address" message */
    unittest_get_output(&jframe);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* now we should lose our address */
    TEST_ASSERT_EQUAL(254, j1939_get_address(CAN_INDEX));

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
   TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

   /* skip "Claim Address" message */
   unittest_get_output(&jframe);

   /* check our address, it should be like we've pointed in j1939_claim_address() */
   TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

   /* send "Claim Address" by another node that has same address but has less priority */
   unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80);

   /* now we shouldn't lose our address */
   TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

   /* check for "Claim Address" message from self */
   unittest_get_output(&jframe);

   TEST_ASSERT_EQUAL(60928U | 255 /* global address */, jframe.PGN.value);
   TEST_ASSERT_EQUAL(8, jframe.dlc);
   TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
   TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}

TEST(j1939_claim_address, response_on_request_claim_address) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

    /* skip "Claim Address" message */
    unittest_get_output(&jframe);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Request PGN" : "Claim Address" */
    unittest_post_input(CAN_INDEX, 59904U, CA_ADDR, 20, 3, 0x00, 0xEE, 0x00);

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
    RUN_TEST_CASE(j1939_claim_address, abort_address_assigment);
    RUN_TEST_CASE(j1939_claim_address, address_loosing);
    RUN_TEST_CASE(j1939_claim_address, address_protecting);
    RUN_TEST_CASE(j1939_claim_address, response_on_request_claim_address);
}
