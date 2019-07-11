/**
 * @file j1939_tp_mgr.h
 * 
 * @brief
 */


#ifndef J1939_TP_MGR_H
#define J1939_TP_MGR_H


#ifdef __cplusplus
extern "C" {
#endif

    
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
 */
typedef enum j1939_tp_cm_conn_abort_reason {
    J1939_CONN_ABORT_REASON_EXISTS          = 1,
    J1939_CONN_ABORT_REASON_NO_RESOURCES    = 2,
    J1939_CONN_ABORT_REASON_TIMEDOUT        = 3,
} j1939_tp_cm_conn_abort_reason;


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
    uint8_t __reserved__[3];
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
    PGN_format PGN;
} j1939_tp_cm_control;


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


#define J1939_TP_SESSIONS_NUM 32 // XXX: maximum number of sessions should be 254


/**
 * @brief
 */
typedef struct j1939_tp_mgr_ctx {
    volatile int reset;
    uint8_t bam_rx_tab[256];
    uint8_t rts_rx_tab[256];
    uint8_t xxx_tx_tab[1]; // only one can be sent from originator at a given time
    j1939_tp_session sessions[J1939_TP_SESSIONS_NUM];
} j1939_tp_mgr_ctx;
    
    
void j1939_tp_mgr_init(j1939_tp_mgr_ctx *const tp_mgr_ctx);
int j1939_tp_mgr_rx_handler(j1939_tp_mgr_ctx *const tp_mgr_ctx, const j1939_primitive *const frame, uint32_t time);
int j1939_tp_mgr_process(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t t_delta);

int j1939_tp_mgr_open_tx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);
int j1939_tp_mgr_close_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid);


#ifdef __cplusplus
}
#endif

#endif /* J1939_TP_MGR_H */

