#ifndef J1939_NETWORK_H_
#define J1939_NETWORK_H_

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

#ifdef __cplusplus
}
#endif

#endif /* J1939_NETWORK_H_ */
