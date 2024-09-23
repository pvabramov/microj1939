#ifndef J1939_SEND_LOCK_H_
#define J1930_SEND_LOCK_H_

#include <J1939/private/j1939_tx_rx_fifo.h>
#include <J1939/private/j1939_private.h>

#include <J1939/j1939_types.h>
#include <J1939/j1939_bsp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * @param primitive
 * @return
 */
static inline int __j1939_send_lock(j1939_phandle phandle, const j1939_primitive * const primitive) {
    int sts = 0;
    int level = j1939_bsp_lock();

    if (j1939_bsp_CAN_send(phandle->index, primitive) < 0) {
        sts = j1939_tx_fifo_write(&phandle->tx_fifo, primitive);
    }

    j1939_bsp_unlock(level);

    return sts;
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_SEND_LOCK_H_ */