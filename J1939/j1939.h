/**
 * @file j1939.h
 * 
 * @brief
 */


#ifndef J1939_H
#define J1939_H

#include <J1939/j1939_config.h>

#include "j1939_priority.h"
#include "j1939_types.h"
#include "j1939_bsp.h"
#include "j1939_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// must be called in main thread
int j1939_initialize(uint8_t index, const j1939_canlink *const canlink, const j1939_bsp *const bsp, const j1939_callbacks *const callbacks);
// must be called in logic thread, no thread safe
int j1939_configure(uint8_t index, uint8_t preferred_address, const j1939_CA_name *const CA_name);

uint8_t j1939_get_address(uint8_t index);

// must be called in logic thread, no thread safe
int j1939_claim_address(uint8_t index);

j1939_claim_status j1939_get_claim_status(uint8_t index);

int j1939_sendmsg_p(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority);
int j1939_sendmsg(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);
int j1939_sendraw(uint8_t index, const j1939_primitive *const primitive);

// int j1939_write_request(...);

// must be called in process thread or logic thread
int j1939_process(uint8_t index);

// must be called in receiving thread (IRQ) or logic thread
int j1939_handle_receiving(uint8_t index, const j1939_primitive *const frame, uint32_t time);
// must be called in transmiting thread (IRQ) or logic thread
int j1939_handle_transmiting(uint8_t index);


#ifdef __cplusplus
}
#endif

#endif /* J1939_H */
