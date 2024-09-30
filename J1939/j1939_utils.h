#ifndef J1939_UTILS_H_
#define J1939_UTILS_H_

#include <stdint.h>
#include <string.h>

#include "j1939_std_addr.h"
#include "j1939_types.h"


#define J1939_PGN_PDU1_MASK             0x3FF00U    // PDU1 mask
#define J1939_PGN_PDU2_MASK             0x3FFFFU    // PDU2 mask

#define J1939_PGN_PDU_FORMAT_MASK       0x0FF00U    // PDU format mask
#define J1939_PGN_PDU_FORMAT_SEP        0x0F000U    // PDU format separator

#define J1939_CANID_PRIORITY_MASK       0x1C000000U // CanId[Priority] mask
#define J1939_CANID_PGN_PDU2_MASK       0x03FFFF00U // CanId[PGN(PDU2)] mask
#define J1939_CANID_PGN_PDU1_MASK       0x03FF0000U // CanId[PGN(PDU1)] mask
#define J1939_CANID_PGN_PF_MASK         0x00FF0000U // CanId[PGN(PF)] mask
#define J1939_CANID_PGN_PF_PDU2         0x00F00000U // PGN[PF(PDU2)]
#define J1939_CANID_PGN_PSDA_MASK       0x0000FF00U // CanId[PGN(PS|DA)] mask
#define J1939_CANID_SA_MASK             0x000000FFU // CanId[SA] mask

#define J1939_CANID_FLAGS_MASK          0xE0000000U // CanId[31,30,29]


#ifdef __cplusplus
extern "C" {
#endif


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

/**
 *
 */
static inline uint8_t j1939_canid_get_SA(uint32_t canid) {
    return (uint8_t)canid;
}

/**
 *
 */
static inline uint32_t j1939_canid_get_PGN(uint32_t canid) {
    uint32_t PGN = (canid >> 8) & J1939_PGN_PDU2_MASK;
    return j1939_is_PDU1(PGN) ? PGN & J1939_PGN_PDU1_MASK : PGN;
}

/**
 *
 */
static inline uint8_t j1939_canid_get_DA(uint32_t canid) {
    uint32_t PGN = (canid >> 8) & J1939_PGN_PDU2_MASK;
    return j1939_is_PDU1(PGN) ? (uint8_t) PGN : (uint8_t) J1939_GLOBAL_ADDRESS;
}

/**
 *
 */
static inline uint8_t j1939_canid_get_Priority(uint32_t canid) {
    return (canid & J1939_CANID_PRIORITY_MASK) >> 26;
}

/**
 *
 */
static inline uint32_t j1939_canid_build_noflags(uint8_t priority, uint32_t PGN, uint8_t dest, uint8_t src) {
    if (j1939_is_PDU1(PGN)) {
        PGN &= J1939_PGN_PDU1_MASK;
        PGN |= dest;
    }
    return ((uint32_t)priority) << 26 | (PGN << 8) | (uint32_t)src;
}

/**
 *
 */
static inline uint32_t j1939_canid_build(uint8_t priority, uint32_t PGN, uint8_t dest, uint8_t src, uint32_t flags) {
    return j1939_canid_build_noflags(priority, PGN, dest, src) | (flags & J1939_CANID_FLAGS_MASK);
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
static inline j1939_primitive j1939_primitive_build(uint32_t PGN, uint8_t priority, uint8_t SA, uint8_t DA, uint8_t DLC, const volatile void *payload) {
    struct j1939_primitive msg = {
        .PGN            = PGN,
        .priority       = priority > J1939_MAX_PRIORITY ? J1939_MAX_PRIORITY : priority,
        .dest_address   = DA,
        .src_address    = SA,
        .dlc            = DLC > J1939_MAX_DL ? J1939_MAX_DL : DLC,
    };

    /* PDU2 format is broadcast message */
    if (j1939_is_PDU2(PGN)) {
        msg.dest_address = J1939_GLOBAL_ADDRESS;
    }

    memcpy(&msg.payload[0], (const void*)payload, msg.dlc);
    memset(&msg.payload[msg.dlc], J1939_PADDING_DATA, J1939_MAX_DL - msg.dlc);

    return msg;
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_UTILS_H_ */
