/**
 * @file j1939_private.h
 * 
 * @brief
 */


#ifndef J1939_PRIVATE_H
#define J1939_PRIVATE_H

#include <stdint.h>
#include <string.h>

#include <J1939/j1939.h>


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
    if (j1939_is_PDU1(msg.PGN))
        msg.PGN.dest_address = DA;
    
    memset(msg.payload, 0xFF, sizeof(msg.payload));
    memcpy(msg.payload, payload, msg.dlc);
    
    return msg;
}


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
typedef struct j1939_payload_request {
    PGN_format PGN;
} j1939_payload_request;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_payload_ack {
    uint8_t control; // see j1939_ack_control
    uint8_t group_function;
    uint8_t __reserved__[2]; // should be 0xFF
    uint8_t address;
    PGN_format PGN;
} j1939_payload_ack;
    

/**
 * @brief
 */
typedef enum j1939_rx_info_type {
    J1939_RX_INFO_TYPE_UNKNOWN      = 0,
    J1939_RX_INFO_TYPE_FRAME        = 1,
    J1939_RX_INFO_TYPE_MULTIPACKET  = 0x40000000,
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


extern int __j1939_send_lock(const j1939_primitive *const primitive);
extern int __j1939_receive_notify(uint32_t type, uint32_t PGN, uint8_t src_addr, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint32_t time);
extern int __j1939_tx_error_notify(j1939_rx_tx_errno error, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz);
extern int __j1939_rx_error_notify(j1939_rx_tx_errno error, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz);

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_H */

