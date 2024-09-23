#ifndef J1939_TX_RX_FIFO_TYPES_H_
#define J1939_TX_RX_FIFO_TYPES_H_

#include <J1939/j1939_config.h>
#include <J1939/j1939_types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* J1939_TX_RX_FIFO_TYPES_H_ */
