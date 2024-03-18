/**
 * @file j1939_tp_mgr.h
 * 
 * @brief
 */


#ifndef J1939_TP_MGR_H
#define J1939_TP_MGR_H

#include "j1939_private_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void j1939_tp_mgr_init(j1939_tp_mgr_ctx *const tp_mgr_ctx);
int j1939_tp_mgr_rx_handler(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, const j1939_primitive *const frame, uint32_t time);
int j1939_tp_mgr_process(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t t_delta);

int j1939_tp_mgr_open_tx_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload);
int j1939_tp_mgr_close_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid);


#ifdef __cplusplus
}
#endif

#endif /* J1939_TP_MGR_H */

