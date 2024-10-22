#ifndef J1939_BSP_H
#define J1939_BSP_H

#include "j1939_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct j1939_canlink {
    int (*send)(uint8_t, const j1939_primitive *const);
} j1939_canlink;


typedef struct j1939_bsp {
    int (*lock)(uint8_t);
    void (*unlock)(uint8_t, int);
    uint32_t (*gettime)(uint8_t);
} j1939_bsp;


#ifdef __cplusplus
}
#endif

#endif /* J1939_BSP_H */
