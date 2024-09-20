/**
 * @file j1939.h
 * 
 * @brief
 */


#ifndef J1939_H
#define J1939_H

#include <J1939/j1939_config.h>


#ifdef __cplusplus
extern "C" {
#endif

#include "j1939_std_addr.h"
#include "j1939_std_pgn.h"
#include "j1939_types.h"


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU1(uint32_t PGN) {
    return (PGN & J1939_PGN_PDU_FORMAT_MASK) < J1939_PGN_PDU_FORMAT_SEP;
}


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU2(uint32_t PGN) {
    return (PGN & J1939_PGN_PDU_FORMAT_MASK) >= J1939_PGN_PDU_FORMAT_SEP;
}


void j1939_initialize(uint8_t index, const j1939_callbacks *const callbacks);
void j1939_configure(uint8_t index, uint8_t preferred_address, const j1939_CA_name *const CA_name);

uint8_t j1939_get_address(uint8_t index);

// must be called in logic thread, no thread safe
int j1939_claim_address(uint8_t index);

int j1939_sendmsg_p(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority);
int j1939_sendmsg(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);
int j1939_sendraw(uint8_t index, const j1939_primitive *const primitive);

// int j1939_write_request(...);

int j1939_process(uint8_t index);
int j1939_handle_receiving(uint8_t index, const j1939_primitive *const frame, uint32_t time);
int j1939_handle_transmiting(uint8_t index);


#ifdef __cplusplus
}
#endif

#endif /* J1939_MESSAGE_H */

