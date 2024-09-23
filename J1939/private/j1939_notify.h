#ifndef J1939_PRIVATE_NOTIFY_H_
#define J1939_PRIVATE_NOTIFY_H_

#include "j1939_phandle.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int __j1939_receive_notify(j1939_phandle phandle, uint32_t type, uint32_t PGN, uint8_t src_addr, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint32_t time);
extern int __j1939_tx_error_notify(j1939_phandle phandle, j1939_rx_tx_errno error, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz);
extern int __j1939_rx_error_notify(j1939_phandle phandle, j1939_rx_tx_errno error, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz);

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_NOTIFY_H_ */
