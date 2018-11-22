#include "unity/unity_fixture.h"


int main(int argc, const char **argv) {
    void runAllTests(void);
    return UnityMain(argc, argv, runAllTests);
}


void runAllTests(void) {
    RUN_TEST_GROUP(j1939_claim_address);
}
