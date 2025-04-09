#ifndef J1939_NETWORK_H_
#define J1939_NETWORK_H_


#define J1939_ACK_NO_GF                             0xFF


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 */
typedef enum j1939_request_status {
    J1939_REQ_HANDLED = 0,
    J1939_REQ_NOT_SUPPORTED = 1,
    J1939_REQ_ACCESS_DENIED = 2,
    J1939_REQ_BUSY = 3
} j1939_request_status;


/**
 * @brief
 */
typedef enum j1939_ack_control {
    J1939_ACK_POSITIVE = 0,
    J1939_ACK_NEGATIVE = 1,
    J1939_ACK_ACCESS_DENIED = 2,
    J1939_ACK_BUSY = 3
} j1939_ack_control;

#ifdef __cplusplus
}
#endif

#endif /* J1939_NETWORK_H_ */
