/**
 * @file j1939_core.c
 *
 * @brief
 */

#include <stdlib.h>
#include <errno.h>

#include <J1939/j1939_config.h>

#include <J1939/j1939.h>
#include <J1939/j1939_bsp.h>

#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_tx_rx_fifo.h>
#include <J1939/private/j1939_notify.h>
#include <J1939/private/j1939_network.h>
#include <J1939/private/j1939_tp_mgr.h>
#include <J1939/private/j1939_send_lock.h>


/**
 * @brief
 */
j1939_handle __j1939_handles[J1939_MAX_HANDLES_NUM];


/**
 * @brief
 */
int j1939_initialize(uint8_t index, const j1939_canlink *const canlink, const j1939_bsp *const bsp, const j1939_callbacks *const callbacks) {

    if (index >= J1939_MAX_HANDLES_NUM) {
        return -EINVAL;
    }

    if (canlink == NULL) {
        return -EINVAL;
    }

    if (canlink->send == NULL) {
        return -EINVAL;
    }

    if (bsp == NULL) {
        return -EINVAL;
    }

    if (bsp->gettime == NULL) {
        return -EINVAL;
    }

    if (callbacks == NULL) {
        return -EINVAL;
    }

    j1939_handle *const handle = &__j1939_handles[index];

    memset(handle, 0, sizeof(j1939_handle));

    handle->index = index;

    handle->claim_status = UNKNOWN;
    handle->state = NOT_STARTED;

    barrier();

    handle->canlink     = *canlink;
    handle->bsp         = *bsp;
    handle->callbacks   = *callbacks;

    j1939_rx_tx_error_fifo_init(&handle->tx_error_fifo);
    j1939_rx_tx_error_fifo_init(&handle->rx_error_fifo);
    j1939_rx_fifo_init(&handle->rx_fifo);
    j1939_tx_fifo_init(&handle->tx_fifo);
    j1939_tp_mgr_init(&handle->tp_mgr_ctx);

    handle->address = J1939_NULL_ADDRESS;
    handle->preferred_address = J1939_NULL_ADDRESS;

    handle->already_rx = 0;
    handle->already_tx = 0;

    handle->preidle_timer = J1939_PREIDLE_TIMER;

    return 0;
}


/**
 * @brief
 *
 * @param drv_conf
 */
int j1939_configure(uint8_t index, uint8_t preferred_address, const j1939_CA_name * const CA_name) {
    j1939_handle *const handle = &__j1939_handles[index];

    CRITICAL_SECTION(handle) {
        if (j1939_network_setup(handle, preferred_address, CA_name) < 0) {
            CRITICAL_SECTION_EXIT(handle, -1);
        }
    }

    return 0;
}


/**
 * @brief
 *
 * @return
 */
uint8_t j1939_get_address(uint8_t index) {
    return __j1939_handles[index].address;
}


/**
 * @brief
 *
 * @param address
 *
 * @return
 */
int j1939_claim_address(uint8_t index) {
    j1939_handle *handle = &__j1939_handles[index];
    return j1939_network_claim_address(handle);
}


/**
 * @brief
 *
 * @param PGN
 * @param dst_addr
 * @param msg_sz
 * @param payload
 * @param priority
 *
 * @return
 */
int j1939_sendmsg_p(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority) {
    j1939_handle *const handle = &__j1939_handles[index];

    if (handle->state != ACTIVE) {
        return -1;
    }

    if (msg_sz <= J1939_MAX_DL) {
        j1939_primitive primitive =
            j1939_primitive_build(PGN,
                                  priority,
                                  handle->address, dst_addr,
                                  msg_sz,
                                  payload);
        return __j1939_send_lock(handle, &primitive);
    }

    return j1939_tp_mgr_open_tx_session(handle, &handle->tp_mgr_ctx, PGN, dst_addr, msg_sz, payload);
}


/**
 * @brief
 *
 * @param PGN
 * @param dst_addr
 * @param msg_sz
 * @param payload
 *
 * @return
 */
int j1939_sendmsg(uint8_t index, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload) {
    return j1939_sendmsg_p(index, PGN, dst_addr, msg_sz, payload, J1939_GENERIC_PRIORITY);
}


/**
 * @brief
 *
 * @param primitive
 *
 * @return
 */
int j1939_sendraw(uint8_t index, const j1939_primitive *const primitive) {
    j1939_handle *handle = &__j1939_handles[index];
    return __j1939_send_lock(handle, primitive);
}


/**
 * @brief
 */
int j1939_handle_transmiting(uint8_t index) {
    j1939_handle *const handle = &__j1939_handles[index];
    j1939_primitive primitive;

    if (handle->already_tx) {
        return 0;
    }

    if (j1939_tx_fifo_read(&handle->tx_fifo, &primitive) < 0) {
        return 0;
    }

    if (__j1939_send_lock(handle, &primitive) < 0) {
        return 0;
    }

    return 1;
}


/**
 * @brief
 * 
 * @return
 */
static int __process_tx(j1939_phandle phandle) {
    int is_active = 0;
    int tx_downcount;

    if (phandle->state != ACTIVE) {
        return 0;
    }

    phandle->already_tx = 1;

    barrier();

    for (tx_downcount = j1939_tx_fifo_size(&phandle->tx_fifo); tx_downcount > 0; --tx_downcount) {
        j1939_primitive primitive;

        if (j1939_tx_fifo_read(&phandle->tx_fifo, &primitive) < 0) {
            break;
        }

        is_active = 1;

        if (__j1939_send_lock(phandle, &primitive) < 0) {
            break;
        }
    }

    barrier();

    phandle->already_tx = 0;

    return is_active;
}


/**
 * @brief
 * 
 * @return
 */
static int __process_rx(j1939_phandle phandle) {
    int is_active = 0;
    int rx_upcount;
    int max_rx_per_tick;

    if (phandle->state != ACTIVE) {
        return 0;
    }

    phandle->already_rx = 1;

    barrier();

    max_rx_per_tick = j1939_rx_fifo_size(&phandle->rx_fifo);

    for (rx_upcount = 0; rx_upcount < max_rx_per_tick; ++rx_upcount) {
        j1939_rx_info rx_info;

        if (j1939_rx_fifo_read(&phandle->rx_fifo, &rx_info) < 0) {
            break;
        }

        is_active = 1;

        if (!j1939_network_rx_process(phandle, &rx_info)) {
            if (phandle->callbacks.rx_handler) {
                if (rx_info.type == J1939_RX_INFO_TYPE_FRAME) {
                    phandle->callbacks.rx_handler(
                        phandle->index,
                        rx_info.PGN,
                        rx_info.src_addr,
                        rx_info.dst_addr,
                        rx_info.msg_sz,
                        &rx_info.payload[0],
                        rx_info.time
                    );
                } else if (rx_info.type == J1939_RX_INFO_TYPE_MULTIPACKET && rx_info.sid != 255) {
                    phandle->callbacks.rx_handler(
                        phandle->index,
                        rx_info.PGN,
                        rx_info.src_addr,
                        rx_info.dst_addr,
                        rx_info.msg_sz,
                        rx_info.payload_ptr,
                        rx_info.time
                    );
                    j1939_tp_mgr_close_session(phandle, &phandle->tp_mgr_ctx, rx_info.sid);
                }
            }
        }
    }

    barrier();

    phandle->already_rx = 0;

    return is_active;
}


/**
 * @brief
 */
static int __process_errors(j1939_phandle phandle, j1939_rx_tx_error_fifo *const fifo, j1939_callback_rx_tx_error_handler handler) {
    int is_active = 0;
    int upcount;
    int max_per_tick;

    if (phandle->state != ACTIVE) {
        return 0;
    }

    max_per_tick = j1939_rx_tx_error_fifo_size(fifo);

    for (upcount = 0; upcount < max_per_tick; ++upcount) {
        j1939_rx_tx_error_info info;

        if (j1939_rx_tx_error_fifo_read(fifo, &info) < 0) {
            break;
        }

        if (handler) {
            handler(phandle->index, info.error, info.PGN, info.addr, info.msg_sz);
        }

        is_active = 1;
    }

    return is_active;
}


/**
 * @brief
 * 
 * @param the_time
 * 
 * @return
 */
static inline int __j1939_process(uint8_t index, uint32_t the_time) {
    j1939_phandle phandle = &__j1939_handles[index];
    uint32_t t_delta;
    int activities;

    if (!phandle->oneshot) {
        phandle->oneshot = 1;
        phandle->last_time = the_time;
        t_delta = 0;
    } else {
        t_delta = the_time - phandle->last_time;
        phandle->last_time = the_time;

        if (0 == t_delta) {
            return -1;
        }
    }

    j1939_state state = phandle->state;

    /* Network Management (J1939-81) */
    activities = j1939_network_process(phandle, t_delta);

    if (state == ACTIVE) {
        /* Error notifications */
        activities += __process_errors(phandle, &phandle->rx_error_fifo, phandle->callbacks.rx_error_handler);
        activities += __process_errors(phandle, &phandle->tx_error_fifo, phandle->callbacks.tx_error_handler);

        /* TP management processing */
        activities += j1939_tp_mgr_process(phandle, &phandle->tp_mgr_ctx, t_delta);

        /* Transmition processing */
        activities += __process_tx(phandle);

        /* Receiving processing */
        activities += __process_rx(phandle);
    }

    if (activities > 0) {
        phandle->preidle_timer = J1939_PREIDLE_TIMER;
    } else {
        phandle->preidle_timer -= t_delta;
        if (phandle->preidle_timer < 0) {
            phandle->preidle_timer = 0;
        }
    }

    return (phandle->preidle_timer > 0);
}


j1939_claim_status j1939_get_claim_status(uint8_t index) {
    j1939_phandle phandle = &__j1939_handles[index];
    return phandle->claim_status;
}


/**
 * @brief
 * 
 * @return
 */
int j1939_process(uint8_t index) {
    j1939_phandle phandle = &__j1939_handles[index];
    return __j1939_process(
        index,
        __j1939_gettime(phandle)
    );
}


/**
 * @brief
 *
 * @param frame
 *
 * @return
 */
int j1939_handle_receiving(uint8_t index, const j1939_primitive *const frame, uint32_t time) {
    j1939_phandle phandle = &__j1939_handles[index];
    uint8_t dst_addr;

    const j1939_state state = phandle->state;

    if (state == NOT_STARTED || state == BUS_OFF) {
        return 0;
    }

    /* Network Management (J1939-81) */
    if (j1939_network_rx_handler(phandle, frame, time)) {
        return 1;
    }

    /*
     * Receiving messages on bus
     */

    if (state != ACTIVE) {
        return 0;
    }

    if (j1939_tp_mgr_rx_handler(phandle, &phandle->tp_mgr_ctx, frame, time)) {
        return 1;
    }

    dst_addr = frame->dest_address;

    if ((dst_addr != phandle->address) && (dst_addr != J1939_GLOBAL_ADDRESS)) {
        return 0;
    }

    /* append receiving data into fifo */
    __j1939_receive_notify(phandle,
                           J1939_RX_INFO_TYPE_FRAME,
                           frame->PGN,
                           frame->src_address,
                           dst_addr,
                           frame->dlc,
                           frame->payload,
                           time);

    return 1;
}
