/**
 * @file j1939_private.h
 * 
 * @brief
 */


#ifndef J1939_PRIVATE_H
#define J1939_PRIVATE_H

#include <string.h>

#include <J1939/j1939_utils.h>
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


extern j1939_handle __j1939_handles[];

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_H */

