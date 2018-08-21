/**
 * @file j1939_tp_mgr_msg.h
 * 
 * @brief
 */


#ifndef J1939_TP_MGR_INC_H
#define J1939_TP_MGR_INC_H

#include "j1939_private.h"



#ifdef __cplusplus
extern "C" {
#endif
    

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
static inline j1939_tp_cm_control __new_tp_cm_RTS(uint16_t total_msg_sz, uint8_t total_pkt_num, uint8_t max_pkt_num, uint32_t PGN) {
    const j1939_tp_cm_control payload = {
        .control = J1939_TP_CM_RTS,
        .RTS.total_msg_sz = total_msg_sz,
        .RTS.total_pkt_num = total_pkt_num,
        .RTS.max_pkt_num = max_pkt_num,      
        .PGN = PGN
    };
    return payload;
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
static inline j1939_tp_cm_control __new_tp_cm_CTS(uint8_t pkt_num, uint8_t pkt_next, uint32_t PGN) {
    const j1939_tp_cm_control payload = {
        .control = J1939_TP_CM_CTS,
        .CTS.pkt_num = pkt_num,
        .CTS.pkt_next = pkt_next,
        .CTS.__reserved__ = { 0xFF, 0xFF },
        .PGN = PGN
    };
    return payload;
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
static inline j1939_tp_cm_control __new_tp_cm_EoMA(uint16_t total_msg_sz, uint8_t total_pkt_num, uint32_t PGN) {
    const j1939_tp_cm_control payload = {
        .control = J1939_TP_CM_EndOfMsgACK,
        .EoMA.total_msg_sz = total_msg_sz,
        .EoMA.total_pkt_num = total_pkt_num,
        .EoMA.__reserved__ = { 0xFF },
        .PGN = PGN
    };
    return payload;
}


/**
 * @brief
 * 
 * @param reason
 * @param PGN
 * 
 * @return 
 */
static inline j1939_tp_cm_control __new_tp_cm_Conn_Abort(uint8_t reason, uint32_t PGN) {
    const j1939_tp_cm_control payload = {
        .control = J1939_TP_CM_Conn_Abort,
        .Conn_Abort.reason = reason,
        .Conn_Abort.__reserved__ = { 0xFF, 0xFF, 0xFF },
        .PGN = PGN
    };
    return payload;
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
static inline j1939_tp_cm_control __new_tp_cm_BAM(uint16_t total_msg_sz, uint8_t total_pkt_num, uint32_t PGN) {
    const j1939_tp_cm_control payload = {
        .control = J1939_TP_CM_BAM,
        .BAM.total_msg_sz = total_msg_sz,
        .BAM.total_pkt_num = total_pkt_num,
        .BAM.__reserved__ = { 0xFF },
        .PGN = PGN
    };
    return payload;
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
static inline int __send_TPDT(uint8_t src_addr, uint8_t dst_addr, const j1939_tp_dt *const tp_dt) {
    j1939_primitive primitive = j1939_primitive_build(J1939_STD_PGN_TPDT,
                                                      J1939_TP_PRIORITY,
                                                      src_addr, dst_addr,
                                                      J1939_STD_PGN_TPDT_DLC,
                                                      tp_dt);
    return __j1939_send_lock(&primitive);
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
static inline int __send_TPCM(uint8_t src_addr, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm_control) {
    j1939_primitive primitive = j1939_primitive_build(J1939_STD_PGN_TPCM,
                                                      J1939_TP_PRIORITY,
                                                      src_addr, dst_addr,
                                                      J1939_STD_PGN_TPCM_DLC,
                                                      tp_cm_control);
    return __j1939_send_lock(&primitive);
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
static inline int __send_CTS(uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint8_t pkt_num, uint8_t pkt_next) {
    j1939_tp_cm_control payload = __new_tp_cm_CTS(pkt_num, pkt_next, PGN);
    return __send_TPCM(src_addr, dst_addr, &payload);
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
static inline int __send_Conn_Abort(uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint8_t reason) {
    j1939_tp_cm_control payload = __new_tp_cm_Conn_Abort(reason, PGN);
    return __send_TPCM(src_addr, dst_addr, &payload);
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
static inline int __send_EoMA(uint8_t src_addr, uint8_t dst_addr, uint32_t PGN, uint16_t total_msg_sz, uint8_t total_pkt_num) {
    j1939_tp_cm_control payload = __new_tp_cm_EoMA(total_msg_sz, total_pkt_num, PGN);
    return __send_TPCM(src_addr, dst_addr, &payload);
}


#ifdef __cplusplus
}
#endif

#endif /* J1939_TP_MGR_INC_H */

