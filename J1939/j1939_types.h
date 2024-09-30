#ifndef J1939_TYPES_H_
#define J1939_TYPES_H_

#include <stdint.h>

#include "j1939_tx_rx_errno.h"
#include "j1939_network.h"

#include "j1939_std_addr.h"
#include "j1939_std_pgn.h"

#define J1939_CONTROL_PRIORITY          3
#define J1939_GENERIC_PRIORITY          6
#define J1939_TP_PRIORITY               7
#define J1939_MAX_PRIORITY              7

#define J1939_MAX_DL                    8U
#define J1939_PADDING_DATA              0xFFU


#ifdef __cplusplus
extern "C" {
#endif

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
typedef enum j1939_claim_status {
    FAILED = -1,
    SUCCESS = 0,
    PROCESSING = 1,
    UNKNOWN
} j1939_claim_status;

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
