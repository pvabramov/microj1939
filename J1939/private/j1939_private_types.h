#ifndef J1939_PRIVATE_TYPES_H_
#define J1939_PRIVATE_TYPES_H_

#include <J1939/j1939_config.h>
#include <J1939/j1939_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief
 */
typedef enum j1939_ack_control {
    J1939_ACK_POSITIVE = 0,
    J1939_ACK_NEGATIVE = 1,
    J1939_ACK_ACCESS_DENIED = 2,
    J1939_ACK_BUSY = 3
} j1939_ack_control;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_payload_request {
    uint8_t PGN[3];
} j1939_payload_request;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_payload_ack {
    uint8_t control; // see j1939_ack_control
    uint8_t group_function;
    uint8_t __reserved__[2]; // should be 0xFF
    uint8_t address;
    uint8_t PGN[3];
} j1939_payload_ack;
    

/**
 * @brief
 */
typedef enum j1939_rx_info_type {
    J1939_RX_INFO_TYPE_UNKNOWN      = 0,
    J1939_RX_INFO_TYPE_FRAME        = 1,
    J1939_RX_INFO_TYPE_REQUEST      = 2,
    J1939_RX_INFO_TYPE_MULTIPACKET  = 0x40000000
} j1939_rx_info_type;
    
    
/**
 * @brief
 */
typedef struct {
    j1939_rx_info_type type;
    uint32_t PGN;
    uint8_t src_addr;
    uint8_t dst_addr;
    uint8_t sid;
    uint16_t msg_sz;
    union {
        uint8_t payload[8];
        const void *payload_ptr;
    };
    uint32_t time;
} j1939_rx_info;


/**
 * @brief
 */
typedef struct {
    j1939_rx_tx_errno error;
    uint32_t PGN;
    uint16_t msg_sz;
    uint8_t addr; // SA or DA
} j1939_rx_tx_error_info;


/**
 * @brief
 */
typedef struct j1939_rx_fifo {
    unsigned head;
    unsigned tail;
    j1939_rx_info items[J1939_RX_FIFO_SZ];
} j1939_rx_fifo;

/**
 * @brief
 */
typedef struct j1939_tx_fifo {
    unsigned head;
    unsigned tail;
    j1939_primitive items[J1939_TX_FIFO_SZ];
} j1939_tx_fifo;

/**
 * @brief
 */
typedef struct j1939_rx_tx_error_fifo {
    unsigned head;
    unsigned tail;
    j1939_rx_tx_error_info items[J1939_RX_TX_ERROR_FIFO_SZ];
} j1939_rx_tx_error_fifo;


/**
 * @brief
 * 
 * J1939-21 (2006), figure 21, Format of Messages For Transport Protocol
 */
typedef enum j1939_tp_cm_format {
    J1939_TP_CM_RTS = 16,
    J1939_TP_CM_CTS = 17,
    J1939_TP_CM_EndOfMsgACK = 19,
    J1939_TP_CM_Conn_Abort = 255,
    J1939_TP_CM_BAM = 32,
} j1939_tp_cm_format;


/**
 * @brief
 * 
 * J1939-21 (2006), table 7, Connection Abort Reason
 * J1939-21 (2022), table 6, Connection Abort Reason
 */
typedef enum j1939_tp_cm_conn_abort_reason {
    J1939_CONN_ABORT_REASON_EXISTS          = 1,
    J1939_CONN_ABORT_REASON_NO_RESOURCES    = 2,
    J1939_CONN_ABORT_REASON_TIMEDOUT        = 3,
    J1939_CONN_ABORT_REASON_CTS_IN_PROGRESS = 4,
    J1939_CONN_ABORT_REASON_RETRASMIT_LIMIT_REACHED = 5,
    J1939_CONN_ABORT_REASON_UNEXPECTED_PACKET = 6,
    J1939_CONN_ABORT_REASON_BAD_SEQUENCE = 7,
    J1939_CONN_ABORT_REASON_DUP_SEQUENCE = 8,
    J1939_CONN_ABORT_REASON_BIG_MESSAGE = 9,
    J1939_CONN_ABORT_REASON_UNKNOWN = 250,
} j1939_tp_cm_conn_abort_reason;


/**
 * J1939-21 (2022), figure 14, Format of messages for transport protocol
 */
typedef enum j1939_tp_cm_conn_abort_role {
    J1939_CONN_ABORT_ROLE_ORIGINATOR = 0,
    J1939_CONN_ABORT_ROLE_RESPONDER = 1,
    J1939_CONN_ABORT_ROLE_NOT_IMPLEMENTED = 3,
} j1939_tp_cm_conn_abort_role;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_RTS_control {
    uint16_t total_msg_sz;
    uint8_t total_pkt_num;
    uint8_t max_pkt_num;
} j1939_tp_cm_RTS_control;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_CTS_control {
    uint8_t pkt_num;
    uint8_t pkt_next;
    uint8_t __reserved__[2];
} j1939_tp_cm_CTS_control;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_EoMA_control {
    uint16_t total_msg_sz;
    uint8_t total_pkt_num;
    uint8_t __reserved__[1];
} j1939_tp_cm_EoMA_control;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_Conn_Abort_control {
    uint8_t reason;
    uint8_t role;
    uint8_t __reserved__[2];
} j1939_tp_cm_Conn_Abort_control;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_BAM_control {
    uint16_t total_msg_sz;
    uint8_t total_pkt_num;
    uint8_t __reserved__[1];
} j1939_tp_cm_BAM_control;

    
/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_cm_control {
    uint8_t control; // see j1939_tp_cm_control
    union {
        j1939_tp_cm_RTS_control         RTS;
        j1939_tp_cm_CTS_control         CTS;
        j1939_tp_cm_EoMA_control        EoMA;
        j1939_tp_cm_Conn_Abort_control  Conn_Abort;
        j1939_tp_cm_BAM_control         BAM;
    };
    uint8_t PGN[3];
} j1939_tp_cm_control;


#undef J1939_MULTIPACKET_DATA_SZ
#define J1939_MULTIPACKET_DATA_SZ           7

/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_tp_dt {
    uint8_t seq_num;
    uint8_t payload[J1939_MULTIPACKET_DATA_SZ];
} j1939_tp_dt;


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


/**
 * @brief
 */
typedef enum j1939_state {
    FATAL_ERROR = -1,
    NOT_STARTED = 0,
    INITIALIZED,
    ATEMPT_TO_CLAIM_ADDRESS,
    CANNOT_CLAIM_ADDRESS,
    ACTIVE,
    BUS_OFF,
} j1939_state;


/**
 * @brief
 */
typedef struct j1939_handle {
    int oneshot;
    uint32_t last_time;

    volatile j1939_claim_status claim_status;
    volatile j1939_state state;
    volatile int already_tx;
    volatile int already_rx;

    volatile uint8_t address;
    volatile uint8_t preferred_address;

    j1939_CA_name CA_name;

    j1939_tp_mgr_ctx tp_mgr_ctx;
    j1939_rx_fifo rx_fifo;
    j1939_tx_fifo tx_fifo;
    j1939_rx_tx_error_fifo rx_error_fifo;
    j1939_rx_tx_error_fifo tx_error_fifo;

    volatile j1939_callback_claim_handler claim_handler;
    volatile j1939_callback_claim_handler cannot_claim_handler;

    j1939_callbacks callbacks;
    int preidle_timer;
    int claim_timer;
    int random_timer;
} j1939_handle;


#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_TYPES_H_ */
