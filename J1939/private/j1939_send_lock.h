#ifndef J1939_SEND_LOCK_H_
#define J1930_SEND_LOCK_H_

#include <errno.h>

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
    int error;

    CRITICAL_SECTION(phandle) {
retry:  error = __j1939_canlink_send(phandle, primitive);

        CRITICAL_SECTION_FLASH(phandle);

        if (error < 0) {
            if (error == -EINTR) {
                goto retry;
            } else if (error == -ENETDOWN) {
                CRITICAL_SECTION_BREAK(phandle);
            }

            error = j1939_tx_fifo_write(&phandle->tx_fifo, primitive);
            if (error < 0) {
                error = -ENOBUFS;
            }
        }
    }

    return error;
}


static inline int __j1939_send_control(j1939_phandle phandle, const j1939_primitive *const j1939_primitive) {
    if (IS_NORMAL_MODE(phandle)) {
        return __j1939_send_lock(phandle, j1939_primitive);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_SEND_LOCK_H_ */
