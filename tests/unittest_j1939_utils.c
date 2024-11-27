#include "unity/unity_fixture.h"

#include <J1939/j1939_utils.h>


TEST_GROUP(j1939_utils);

TEST_SETUP(j1939_utils) {}
TEST_TEAR_DOWN(j1939_utils) {}

TEST(j1939_utils, canid_parsing_pdu1) {
    uint32_t canid0 = 0x1CEE2413;

    TEST_ASSERT_EQUAL(7,        j1939_canid_get_Priority(canid0));
    TEST_ASSERT_EQUAL(0xEE00,   j1939_canid_get_PGN(canid0));
    TEST_ASSERT_EQUAL(0x24,     j1939_canid_get_DA(canid0));
    TEST_ASSERT_EQUAL(0x13,     j1939_canid_get_SA(canid0));
}

TEST(j1939_utils, canid_parsing_pdu2) {
    uint32_t canid0 = 0x18F02413;

    TEST_ASSERT_EQUAL(6,        j1939_canid_get_Priority(canid0));
    TEST_ASSERT_EQUAL(0xF024,   j1939_canid_get_PGN(canid0));
    TEST_ASSERT_EQUAL(255,      j1939_canid_get_DA(canid0));
    TEST_ASSERT_EQUAL(0x13,     j1939_canid_get_SA(canid0));
}

TEST(j1939_utils, canid_build_pdu1) {
    TEST_ASSERT_EQUAL(0x18CA2110, j1939_canid_build(6, 0xCA00, 0x21, 0x10, 0));
    TEST_ASSERT_EQUAL(0x18CA2110, j1939_canid_build_noflags(6, 0xCA00, 0x21, 0x10));
}

TEST(j1939_utils, canid_build_pdu2) {
    TEST_ASSERT_EQUAL(0x1CFA2114, j1939_canid_build(7, 0xFA21, 0xFF, 0x14, 0));
    TEST_ASSERT_EQUAL(0x1CFA2114, j1939_canid_build_noflags(7, 0xFA21, 0xFF, 0x14));
}

TEST(j1939_utils, j1939_primitive_sizeof) {
    TEST_ASSERT_EQUAL(16, sizeof(j1939_primitive));
}

TEST_GROUP_RUNNER(j1939_utils) {
    RUN_TEST_CASE(j1939_utils, canid_parsing_pdu1);
    RUN_TEST_CASE(j1939_utils, canid_parsing_pdu2);
    RUN_TEST_CASE(j1939_utils, canid_build_pdu1);
    RUN_TEST_CASE(j1939_utils, canid_build_pdu2);
    RUN_TEST_CASE(j1939_utils, j1939_primitive_sizeof);
}
