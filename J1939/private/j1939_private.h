/**
 * @file j1939_private.h
 * 
 * @brief
 */


#ifndef J1939_PRIVATE_H
#define J1939_PRIVATE_H

#include <string.h>

#include "j1939_private_types.h"

/* Optimization barrier */
#ifndef barrier
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief
 * 
 * @return 
 */
static inline uint8_t U8_MIN(uint8_t a, uint8_t b) {
    return (a < b) ? a : b;
}


/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @return uint16_t 
 */
static inline uint16_t U16_MIN(uint16_t a, uint16_t b) {
    return (a < b) ? a : b;
}


/**
 * Unpacks PGN data from payload to scalar.
 */
static inline uint32_t UNPACK_PGN(const uint8_t PGN[3]) {
    return ((uint32_t)PGN[2] << 16) | ((uint32_t)PGN[1] << 8) | (uint32_t)PGN[0];
}


/**
 * Packs PGN scalar into payload.
 */
static inline void PACK_PGN(uint32_t PGN, uint8_t OUT[3]) {
    OUT[0] = (uint8_t)PGN;
    OUT[1] = (uint8_t)(PGN >> 8);
    OUT[2] = (uint8_t)(PGN >> 16);
}


/**
 * @brief
 * 
 * @param PGN
 * @param priority
 * @param SA
 * @param DLC
 * @param payload
 * 
 * @return 
 */
static inline j1939_primitive j1939_primitive_build(uint32_t PGN, uint8_t priority, uint8_t SA, uint8_t DA, uint8_t DLC, const void *const payload) {
    struct j1939_primitive msg = {
        .PGN = PGN,
        .priority = (priority > J1939_MAX_PRIORITY) ? J1939_MAX_PRIORITY : priority,
        .dest_address = DA,
        .src_address = SA,
        .dlc = U8_MIN(DLC, 8),
    };
    
    /* PDU2 format is broadcast message */
    if (j1939_is_PDU2(msg.PGN)) {
        msg.dest_address = J1939_GLOBAL_ADDRESS;
    }
    
    memset(msg.payload, 0xFF, sizeof(msg.payload));
    memcpy(msg.payload, payload, msg.dlc);
    
    return msg;
}

extern j1939_handle __j1939_handles[];

static inline uint8_t __get_address(uint8_t index) {
    return __j1939_handles[index].address;
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_H */

