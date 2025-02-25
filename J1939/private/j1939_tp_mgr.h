/**
 * @file j1939_tp_mgr.h
 *
 * @brief
 */


#ifndef J1939_TP_MGR_H
#define J1939_TP_MGR_H

#include <J1939/j1939_types.h>

#include "j1939_phandle.h"
#include "j1939_tp_mgr_types.h"


#undef J1939_TP_MIN_MSG_SZ
#define J1939_TP_MIN_MSG_SZ        9


#ifdef __cplusplus
extern "C" {
#endif

void j1939_tp_mgr_init(j1939_tp_mgr_ctx *const tp_mgr_ctx);

int j1939_tp_mgr_rx_handler(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, const j1939_primitive *const frame, uint32_t time);
int j1939_tp_mgr_process(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t t_delta);

int j1939_tp_mgr_open_tx_session(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, void **payload, int start_tx);
int j1939_tp_mgr_start_tx_session(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, int sid);
int j1939_tp_mgr_close_session(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid);
int j1939_tp_mgr_close_session_with_error(j1939_phandle phandle, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid, j1939_rx_tx_errno error);


#ifdef __cplusplus
}
#endif

#endif /* J1939_TP_MGR_H */

