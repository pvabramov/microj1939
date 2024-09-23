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
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* check for output */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, successful_address_assigment) {
    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* check for output */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(NULL), "No <claim address> message");

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(100);

    /* we need to process the protocol */
    j1939_process(CAN_INDEX);

    /* check our address, it should be the null address yet */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* some time has passed */
    unittest_add_time(150);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));
}


TEST(j1939_claim_address, abort_address_assigment) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* skip our "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* set a max. random Cannot Claim Address */
    unittest_add_time(153);

    /* now we should get null address */
    j1939_process(CAN_INDEX);

    /* check our address, it should be the null address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* check for "Cannot Claim Address" message */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <cannot claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(254 /* null address */, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, address_loosing) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* set a max. random Cannot Claim Address */
    unittest_add_time(153);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* now we should lose our address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* check for "Cannot Claim Address" message */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <cannot claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(254 /* null address */, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}

TEST(j1939_claim_address, address_protecting) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Claim Address" by another node that has same address but has less priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* now we shouldn't lose our address */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* check for "Claim Address" message from self */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}

TEST(j1939_claim_address, response_on_request_claim_address) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Request PGN" : "Claim Address" */
    unittest_post_input(CAN_INDEX, 59904U, CA_ADDR, 20, 3, 0x00, 0xEE, 0x00);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* check for "Claim Address" message from self */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, response_on_request_claim_address_to_global_address_claimed) {
    j1939_primitive jframe;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Request PGN" : "Claim Address" */
    unittest_post_input(CAN_INDEX, 59904U, 255 /* global address */, 20, 3, 0x00, 0xEE, 0x00);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* check for "Claim Address" message from self */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, response_on_request_claim_address_to_global_address_noclaimeaddress) {
    j1939_primitive jframe;

     /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* we have no address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* send "Request PGN" : "Claim Address" */
    unittest_post_input(CAN_INDEX, 59904U, 255 /* global address */, 20, 3, 0x00, 0xEE, 0x00);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* check for "Claim Address" message from self */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));
}


TEST(j1939_claim_address, response_on_request_claim_address_to_global_address_cannotclaimeaddress) {
    j1939_primitive jframe0, jframe1;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* set a max. random Cannot Claim Address */
    unittest_add_time(153);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* now we should lose our address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* check for "Cannot Claim Address" message */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe0), "No <cannot claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe0.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe0.dest_address);
    TEST_ASSERT_EQUAL(8, jframe0.dlc);
    TEST_ASSERT_EQUAL(254 /* null address */, jframe0.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe0.payload));

    /* send "Request PGN" : "Claim Address" */
    unittest_post_input(CAN_INDEX, 59904U, 255 /* global address */, 20, 3, 0x00, 0xEE, 0x00);

    /* set a max. random Cannot Claim Address */
    unittest_add_time(153);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* check for "Cannot Claim Address" message from self */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe1), "No <cannot claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe1.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe1.dest_address);
    TEST_ASSERT_EQUAL(8, jframe1.dlc);
    TEST_ASSERT_EQUAL(254U, jframe1.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe1.payload));
}


TEST(j1939_claim_address, try_to_claim_another_address_on_cannot_claim_address) {
    j1939_primitive jframe;
    unittest_j1939_claim_msg cannot_claim;
    unittest_j1939_claim_msg claim;

    /* try to claim address, it should be ok */
    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX));

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* skip "Claim Address" message */
    unittest_get_output(NULL);

    /* process protocol for the first time */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(250);

    /* now we should get our address */
    j1939_process(CAN_INDEX);

    /* claim_handler should be called */
    TEST_ASSERT_EQUAL(0, unittest_get_claim(NULL));

    /* some time has passed */
    unittest_add_time(10);

    /* check our address, it should be like we've pointed in j1939_configure() */
    TEST_ASSERT_EQUAL(CA_ADDR, j1939_get_address(CAN_INDEX));

    /* dont send Cannot Claim Address, we will revoke the new address later */
    unittest_set_cannot_claim_status(1);

    /* send "Claim Address" by another node that has same address and has more priority */
    unittest_post_input(CAN_INDEX, 60928U, 255U, CA_ADDR, 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* now we should lose our address */
    TEST_ASSERT_EQUAL(254U, j1939_get_address(CAN_INDEX));

    /* cannot_claim_handler should be called */
    TEST_ASSERT_EQUAL(0, unittest_get_cannot_claim(&cannot_claim));

    TEST_ASSERT_EQUAL(CAN_INDEX, cannot_claim.index);
    TEST_ASSERT_EQUAL(CA_ADDR, cannot_claim.address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, cannot_claim.name.name);

    // cannot_claim_handler callback body {{{
    // claim new address
    j1939_configure(CAN_INDEX, CA_ADDR + 1, &CA_name);
    j1939_claim_address(CAN_INDEX);
    // }}}

    /* j1939 sends "Claim Address" frame into CAN */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(10);

    /* check for "Claim Address" message */
    TEST_ASSERT_EQUAL_MESSAGE(0, unittest_get_output(&jframe), "No <claim address> message");

    TEST_ASSERT_EQUAL(60928U, jframe.PGN);
    TEST_ASSERT_EQUAL(255 /* global address */, jframe.dest_address);
    TEST_ASSERT_EQUAL(8, jframe.dlc);
    TEST_ASSERT_EQUAL(CA_ADDR + 1, jframe.src_address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, (*(uint64_t*)jframe.payload));

    /* process the protocol */
    j1939_process(CAN_INDEX);

    /* some time has passed */
    unittest_add_time(240);

    /* now we should get new address */
    j1939_process(CAN_INDEX);

    /* check our address, it should be like we've pointed in j1939_claim_address() */
    TEST_ASSERT_EQUAL(CA_ADDR + 1, j1939_get_address(CAN_INDEX));

    /* claim_handler should be called */
    TEST_ASSERT_EQUAL(0, unittest_get_claim(&claim));

    TEST_ASSERT_EQUAL(CAN_INDEX, claim.index);
    TEST_ASSERT_EQUAL(CA_ADDR + 1, claim.address);
    TEST_ASSERT_EQUAL_HEX(CA_name.name, claim.name.name);
}


TEST_GROUP_RUNNER(j1939_claim_address) {
    RUN_TEST_CASE(j1939_claim_address, claim_address_message_sending);
    RUN_TEST_CASE(j1939_claim_address, successful_address_assigment);
    RUN_TEST_CASE(j1939_claim_address, abort_address_assigment);
    RUN_TEST_CASE(j1939_claim_address, address_loosing);
    RUN_TEST_CASE(j1939_claim_address, address_protecting);
    RUN_TEST_CASE(j1939_claim_address, response_on_request_claim_address);
    RUN_TEST_CASE(j1939_claim_address, response_on_request_claim_address_to_global_address_claimed);
    RUN_TEST_CASE(j1939_claim_address, response_on_request_claim_address_to_global_address_noclaimeaddress);
    RUN_TEST_CASE(j1939_claim_address, response_on_request_claim_address_to_global_address_cannotclaimeaddress);
    RUN_TEST_CASE(j1939_claim_address, try_to_claim_another_address_on_cannot_claim_address);
}
