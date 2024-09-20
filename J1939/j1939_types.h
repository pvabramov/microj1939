#ifndef J1939_TYPES_H_
#define J1939_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define J1939_CONTROL_PRIORITY          3
#define J1939_GENERIC_PRIORITY          6
#define J1939_TP_PRIORITY               7
#define J1939_MAX_PRIORITY              7

#define J1939_PGN_PDU1_MASK             0x3FF00U    // PDU1 mask
#define J1939_PGN_PDU2_MASK             0x3FFFFU    // PDU2 mask

#define J1939_PGN_PDU_FORMAT_MASK       0x0FF00U    // PDU format mask
#define J1939_PGN_PDU_FORMAT_SEP        0x0F000U    // PDU format separator


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


/**
 * @brief
 */
typedef enum j1939_request_status {
    J1939_REQ_HANDLED = 0,
    J1939_REQ_NOT_SUPPORTED = 1,
    J1939_REQ_ACCESS_DENIED = 2,
    J1939_REQ_BUSY = 3
} j1939_request_status;

///
typedef void (*j1939_callback_rx_handler)(uint8_t index, uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time);
///
typedef void (*j1939_callback_rx_tx_error_handler)(uint8_t index, j1939_rx_tx_errno error, uint32_t PGN, uint8_t address, uint16_t msg_sz);
///
typedef j1939_request_status (*j1939_callback_request_handler)(uint8_t index, uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint32_t time);
///
typedef int (*j1939_callback_claim_handler)(uint8_t index, uint8_t address, const j1939_CA_name *const name);

/**
 * @brief
 */
typedef struct j1939_callbacks {
    j1939_callback_rx_handler rx_handler;
    j1939_callback_request_handler request_handler;
    j1939_callback_claim_handler claim_handler; // result is ignored
    j1939_callback_claim_handler cannot_claim_handler;
    j1939_callback_rx_tx_error_handler rx_error_handler;
    j1939_callback_rx_tx_error_handler tx_error_handler;
} j1939_callbacks;


/**
 * @brief J1939 frame
 */
typedef struct j1939_primitive {
    uint32_t PGN;
    uint16_t dlc;
    uint8_t dest_address; // DA
    uint8_t src_address; // SA
    uint8_t priority;
    uint8_t payload[8];
} j1939_primitive;

#ifdef __cplusplus
}
#endif

#endif /* J1939_TYPES_H_ */
