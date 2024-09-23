#ifndef J1939_PRIVATE_NETWROK_MSG_H_
#define J1939_PRIVATE_NETWORK_MSG_H_

#include <stdint.h>

#include "j1939_private.h"
#include "j1939_send_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

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
typedef struct __attribute__((__packed__)) j1939_payload_request {
    uint8_t PGN[3];
} j1939_payload_request;


/**
 * @brief
 */
typedef struct __attribute__((__packed__)) j1939_payload_ack {
    uint8_t control; // see j1939_ack_control
    uint8_t group_function;
    uint8_t __reserved__[2]; // should be 0xFF
    uint8_t address;
    uint8_t PGN[3];
} j1939_payload_ack;


/**
 * @brief
 *
 * @param address
 */
static inline void __send_Claim_Address(j1939_phandle phandle, uint8_t address) {
    const j1939_primitive aclm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACLM, J1939_GENERIC_PRIORITY,
                              address, J1939_GLOBAL_ADDRESS, J1939_STD_PGN_ACLM_DLC, &phandle->CA_name);
    __j1939_send_lock(phandle, &aclm_primitive);
}


/**
 * @brief
 *
 * @param ack_type
 * @param gf
 * @param originator_sa
 * @param PGN
 */
static inline void __send_ACK(j1939_phandle phandle, j1939_ack_control ack_type, uint8_t gf, uint8_t originator, uint32_t PGN) {
    j1939_payload_ack ack_body = {
        .__reserved__ = { 0xFF, 0xFF},
        .control = ack_type,
        .group_function = gf,
        .address = originator
    };

    PACK_PGN(PGN, ack_body.PGN);

    /*
     * The Acknowledgment PGN response uses a global destination address even though the PGN that
     * causes Acknowledgment was sent to a specific destination address.
     */
    const j1939_primitive ackm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACKM, J1939_GENERIC_PRIORITY,
                              phandle->address, J1939_GLOBAL_ADDRESS,
                              J1939_STD_PGN_ACKM_DLC, &ack_body);
    __j1939_send_lock(phandle, &ackm_primitive);
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_NETWORK_MSG_H_ */
