/**
 * @file j1939_bsp.h
 * 
 * @brief
 */


#ifndef J1939_BSP_H
#define J1939_BSP_H

#include <J1939/j1939.h>


#ifdef __cplusplus
extern "C" {
#endif


extern int j1939_bsp_lock(void);
extern void j1939_bsp_unlock(int level);

extern uint32_t j1939_bsp_get_time();
extern void j1939_bsp_mdelay(uint32_t ms);

extern int j1939_bsp_CAN_send(uint8_t index, const j1939_primitive *const primitive);


#ifdef __cplusplus
}
#endif

#endif /* J1939_BSP_H */
