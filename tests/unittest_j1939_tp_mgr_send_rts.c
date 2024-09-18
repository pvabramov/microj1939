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


static j1939_primitive jframe;


TEST_GROUP(j1939_tp_mgr_send_rts);


TEST_SETUP(j1939_tp_mgr_send_rts) {
    memset(&jframe, 0xFF, sizeof(j1939_primitive));

    TEST_ASSERT_EQUAL(0, unittest_helpers_setup(CAN_INDEX));

    /* need to be configured each time for one test */
    j1939_configure(CAN_INDEX, CA_ADDR, &CA_name);

    TEST_ASSERT_EQUAL(0, j1939_claim_address(CAN_INDEX, CA_ADDR));

    /* empty read of "Claim Address" */
    unittest_get_output(NULL);

    /* process one IDLE tick */
    j1939_process(CAN_INDEX);

    /* 250 ms in order to claim address */
    unittest_add_time(250);

    /* we have waited for 250 ms to claim address */
    j1939_process(CAN_INDEX);

    /* next tick */
    unittest_add_time(20);
}


TEST_TEAR_DOWN(j1939_tp_mgr_send_rts) {
    unittest_helpers_cleanup();
}


TEST(j1939_tp_mgr_send_rts, send_RTS_message_one_frame_per_CTS) {
    uint8_t data[15] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

    /* try to send data to specific destination by RTS method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x1300, 57, 15, data));

    /* controller should send TP_CM with RTS control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 57,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(15 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(3,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x13,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* now controller should wait for CTS frame from destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 57, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x13, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /* connection should be established */

    /*
     * THE FIRST TP_DT frame
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 57,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /* cause we set Number of packets = 1, controller should wait for CTS */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    /* thus there shouldn't be an output */
    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* send CTS to continue */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 57, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        2,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x13, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 57,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /* cause we set Number of packets = 1 controller is waiting for CTS */
    /* now lets try to receive the 2nd frame one more time */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 57, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        2,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x13, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE SECOND TP_DT frame (ONE MORE TIME)
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 57,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /* cause we set Number of packets = 1 controller is waiting for CTS */
    /* tell controller to send the last frame */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 57, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        3,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x13, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE THIRD (the last) TP_DT frame
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 57,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* we got the last frame so we should send EndOfMsgAck */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 57, 8,
        19,                                                 /* Control byte = 19, End_of_Message Acknowledge */
        15, 0,                                              /* Total message size, number of bytes */
        3,                                                  /* Total number of packets */
        0xFF, 0xFF, 0xFF,
        0x00, 0x13, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /* there shouldn't be any output data from controller anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* session should be closed to check that do sendmsg one more time */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x5600, 17, 15, data));
}


TEST(j1939_tp_mgr_send_rts, send_RTS_message_two_frames_per_CTS) {
    uint8_t data[15] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

    /* try to send data to specific destination by RTS method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x5600, 17, 15, data));

    /* controller should send TP_CM with RTS control byte */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(15 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(3,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x56,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* now controller should wait for CTS frame from destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        2,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /* connection should be established */

    /*
     * THE FIRST TP_DT frame
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /* cause we set Number of packets = 2 now controller is waiting for CTS */
    /* tell controller to send the last frame */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        3,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE THIRD TP_DT frame
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* lets do retransmition of 3rd packet */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        1,                                                  /* Number of packets that can be sent. */
        3,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x00, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE THIRD (the last) TP_DT frame
     */
    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]); /* SAE J1939-21: */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]); /* The extra bytes should be filled with FF */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /* we got the last frame so we should send EndOfMsgAck */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        19,                                                 /* Control byte = 19, End_of_Message Acknowledge */
        15, 0,                                              /* Total message size, number of bytes */
        3,                                                  /* Total number of packets */
        0xFF, 0xFF, 0xFF,
        0x00, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /* there shouldn't be any output data from controller anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* session should be closed to check that do sendmsg one more time */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x1300, 57, 15, data));
}


TEST(j1939_tp_mgr_send_rts, send_two_simultaneously_RTS_messages) {
    uint8_t data[15] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

    /* try to send data to the first destination by RTS method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x5631, 17, 15, data));

    /* try to send data to the second destination by RTS method of transport protocol */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x6513, 45, 15, data));

    /* controller should send TP_CM with RTS control byte for the first destination */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(15 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(3,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x56,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    /* controller should send TP_CM with RTS control byte for the first destination */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 45,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(16,               jframe.payload[0]); /* Control byte = Request_To_Send (RTS) */
    TEST_ASSERT_EQUAL(15 /* lo byte */, jframe.payload[1]); /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0  /* hi byte */, jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                jframe.payload[3]); /* Total number of packets */
    TEST_ASSERT_EQUAL(3,                jframe.payload[4]); /* Maximum number of packets that can be sent in response to one CTS */
    TEST_ASSERT_EQUAL(0x13,             jframe.payload[5]); /* Parameter Group Number of the packeted message */
    TEST_ASSERT_EQUAL(0x65,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* now controller should wait for CTS frame from the first destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        3,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x31, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE FIRST TP_DT frame (to the first destination)
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame (to the first destination)
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /* now controller got CTS frame from the second destination to establish connection */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 45, 8,
        17,                                                 /* Control byte = 17, Destination Specific Clear_To_Send (CTS) */
        3,                                                  /* Number of packets that can be sent. */
        1,                                                  /* Next packet number to be sent */
        0xFF, 0xFF, 0xFF,
        0x13, 0x65, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /*
     * THE THIRD TP_DT frame (to the first destination)
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 17,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    /*
     * THE FIRST TP_DT frame (to the second destination)
     */

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 45,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(1,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x31,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x32,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x33,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x34,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x35,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x36,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x37,             jframe.payload[7]);

    /*
     * THE SECOND TP_DT frame (to the second destination)
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 45,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(2,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x38,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0x39,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0x3A,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0x3B,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0x3C,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0x3D,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x3E,             jframe.payload[7]);

    /*
     * THE THIRD TP_DT frame (to the second destination)
     */

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(235 << 8 | 45,    jframe.PGN.value);
    TEST_ASSERT_EQUAL(CA_ADDR,          jframe.src_address);
    TEST_ASSERT_EQUAL(8,                jframe.dlc);
    TEST_ASSERT_EQUAL(3,                jframe.payload[0]); /* Sequence Number */
    TEST_ASSERT_EQUAL(0x3F,             jframe.payload[1]); /* Packetized Data (7 bytes) */
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[2]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[3]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[4]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[5]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[6]);
    TEST_ASSERT_EQUAL(0xFF,             jframe.payload[7]);

    j1939_process(CAN_INDEX);
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* check that sessions aren't closed, cause EoMA should be received first */
    TEST_ASSERT(j1939_sendmsg(CAN_INDEX, 0x5631, 17, 15, data) < 0);
    TEST_ASSERT(j1939_sendmsg(CAN_INDEX, 0x6513, 45, 15, data) < 0);

    /* we got the last frame so we should send EndOfMsgAck */
    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 17, 8,
        19,                                                 /* Control byte = 19, End_of_Message Acknowledge */
        15, 0,                                              /* Total message size, number of bytes */
        3,                                                  /* Total number of packets */
        0xFF, 0xFF, 0xFF,
        0x31, 0x56, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    unittest_post_input(CAN_INDEX, 236 << 8, CA_ADDR, 45, 8,
        19,                                                 /* Control byte = 19, End_of_Message Acknowledge */
        15, 0,                                              /* Total message size, number of bytes */
        3,                                                  /* Total number of packets */
        0xFF, 0xFF, 0xFF,
        0x13, 0x65, 0x00                                    /* Parameter Group Number of the packeted message */
    );

    /* there shouldn't be any output data from controller anymore */
    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /* session should be closed to check that do sendmsg one more time */
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x5631, 17, 15, data));
    TEST_ASSERT_EQUAL(0, j1939_sendmsg(CAN_INDEX, 0x6513, 45, 15, data));
}


TEST_GROUP_RUNNER(j1939_tp_mgr_send_rts) {
    RUN_TEST_CASE(j1939_tp_mgr_send_rts, send_RTS_message_one_frame_per_CTS);
    RUN_TEST_CASE(j1939_tp_mgr_send_rts, send_RTS_message_two_frames_per_CTS);
    RUN_TEST_CASE(j1939_tp_mgr_send_rts, send_two_simultaneously_RTS_messages);
}
