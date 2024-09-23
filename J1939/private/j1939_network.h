#ifndef J1939_PRIVATE_NETWORK_H_
#define J1939_PRIVATE_NETWORK_H_

#include <stdint.h>

#include "j1939_phandle.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int j1939_network_setup(j1939_phandle phandle, uint8_t preferred_address, const j1939_CA_name *const name);
extern int j1939_network_claim_address(j1939_phandle phandle);

extern int j1939_network_rx_process(j1939_phandle phandle, const j1939_rx_info *const rx_info);
extern int j1939_network_rx_handler(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time);

extern int j1939_network_process(j1939_phandle phandle, uint32_t t_delta);

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_NETWORK_H_ */
