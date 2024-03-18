/**
 * @file j1939_private.h
 * 
 * @brief
 */


#ifndef J1939_PRIVATE_H
#define J1939_PRIVATE_H

#include <string.h>

#include "j1939_private_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief
 * 
 * @return 
 */
static inline uint8_t U8_MIN(uint8_t a, uint8_t b) {
    return (a < b) ? a : b;
}


/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @return uint16_t 
 */
static inline uint16_t U16_MIN(uint16_t a, uint16_t b) {
    return (a < b) ? a : b;
}


/**
 * @brief
 * 
 * @param PGN
 * @param priority
 * @param SA
 * @param DLC
 * @param payload
 * 
 * @return 
 */
static inline j1939_primitive j1939_primitive_build(uint32_t PGN, uint8_t priority, uint8_t SA, uint8_t DA, uint8_t DLC, const void *const payload) {
    struct j1939_primitive msg = {
        .priority = (priority > J1939_MAX_PRIORITY) ? J1939_MAX_PRIORITY : priority,
        .src_address = SA,
        .dlc = U8_MIN(DLC, 8),
    };
    
    j1939_PGN_code_set(&msg.PGN, PGN);
    
    /* PDU1 format takes PS as dest.address */
    if (j1939_is_PDU1(msg.PGN)) {
        msg.PGN.dest_address = DA;
    }
    
    memset(msg.payload, 0xFF, sizeof(msg.payload));
    memcpy(msg.payload, payload, msg.dlc);
    
    return msg;
}

extern j1939_handle __j1939_handles[];

extern int __j1939_send_lock(uint8_t index, const j1939_primitive *const primitive);
extern int __j1939_receive_notify(uint8_t index, uint32_t type, uint32_t PGN, uint8_t src_addr, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint32_t time);
extern int __j1939_tx_error_notify(uint8_t index, j1939_rx_tx_errno error, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz);
extern int __j1939_rx_error_notify(uint8_t index, j1939_rx_tx_errno error, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz);

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_H */

