/**
 * @file j1939.h
 * 
 * @brief
 */


#ifndef J1939_H
#define J1939_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#include "j1939_std_addr.h"
#include "j1939_std_pgn.h"

#define J1939_CAN_BAUDRATE              250000

#define J1939_PACKET_DATA_SZ            8
#define J1939_MULTIPACKET_DATA_SZ       7
#define J1939_MAX_DATA_SZ               (J1939_MULTIPACKET_DATA_SZ * 255)

#define J1939_CONTROL_PRIORITY          3
#define J1939_GENERIC_PRIORITY          6
#define J1939_TP_PRIORITY               7
#define J1939_MAX_PRIORITY              7


/**
 * @brief
 */
typedef union {
    struct {
        uint32_t identity_number : 21;
        uint32_t manufacturer_code : 11;
        uint8_t ECU_instance : 3;
        uint8_t function_instance : 5;
        uint8_t function;
        uint8_t __reserved__ : 1;
        uint8_t vehicle_system : 7;
        uint8_t vehicle_instance : 4;
        uint8_t industry_group : 3;
        uint8_t arbitrary_address_capable : 1;
    };
    uint64_t name;
} j1939_CA_name;


/**
 * @brief J1939 PGN format
 */
typedef union PGN_format {
    uint32_t value;
    struct {
        // [0]
        union {
            uint8_t pdu_specific; // PS
            uint8_t dest_address; // DA
        };
        // [1]
        uint8_t pdu_format; // PF
        // [2]
        uint8_t dp : 1;
        uint8_t edp : 1;
        uint8_t __zeros__ : 6;
        // [3]
        uint8_t __padding__;
    };
} PGN_format;


/**
 * @brief Cast integer value to PGN_format type
 *
 * @param x Value to cast
 * @return PGN_format
 */
static inline PGN_format TREAT_AS_PGN(uint32_t x) {
    PGN_format PGN = { x };
    return PGN;
}


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU1(PGN_format PGN) {
    return PGN.pdu_format < 240;
}


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU2(PGN_format PGN) {
    return PGN.pdu_format >= 240;
}


/**
 * @brief
 * 
 * @param PGN
 */
static inline void j1939_PGN_code_set(PGN_format *const PGN, uint32_t value) {
    PGN->value = value;
    PGN->__zeros__ = 0;
    PGN->__padding__ = 0;
}


/**
 * @brief
 * 
 * @param PGN
 * @return 
 */
static inline uint32_t j1939_PGN_code_get(PGN_format PGN) {
    PGN.__zeros__ = 0;
    PGN.__padding__ = 0;
    
    // The PDU Format is 239 or less and the PDU Specific field is set to 0
    if (j1939_is_PDU1(PGN))
        PGN.pdu_specific = 0;
    
    return PGN.value;
}

/**
 * @brief
 */
typedef enum j1939_rx_tx_errno {
    J1939_RX_TX_ERROR_SUCCESS = 0,
    J1939_RX_TX_ERROR_FAILED = 1,
    J1939_RX_TX_ERROR_EXISTS,
    J1939_RX_TX_ERROR_ABORTED,
    J1939_RX_TX_ERROR_LOST_PACKET,
    J1939_RX_TX_ERROR_TIMEDOUT,
} j1939_rx_tx_errno;

///
typedef void (*j1939_callback_rx_handler)(uint32_t PGN, uint8_t src_address, uint16_t msg_sz, const void *const payload, uint32_t time);
///
typedef void (*j1939_callback_rx_tx_error_handler)(j1939_rx_tx_errno error, uint32_t PGN, uint8_t address, uint16_t msg_sz);

/**
 * @brief
 */
typedef struct j1939_callbacks {
    j1939_callback_rx_handler rx_handler;
    j1939_callback_rx_tx_error_handler rx_error_handler;
    j1939_callback_rx_tx_error_handler tx_error_handler;
} j1939_callbacks;


/**
 * @brief J1939 frame
 */
typedef struct j1939_primitive {
    PGN_format PGN;
    uint8_t priority;
    uint8_t src_address; // SA
    uint16_t dlc;
    uint8_t payload[8];
} j1939_primitive;


void j1939_initialize(const j1939_callbacks *const callbacks);
void j1939_configure(uint8_t preferred_address, const j1939_CA_name *const CA_name);

uint8_t j1939_get_address(void);
int j1939_claim_address(uint8_t address);

int j1939_sendmsg_p(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority);
int j1939_sendmsg(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);
int j1939_sendraw(const j1939_primitive *const primitive);

// int j1939_write_request(...);

int j1939_process(void);
int j1939_handle_receiving(const j1939_primitive *const frame, uint32_t time);
int j1939_handle_transmiting(void);


#ifdef __cplusplus
}
#endif

#endif /* J1939_MESSAGE_H */

