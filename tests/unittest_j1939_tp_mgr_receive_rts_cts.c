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
static j1939_primitive jframe;


TEST_GROUP(j1939_tp_mgr_receive_rts_cts);


TEST_SETUP(j1939_tp_mgr_receive_rts_cts) {
    memset(&rx_msg, 0xFF, sizeof(unittest_j1939_rx_msg));
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


TEST_TEAR_DOWN(j1939_tp_mgr_receive_rts_cts) {
    unittest_helpers_cleanup();
}


TEST(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_one_packet_per_CTS) {

    /*
     * ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x45, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            1,                  /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);


    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x45,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE FIRST DATA FRAME
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x45, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);

    j1939_process();
    unittest_add_time(20);

    /* on TP_DT receiving controller should send CTS to make acknowledge */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x45,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(2,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE SECOND DATA FRAME
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x45, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);

    j1939_process();
    unittest_add_time(20);

    /* on TP_DT receiving controller should send CTS to make acknowledge */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x45,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE THIRD DATA FRAME
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x45, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    j1939_process();
    unittest_add_time(20);

    /* on the last TP_DT receiving controller should send EndOfMsgAck to close connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x45,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(19,                           jframe.payload[0]);         /* Control byte = EndOfMsgAck */
    TEST_ASSERT_EQUAL(15,                           jframe.payload[1]);         /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0,                            jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                            jframe.payload[3]);         /* Total number of packets */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * COMPLETE RECEIVING
     */

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xAD00,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x45,     rx_msg.SA);
    TEST_ASSERT_EQUAL(15,       rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
    TEST_ASSERT_EQUAL(0x38,     rx_msg.data[7]);
    TEST_ASSERT_EQUAL(0x39,     rx_msg.data[8]);
    TEST_ASSERT_EQUAL(0x3A,     rx_msg.data[9]);
    TEST_ASSERT_EQUAL(0x3B,     rx_msg.data[10]);
    TEST_ASSERT_EQUAL(0x3C,     rx_msg.data[11]);
    TEST_ASSERT_EQUAL(0x3D,     rx_msg.data[12]);
    TEST_ASSERT_EQUAL(0x3E,     rx_msg.data[13]);
    TEST_ASSERT_EQUAL(0x3F,     rx_msg.data[14]);
    TEST_ASSERT_EQUAL(270,      rx_msg.time);

    /* there is only one multiframe message */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_two_packets_per_CTS) {

    /*
     * ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x33, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            2,                  /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);


    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x33,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(2,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE FIRST & SECOND DATA FRAME
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);

    j1939_process();
    unittest_add_time(20);

    /* on TP_DT receiving controller should send CTS to make acknowledge */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x33,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE THIRD DATA FRAME
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    j1939_process();
    unittest_add_time(20);

    /* on the last TP_DT receiving controller should send EndOfMsgAck to close connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x33,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(19,                           jframe.payload[0]);         /* Control byte = EndOfMsgAck */
    TEST_ASSERT_EQUAL(15,                           jframe.payload[1]);         /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0,                            jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                            jframe.payload[3]);         /* Total number of packets */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * COMPLETE RECEIVING
     */

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xAD00,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.SA);
    TEST_ASSERT_EQUAL(15,       rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
    TEST_ASSERT_EQUAL(0x38,     rx_msg.data[7]);
    TEST_ASSERT_EQUAL(0x39,     rx_msg.data[8]);
    TEST_ASSERT_EQUAL(0x3A,     rx_msg.data[9]);
    TEST_ASSERT_EQUAL(0x3B,     rx_msg.data[10]);
    TEST_ASSERT_EQUAL(0x3C,     rx_msg.data[11]);
    TEST_ASSERT_EQUAL(0x3D,     rx_msg.data[12]);
    TEST_ASSERT_EQUAL(0x3E,     rx_msg.data[13]);
    TEST_ASSERT_EQUAL(0x3F,     rx_msg.data[14]);
    TEST_ASSERT_EQUAL(270,      rx_msg.time);

    /* there is only one multiframe message */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_any_packets_per_CTS) {
    /*
     * ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x33, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,               /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);


    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x33,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE 1st, 2nd, 3rd DATA FRAMES
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);
    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    j1939_process();
    unittest_add_time(20);

    /* on the last TP_DT receiving controller should send EndOfMsgAck to close connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x33,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(19,                           jframe.payload[0]);         /* Control byte = EndOfMsgAck */
    TEST_ASSERT_EQUAL(15,                           jframe.payload[1]);         /* Total message size, number of bytes */
    TEST_ASSERT_EQUAL(0,                            jframe.payload[2]);
    TEST_ASSERT_EQUAL(3,                            jframe.payload[3]);         /* Total number of packets */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * COMPLETE RECEIVING
     */

    TEST_ASSERT(unittest_get_input(&rx_msg) > 0);

    TEST_ASSERT_EQUAL(0xAD00,   rx_msg.PGN);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.SA);
    TEST_ASSERT_EQUAL(15,       rx_msg.len);
    TEST_ASSERT_EQUAL(0x31,     rx_msg.data[0]);
    TEST_ASSERT_EQUAL(0x32,     rx_msg.data[1]);
    TEST_ASSERT_EQUAL(0x33,     rx_msg.data[2]);
    TEST_ASSERT_EQUAL(0x34,     rx_msg.data[3]);
    TEST_ASSERT_EQUAL(0x35,     rx_msg.data[4]);
    TEST_ASSERT_EQUAL(0x36,     rx_msg.data[5]);
    TEST_ASSERT_EQUAL(0x37,     rx_msg.data[6]);
    TEST_ASSERT_EQUAL(0x38,     rx_msg.data[7]);
    TEST_ASSERT_EQUAL(0x39,     rx_msg.data[8]);
    TEST_ASSERT_EQUAL(0x3A,     rx_msg.data[9]);
    TEST_ASSERT_EQUAL(0x3B,     rx_msg.data[10]);
    TEST_ASSERT_EQUAL(0x3C,     rx_msg.data[11]);
    TEST_ASSERT_EQUAL(0x3D,     rx_msg.data[12]);
    TEST_ASSERT_EQUAL(0x3E,     rx_msg.data[13]);
    TEST_ASSERT_EQUAL(0x3F,     rx_msg.data[14]);
    TEST_ASSERT_EQUAL(270,      rx_msg.time);

    /* there is only one multiframe message */
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_rts_cts, connection_abort_on_wrong_seq_number) {
    /*
     * ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x91, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,               /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);

    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x91,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE 1st, (no) --2nd--, 3rd DATA FRAMES
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x91, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);
    unittest_post_input(235 << 8, CA_ADDR, 0x91, 8,
            3,                  /* Sequence Number */
            0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    j1939_process();
    unittest_add_time(20);

    /* on wrong seq.number receiving controller should send Conn_Abort to close connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x91,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(255,                          jframe.payload[0]);         /* Control byte = Conn_Abort */
    TEST_ASSERT_EQUAL(2,                            jframe.payload[1]);         /* Connection Abort reason
                                                                                   SAE J1939-21:
                                                                                       System resources were needed for another
                                                                                       task so this connection managed session was
                                                                                       terminated. */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * BROKEN RECEIVING
     */

    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST(j1939_tp_mgr_receive_rts_cts, connection_abort_on_already_managed_session) {
    /*
     * THE FIRST ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x91, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,               /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);

    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x91,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE SECOND ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x91, 8,
            16                  /* Control Byte = RTS */,
            0x09, 0x00,         /* Total message size = 15 */
            2,                  /* Total number of packets */
            2,                  /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAE, 0x00    /* PGN of the packeted message */);

    j1939_process();
    unittest_add_time(20);

    /* on another RTS receiving controller should send Conn_Abort to not establish the second session from same source */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x91,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(255,                          jframe.payload[0]);         /* Control byte = Conn_Abort */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[1]);         /* Connection Abort reason
                                                                                   SAE J1939-21:
                                                                                       Already in one or more connection
                                                                                       managed sessions and cannot support another. */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAE,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);
}


TEST(j1939_tp_mgr_receive_rts_cts, receive_connection_abort) {
    /*
     * ESTABLISHING CONNECTION
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x91, 8,
            16                  /* Control Byte = RTS */,
            0x0F, 0x00,         /* Total message size = 15 */
            3,                  /* Total number of packets */
            0xFF,               /* Maximum number of packets that can be sent in response to one CTS */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);

    j1939_process();
    unittest_add_time(20);

    /* on RTS receiving controller should send CTS to establish connection */
    TEST_ASSERT_EQUAL(0, unittest_get_output(&jframe));

    TEST_ASSERT_EQUAL(236 << 8 | 0x91,              jframe.PGN.value);          /* TP_CM */
    TEST_ASSERT_EQUAL(CA_ADDR,                      jframe.src_address);
    TEST_ASSERT_EQUAL(8,                            jframe.dlc);
    TEST_ASSERT_EQUAL(17,                           jframe.payload[0]);         /* Control byte = CTS */
    TEST_ASSERT_EQUAL(3,                            jframe.payload[1]);         /* Number of packets that can be sent */
    TEST_ASSERT_EQUAL(1,                            jframe.payload[2]);         /* Next packet number to be sent */
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[5]);         /* PGN bytes */
    TEST_ASSERT_EQUAL(0xAD,                         jframe.payload[6]);
    TEST_ASSERT_EQUAL(0x00,                         jframe.payload[7]);

    /*
     * THE 1st, 2nd, DATA FRAMES
     */

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            1,                  /* Sequence Number */
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37);

    unittest_post_input(235 << 8, CA_ADDR, 0x33, 8,
            2,                  /* Sequence Number */
            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E);

    j1939_process();
    unittest_add_time(20);

    TEST_ASSERT(unittest_get_output(NULL) < 0);

    /*
     * Connection abort message
     */

    /* TP.CM = RTS */
    unittest_post_input(236 << 8, CA_ADDR, 0x91, 8,
            255                 /* Control Byte = Conn_Abort */,
            3,                  /* Connection Abort reason
                                   SAE J1939-21:
                                       A timeout occurred and this is
                                       the connection abort to close the session. */
            0xFF, 0xFF, 0xFF,   /* Reserved */
            0x00, 0xAD, 0x00    /* PGN of the packeted message */);

    j1939_process();
    unittest_add_time(20);

    /*
     * BROKEN RECEIVING
     */

    TEST_ASSERT(unittest_get_output(NULL) < 0);
    TEST_ASSERT(unittest_get_input(NULL) < 0);
}


TEST_GROUP_RUNNER(j1939_tp_mgr_receive_rts_cts) {
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_one_packet_per_CTS);
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_two_packets_per_CTS);
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, receive_RTS_message_any_packets_per_CTS);
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, connection_abort_on_wrong_seq_number);
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, connection_abort_on_already_managed_session);
    RUN_TEST_CASE(j1939_tp_mgr_receive_rts_cts, receive_connection_abort);
}
