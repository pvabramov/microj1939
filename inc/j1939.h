/**
 * @file j1939.h
 * 
 * @brief
 */
#ifndef J1939_H
#define J1939_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
    
    
#define J1939_MULTIPACKET_DATA_SZ       7
#define J1939_MAX_DATA_SZ               (J1939_MULTIPACKET_DATA_SZ * 255)

#define J1939_CONTROL_PRIORITY          3
#define J1939_GENERIC_PRIORITY          6
#define J1939_TP_PRIORITY               7

#define J1939_NULL_ADDRESS              254U
#define J1939_GLOBAL_ADDRESS            255U
    
#define J1939_STD_PGN_ACKM              59392U
#define J1939_STD_PGN_ACKM_DLC          8

#define J1939_STD_PGN_RQST              59904U
#define J1939_STD_PGN_RQST_DLC          3

#define J1939_STD_PGN_TPDT              60160U
#define J1939_STD_PGN_TPDT_DLC          8

#define J1939_STD_PGN_TPCM              60416U
#define J1939_STD_PGN_TPCM_DLC          8

#define J1939_STD_PGN_ACLM              60928U
#define J1939_STD_PGN_ACLM_DLC          8
    
#define J1939_STD_PGN_PROPA             61184U
// no DLC

    
/**
 * @brief
 */
typedef union {
    struct {
        uint32_t identity_number : 21;
        uint32_t manufacturer_code : 11;
        uint8_t ECU_instance : 3;
        uint8_t function_instance : 5;
        uint8_t function;
        uint8_t __reserved__ : 1;
        uint8_t vehicle_system : 7;
        uint8_t vehicle_instance : 4;
        uint8_t industry_group : 3;
        uint8_t arbitrary_address_capable : 1;
    };
    uint64_t name;
} j1939_CA_name;


/**
 * @brief J1939 PGN format
 */
typedef union PGN_format {
    uint32_t value;
    struct {
        // [0]
        union {
            uint8_t pdu_specific; // PS
            uint8_t dest_address; // DA
        };
        // [1]
        uint8_t pdu_format; // PF
        // [2]
        uint8_t dp : 1;
        uint8_t edp : 1;
        uint8_t __zeros__ : 6;
        // [3]
        uint8_t __padding__;
    };
} PGN_format;


#define TREAT_AS_PGN(x) ((PGN_format)(x))


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU1(PGN_format PGN) {
    return PGN.pdu_format < 240;
}


/**
 * @brief
 * 
 * @param PGN
 * 
 * @return 
 */
static inline int j1939_is_PDU2(PGN_format PGN) {
    return PGN.pdu_format >= 240;
}


/**
 * @brief
 * 
 * @param PGN
 */
static inline void j1939_PGN_code_set(PGN_format *const PGN, uint32_t value) {
    PGN->value = value;
    PGN->__zeros__ = 0;
    PGN->__padding__ = 0;
}


/**
 * @brief
 * 
 * @param PGN
 * @return 
 */
static inline uint32_t j1939_PGN_code_get(PGN_format PGN) {
    PGN.__zeros__ = 0;
    PGN.__padding__ = 0;
    
    // The PDU Format is 239 or less and the PDU Specific field is set to 0
    if (j1939_is_PDU1(PGN))
        PGN.pdu_specific = 0;
    
    return PGN.value;
}

///
typedef void (*j1939_callback_rx_handler)(uint32_t PGN, uint8_t src_address, uint16_t msg_sz, const void *const payload);


/**
 * @brief
 */
typedef struct j1939_callbacks {
    j1939_callback_rx_handler rx_handler;
} j1939_callbacks;


void j1939_initialize(const j1939_callbacks *const callbacks);
void j1939_configure(uint8_t preferred_address, const j1939_CA_name *const CA_name);

uint8_t j1939_get_address(void);
int j1939_claim_address(uint8_t address);

int j1939_sendmsg_p(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority);
int j1939_sendmsg(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);

// int j1939_write_request(...);

void j1939_tick(uint32_t t_delta);


#ifdef __cplusplus
}
#endif

#endif /* J1939_MESSAGE_H */

