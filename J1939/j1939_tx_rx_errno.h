#ifndef J1939_TX_RX_ERRNO_H_
#define J1939_TX_RX_ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* J1939_TX_RX_ERRNO_H_ */
