#ifndef J1939_TP_MGR_TYPES_H_
#define J1939_TP_MGR_TYPES_H_

#include <stdint.h>

#include <J1939/j1939_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 */
typedef enum j1939_tp_timeouts {
    J1939_TP_TO_Tr = 200,    // 200 ms
    J1939_TP_TO_Th = 500,    // 500 ms
    J1939_TP_TO_T1 = 750,    // 750 ms
    J1939_TP_TO_T2 = 1250,   // 1250 ms
    J1939_TP_TO_T3 = 1250,   // 1250 ms
    J1939_TP_TO_T4 = 1050,   // 1050 ms
    J1939_TP_TO_INF = 0x7FFFFFFF,
} j1939_tp_timeouts;


/**
 * @brief
 */
typedef enum {
    J1939_TP_STATE_FREE = 0,
    J1939_TP_STATE_RESERVED,
    J1939_TP_STATE_TIMEDOUT,
    J1939_TP_STATE_READY,
    J1939_TP_STATE_TRANSMIT,
    J1939_TP_STATE_WAIT_CTS,
    J1939_TP_STATE_WAIT_EOMA,
} j1939_tp_session_state;


/**
 * @brief
 */
typedef enum j1939_tp_session_mode {
    J1939_TP_MODE_BAM,
    J1939_TP_MODE_RTS,
} j1939_tp_session_mode;


/**
 * @brief
 */
typedef enum j1939_tp_session_dir {
    J1939_TP_DIR_UNKNOWN,
    J1939_TP_DIR_IN,
    J1939_TP_DIR_OUT,
} j1939_tp_session_dir;


/**
 * @brief
 */
typedef struct j1939_tp_session {
    uint8_t id;
    volatile uint8_t state;
    uint8_t mode;
    uint8_t dir;
    
    // pkt transmition control fields
    volatile int transmition_timeout;
    
    volatile uint8_t total_pkt_num;
    volatile uint8_t pkt_max;
    volatile uint8_t pkt_next;
    
    // pkt data and info
    uint32_t time;
    uint32_t PGN;
    uint8_t dst_addr;
    uint8_t src_addr;
    uint16_t msg_sz;
    uint8_t buffer[J1939_MAX_DATA_SZ];
} j1939_tp_session;


/**
 * @brief
 */
typedef struct j1939_tp_mgr_ctx {
    volatile int reset;
    /*
     * J1939-21, 5.10.5.1:
     *
     * A node must also be able to support one RTS/CTS session and one BAM session concurrently from the same source
     * address. Therefore, the responder must use the destination address of the two transport protocol messages to keep
     * them properly separated.
     */
    uint8_t bam_rx_tab[256];
    uint8_t rts_rx_tab[256];

    /*
     * J1939-21, 5.10.5.1:
     *
     * Each node on the network can originate one destination specific connection transfer with a given destination at a time.
     * This is due to the fact that the TP.DT only contains the source address and destination address and not the PGN of the
     * data being transferred.
     *
     * Only one multipacket BAM (i.e. global destination) can be sent from an originator at a given time. This is due to TP.DT
     * not containing the actual PGN or a connection identifier. However, responders (i.e. receiving devices in this specific
     * example) must recognize that multiple multipacket messages can be received, interspersed with one another, from
     * different originators (i.e. source addresses).
     */
    uint8_t xxx_tx_tab[256]; /* 255 index is BAM session id */

    union {
        j1939_tp_session sessions[J1939_TP_SESSIONS_NUM + 1 /* plus one session for BAM session */];

        struct {
            j1939_tp_session rx_sessions[J1939_TP_RX_SESSIONS_NUM];
            j1939_tp_session tx_sessions[J1939_TP_TX_SESSIONS_NUM + 1 /* 0 index is BAM session */];
        };
    };
} j1939_tp_mgr_ctx;


#ifdef __cplusplus
}
#endif

#endif
