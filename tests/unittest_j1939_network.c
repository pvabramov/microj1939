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

static const j1939_software_identification software = {
    .nfields        = 3,
    .identification = "unittest*j1939_network*software*",
};

static const j1939_component_identification component = {
    .make           = "UTEST",
    .model          = "j1939",
    .serial_number  = "123456789",
};


static j1939_primitive jframe;
static unittest_j1939_observing_msg nodes[8];


TEST_GROUP(j1939_network);


TEST_SETUP(j1939_network) {
    TEST_ASSERT_EQUAL(0, unittest_helpers_setup_ext(CAN_INDEX, &software, &component));

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
    TEST_ASSERT_EQUAL(J1939_OBSERVING_NODES_START,  nodes[0].state);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[1].index);
    TEST_ASSERT_EQUAL(0x02,                         nodes[1].address);
    TEST_ASSERT_EQUAL_UINT64(0x0302,                nodes[1].name.name);
    TEST_ASSERT_EQUAL(J1939_OBSERVING_NODES_PROC,   nodes[1].state);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[2].index);
    TEST_ASSERT_EQUAL(0x03,                         nodes[2].address);
    TEST_ASSERT_EQUAL_UINT64(0x0403,                nodes[2].name.name);
    TEST_ASSERT_EQUAL(J1939_OBSERVING_NODES_PROC,   nodes[2].state);

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[3].index);
    TEST_ASSERT_EQUAL(254,                          nodes[3].address);
    TEST_ASSERT_EQUAL_UINT64(0x0101010101010101,    nodes[3].name.name);
    TEST_ASSERT_EQUAL(J1939_OBSERVING_NODES_PROC,   nodes[3].state);

    /* next tick */
    unittest_add_time(1000);

    /* observing timeout has been occured, now we should ignore Claim Address messages not addressed to us */
    j1939_process(CAN_INDEX);

    unittest_post_input(CAN_INDEX, 238 << 8, 0xFF, 0x01, 8,
        /* NAME */
        01, 02, 00, 00, 00, 00, 00, 00);

    /* the end of obsering should be */
    TEST_ASSERT_EQUAL(1, unittest_get_nodes(nodes));

    TEST_ASSERT_EQUAL(CAN_INDEX,                    nodes[0].index);
    TEST_ASSERT_EQUAL(254,                          nodes[0].address);
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFFFFFFFFF,    nodes[0].name.name);
    TEST_ASSERT_EQUAL(J1939_OBSERVING_NODES_END,    nodes[0].state);

    /* next tick */
    unittest_add_time(1000);

    /* observing timeout has been occured, now we should ignore Claim Address messages not addressed to us */
    j1939_process(CAN_INDEX);

    /* no any observed nodes should be appeared */
    TEST_ASSERT_EQUAL(0, unittest_get_nodes(nodes));
}


TEST(j1939_network, software_identification) {
    unittest_post_input(CAN_INDEX, 234 << 8, CA_ADDR, 251, 3,
        0xDA, 0xFE, 0x00                                /* Parameter Group Number being requested = Software Identification */
    );

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* controller should send answer via TP */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(33 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(5,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(5,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0xDA,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0xFE,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* now controller should wait for CTS frame from destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 251, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        5,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0xDA, 0xFE, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets "unittest*j1939_network*software*" */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x03,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('u',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('n',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('i',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('t',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('t',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('e',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('s',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('t',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('*',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('j',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('1',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('9',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('3',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('9',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('_',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('n',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('e',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('t',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('w',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('o',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(4,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('r',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('k',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('*',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('s',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('o',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('f',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('t',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(5,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('w',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('a',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('r',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('e',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('*',              jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* server has completed to send software identification */
    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST(j1939_network, component_identification) {
    unittest_post_input(CAN_INDEX, 234 << 8, CA_ADDR, 251, 3,
        0xEB, 0xFE, 0x00                                /* Parameter Group Number being requested = Component Identification */
    );

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* controller should send answer via TP */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(23 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(4,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(4,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0xEB,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0xFE,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* now controller should wait for CTS frame from destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 251, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        4,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0xEB, 0xFE, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets "UTEST*j1939*123456789**" */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('U',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('T',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('E',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('S',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('T',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('*',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('j',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('1',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('9',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('3',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('9',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('*',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('1',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('2',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('3',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('4',              jframe.payload[2]);
    TEST_ASSERT_EQUAL('5',              jframe.payload[3]);
    TEST_ASSERT_EQUAL('6',              jframe.payload[4]);
    TEST_ASSERT_EQUAL('7',              jframe.payload[5]);
    TEST_ASSERT_EQUAL('8',              jframe.payload[6]);
    TEST_ASSERT_EQUAL('9',              jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* now server should send TP.TD packets */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8,         jframe.PGN);
    TEST_ASSERT_EQUAL(251,              jframe.dest_address);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(4,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL('*',              jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL('*',              jframe.payload[2]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* server has completed to send software identification */
    TEST_ASSERT(unittest_get_output(NULL) < 0);
}


TEST_GROUP_RUNNER(j1939_network) {
    RUN_TEST_CASE(j1939_network, observe_nodes);
    RUN_TEST_CASE(j1939_network, software_identification);
    RUN_TEST_CASE(j1939_network, component_identification);
}
