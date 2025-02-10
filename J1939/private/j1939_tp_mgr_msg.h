#ifndef J1939_TP_MGR_INC_H
#define J1939_TP_MGR_INC_H

#include "j1939_private.h"
#include "j1939_send_lock.h"


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
 *
 * @param total_msg_sz
 * @param total_pkt_num
 * @param max_pkt_num
 * @param PGN
 *
 * @return
 */
static inline j1939_tp_cm_control* __new_tp_cm_RTS(uint16_t total_msg_sz, uint8_t total_pkt_num, uint8_t max_pkt_num, uint32_t PGN, j1939_tp_cm_control *const tp) {
    tp->control             = J1939_TP_CM_RTS;

    tp->RTS.total_msg_sz    = total_msg_sz;
    tp->RTS.total_pkt_num   = total_pkt_num;
    tp->RTS.max_pkt_num     = max_pkt_num;

    PACK_PGN(PGN, tp->PGN);

    return tp;
}


/**
 * @brief
 *
 * @param pkt_num
 * @param pkt_next
 * @param PGN
 *
 * @return
 */
static inline j1939_tp_cm_control* __new_tp_cm_CTS(uint8_t pkt_num, uint8_t pkt_next, uint32_t PGN, j1939_tp_cm_control *const tp) {
    tp->control = J1939_TP_CM_CTS;

    tp->CTS.pkt_num         = pkt_num;
    tp->CTS.pkt_next        = pkt_next;
    tp->CTS.__reserved__[0] = 0xFF;
    tp->CTS.__reserved__[1] = 0xFF;

    PACK_PGN(PGN, tp->PGN);

    return tp;
}


/**
 * @brief
 *
 * @param total_msg_sz
 * @param total_pkt_num
 * @param PGN
 *
 * @return
 */
static inline j1939_tp_cm_control* __new_tp_cm_EoMA(uint16_t total_msg_sz, uint8_t total_pkt_num, uint32_t PGN, j1939_tp_cm_control *const tp) {
    tp->control = J1939_TP_CM_EndOfMsgACK;

    tp->EoMA.total_msg_sz       = total_msg_sz;
    tp->EoMA.total_pkt_num      = total_pkt_num;
    tp->EoMA.__reserved__[0]    = 0xFF;

    PACK_PGN(PGN, tp->PGN);

    return tp;
}


/**
 * @brief
 *
 * @param reason
 * @param PGN
 *
 * @return
 */
static inline j1939_tp_cm_control* __new_tp_cm_Conn_Abort(uint8_t reason, uint8_t role, uint32_t PGN, j1939_tp_cm_control *const tp) {
    tp->control = J1939_TP_CM_Conn_Abort;

    tp->Conn_Abort.reason           = reason;
    tp->Conn_Abort.role             = 0xFC | (role & 0x3);
    tp->Conn_Abort.__reserved__[0]  = 0xFF;
    tp->Conn_Abort.__reserved__[1]  = 0xFF;

    PACK_PGN(PGN, tp->PGN);

    return tp;
}


/**
 * @brief
 *
 * @param total_msg_sz
 * @param total_pkt_num
 * @param PGN
 *
 * @return
 */
static inline j1939_tp_cm_control* __new_tp_cm_BAM(uint16_t total_msg_sz, uint8_t total_pkt_num, uint32_t PGN, j1939_tp_cm_control *const tp) {
    tp->control = J1939_TP_CM_BAM;

    tp->BAM.total_msg_sz    = total_msg_sz;
    tp->BAM.total_pkt_num   = total_pkt_num;
    tp->BAM.__reserved__[0] = 0xFF;

    PACK_PGN(PGN, tp->PGN);

    return tp;
}


/**
 * @brief
 *
 * @param src_addr
 * @param dst_addr
 * @param tp_dt
 *
 * @return
 */
static inline int __send_TPDT(j1939_phandle phandle, uint8_t src_addr, uint8_t dst_addr, const j1939_tp_dt *const tp_dt) {
    j1939_primitive frame;
    return __j1939_send_control(
        phandle,
        j1939_primitive_build(J1939_STD_PGN_TPDT, J1939_TP_PRIORITY,
                              src_addr, dst_addr, J1939_STD_PGN_TPDT_DLC, tp_dt, &frame
        )
    );
}


/**
 * @brief
 *
 * @param src_addr
 * @param dst_addr
 * @param tp_cm_control
 *
 * @return
 */
static inline int __send_TPCM(j1939_phandle phandle, uint8_t src_addr, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm_control) {
    j1939_primitive frame;
    return __j1939_send_control(
        phandle,
        j1939_primitive_build(J1939_STD_PGN_TPCM, J1939_TP_PRIORITY,
                              src_addr, dst_addr, J1939_STD_PGN_TPCM_DLC, tp_cm_control, &frame
        )
    );
}


/**
 * @brief
 *
 * @param src_addr
 * @param dst_addr
 * @param PGN
 * @param pkt_num
 * @param pkt_next
 */
static inline int __send_CTS(j1939_phandle phandle, uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint8_t pkt_num, uint8_t pkt_next) {
    j1939_tp_cm_control tp;
    return __send_TPCM(phandle, src_addr, dst_addr, __new_tp_cm_CTS(pkt_num, pkt_next, PGN, &tp));
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param src_addr
 * @param dst_addr
 * @param PGN
 * @param reason
 */
static inline int __send_Conn_Abort(j1939_phandle phandle, uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint8_t reason, uint8_t role) {
    j1939_tp_cm_control tp;
    return __send_TPCM(phandle, src_addr, dst_addr, __new_tp_cm_Conn_Abort(reason, role, PGN, &tp));
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param src_addr
 * @param dst_addr
 * @param PGN
 * @param total_msg_sz
 * @param total_pkt_num
 */
static inline int __send_EoMA(j1939_phandle phandle, uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint16_t total_msg_sz, uint8_t total_pkt_num) {
    j1939_tp_cm_control tp;
    return __send_TPCM(phandle, src_addr, dst_addr, __new_tp_cm_EoMA(total_msg_sz, total_pkt_num, PGN, &tp));
}


#ifdef __cplusplus
}
#endif

#endif /* J1939_TP_MGR_INC_H */

