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


#define IS_NORMAL_MODE(phandle)         (!(phandle)->slave_mode)

/* Optimization barrier */
#ifndef barrier
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")
#endif

#ifndef CRITICAL_SECTION
#define CRITICAL_SECTION(phandle) \
    for (int __critical_section_level = __j1939_lock(phandle), __critical_section_exit = 1; \
        __critical_section_exit; \
        __critical_section_exit = __j1939_unlock(phandle, __critical_section_level))
#endif

#ifndef CRITICAL_SECTION_EXIT
#define CRITICAL_SECTION_EXIT(phandle, ret) \
    do { __critical_section_exit = __j1939_unlock(phandle, __critical_section_level); return (ret); } while (0)
#endif

#ifndef CRITICAL_SECTION_BREAK
#define CRITICAL_SECTION_BREAK(phandle) continue
#endif

#ifndef CRITICAL_SECTION_FLASH
#define CRITICAL_SECTION_FLASH(phandle) \
    do { \
        __j1939_unlock(phandle, __critical_section_level); \
        __critical_section_level = __j1939_lock(phandle); \
    } while (0)
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
 *
 */
static inline int __j1939_lock(j1939_phandle phandle) {
    if (phandle->bsp.lock) {
        return phandle->bsp.lock(phandle->index);
    }
    return 0;
}


/**
 *
 */
static inline int __j1939_unlock(j1939_phandle phandle, int level) {
    if (phandle->bsp.unlock) {
        phandle->bsp.unlock(phandle->index, level);
    }
    return 0;
}


/**
 *
 */
static inline uint32_t __j1939_gettime(j1939_phandle phandle) {
    if (phandle->bsp.gettime) {
        return phandle->bsp.gettime(phandle->index);
    }
    return -1U;
}


/**
 *
 */
static inline int __j1939_canlink_send(j1939_phandle phandle, const j1939_primitive *const primitive) {
    if (phandle->canlink.send) {
        return phandle->canlink.send(phandle->index, primitive);
    }
    return -1;
}


extern j1939_handle __j1939_handles[];

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_H */

