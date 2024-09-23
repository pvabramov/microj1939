/**
 * @file j1939_core.c
 *
 * @brief
 */

#include <J1939/j1939_config.h>

#include <J1939/j1939_bsp.h>
#include <J1939/private/j1939_tp_mgr.h>
#include <J1939/private/j1939_tx_rx_fifo.h>
#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_rand.h>


#define PREIDLE_TIMEOUT 2000

#define CLAIM_RANDOM    jrandr(3, 153)


/**
 * @brief
 */
j1939_handle __j1939_handles[J1939_MAX_HANDLES_NUM];


/**
 * @brief
 *
 * @param primitive
 * @return
 */
int __j1939_send_lock(uint8_t index, const j1939_primitive * const primitive) {
    int sts = 0;
    int level = j1939_bsp_lock();

    if (j1939_bsp_CAN_send(index, primitive) < 0) {
        sts = j1939_tx_fifo_write(&__j1939_handles[index].tx_fifo, primitive);
    }

    j1939_bsp_unlock(level);

    return sts;
}


/**
 * @brief
 *
 * @param type
 * @param PGN
 * @param src_addr
 * @param msg_sz
 * @param payload
 * @return
 */
int __j1939_receive_notify(uint8_t index, uint32_t type, uint32_t PGN, uint8_t src_addr, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint32_t time) {
    j1939_rx_info rx_info;

    if (type & J1939_RX_INFO_TYPE_MULTIPACKET) {
        rx_info.sid = type & 0xFF;
        type = J1939_RX_INFO_TYPE_MULTIPACKET;
    } else {
        rx_info.sid = 255;
    }

    rx_info.type = type;
    rx_info.msg_sz = msg_sz;
    rx_info.src_addr = src_addr;
    rx_info.dst_addr = dst_addr;
    rx_info.PGN = PGN;
    rx_info.time = time;

    if (type == J1939_RX_INFO_TYPE_MULTIPACKET) {
        rx_info.payload_ptr = payload;
    } else {
        memcpy(rx_info.payload, payload, U16_MIN(msg_sz, 8));
    }

    return j1939_rx_fifo_write(&__j1939_handles[index].rx_fifo, &rx_info);
}


/**
 * @brief
 * 
 * @param error
 * @param PGN
 * @param dst_addr
 * @param msg_sz
 */
int __j1939_tx_error_notify(uint8_t index, j1939_rx_tx_errno error, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz) {
    if (__j1939_handles[index].callbacks.tx_error_handler) {
        const j1939_rx_tx_error_info info = {
            .error = error,
            .PGN = PGN,
            .addr = dst_addr,
            .msg_sz = msg_sz
        };
        return j1939_rx_tx_error_fifo_write(&__j1939_handles[index].tx_error_fifo, &info);
    }
    return 0;
}


/**
 * @brief 
 * 
 * @param error 
 * @param PGN 
 * @param src_addr 
 * @param msg_sz 
 * @return int 
 */
int __j1939_rx_error_notify(uint8_t index, j1939_rx_tx_errno error, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz) {
    if (__j1939_handles[index].callbacks.rx_error_handler) {
        const j1939_rx_tx_error_info info = {
            .error = error,
            .PGN = PGN,
            .addr = src_addr,
            .msg_sz = msg_sz
        };
        return j1939_rx_tx_error_fifo_write(&__j1939_handles[index].rx_error_fifo, &info);
    }
    return 0;
}


/**
 * @brief
 *
 * @param address
 */
static inline void __send_claim_address(uint8_t index, uint8_t address) {
    const j1939_primitive aclm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACLM, J1939_GENERIC_PRIORITY,
                              address, J1939_GLOBAL_ADDRESS, J1939_STD_PGN_ACLM_DLC, &__j1939_handles[index].CA_name);
    __j1939_send_lock(index, &aclm_primitive);
}


/**
 * @brief
 *
 * @param ack_type
 * @param gf
 * @param originator_sa
 * @param PGN
 */
static inline void __send_ACK(uint8_t index, j1939_ack_control ack_type, uint8_t gf, uint8_t originator, uint32_t PGN) {
    j1939_payload_ack ack_body = {
        .__reserved__ = { 0xFF, 0xFF},
        .control = ack_type,
        .group_function = gf,
        .address = originator
    };

    PACK_PGN(PGN, ack_body.PGN);

    /*
     * The Acknowledgment PGN response uses a global destination address even though the PGN that
     * causes Acknowledgment was sent to a specific destination address.
     */
    const j1939_primitive ackm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACKM, J1939_GENERIC_PRIORITY,
                              __j1939_handles[index].address, J1939_GLOBAL_ADDRESS,
                              J1939_STD_PGN_ACKM_DLC, &ack_body);
    __j1939_send_lock(index, &ackm_primitive);
}


/**
 * @brief
 */
void j1939_initialize(uint8_t index, const j1939_callbacks * const callbacks) {
    j1939_handle *const handle = &__j1939_handles[index];

    memset(handle, 0, sizeof(j1939_handle));

    handle->state = NOT_STARTED;

    barrier();

    handle->callbacks = *callbacks;

    j1939_rx_tx_error_fifo_init(&handle->tx_error_fifo);
    j1939_rx_tx_error_fifo_init(&handle->rx_error_fifo);
    j1939_rx_fifo_init(&handle->rx_fifo);
    j1939_tx_fifo_init(&handle->tx_fifo);
    j1939_tp_mgr_init(&handle->tp_mgr_ctx);

    handle->address = J1939_NULL_ADDRESS;
    handle->preferred_address = J1939_NULL_ADDRESS;

    handle->already_rx = 0;
    handle->already_tx = 0;

    handle->preidle_timer = PREIDLE_TIMEOUT;
}


/**
 * @brief
 *
 * @param drv_conf
 */
void j1939_configure(uint8_t index, uint8_t preferred_address, const j1939_CA_name * const CA_name) {
    j1939_handle *const handle = &__j1939_handles[index];

    if (!CA_name || (handle->state != NOT_STARTED && handle->state != CANNOT_CLAIM_ADDRESS)) {
        return;
    }

    handle->CA_name = *CA_name;

    handle->address = J1939_NULL_ADDRESS;
    handle->preferred_address = preferred_address;

    barrier();

    handle->state = INITIALIZED;
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
    j1939_handle *const handle = &__j1939_handles[index];

    if (handle->state != INITIALIZED) {
        return -2;
    }

    /* run callbacks only one time */
    handle->claim_handler = handle->callbacks.claim_handler;
    handle->cannot_claim_handler = handle->callbacks.cannot_claim_handler;

    handle->claim_timer = 250; /* claim address timeout */

    barrier();

    handle->state = ATEMPT_TO_CLAIM_ADDRESS;

    /* tell everybody for attempting to claim an address */
    __send_claim_address(index, handle->preferred_address);

    return 0;
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

    if (msg_sz <= 8) {
        j1939_primitive primitive =
            j1939_primitive_build(PGN,
                                  priority,
                                  handle->address, dst_addr,
                                  msg_sz,
                                  payload);
        return __j1939_send_lock(index, &primitive);
    }

    return j1939_tp_mgr_open_tx_session(index, &handle->tp_mgr_ctx, PGN, dst_addr, msg_sz, payload);
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
    return __j1939_send_lock(index, primitive);
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

    if (__j1939_send_lock(index, &primitive) < 0) {
        return 0;
    }

    return 1;
}


/**
 * @brief
 * 
 * @return
 */
static int j1939_process_tx(uint8_t index) {
    j1939_handle *const handle = &__j1939_handles[index];
    int is_active = 0;
    int tx_downcount;

    if (handle->state != ACTIVE) {
        return 0;
    }

    handle->already_tx = 1;

    barrier();

    for (tx_downcount = j1939_tx_fifo_size(&handle->tx_fifo); tx_downcount > 0; --tx_downcount) {
        j1939_primitive primitive;

        if (j1939_tx_fifo_read(&handle->tx_fifo, &primitive) < 0) {
            break;
        }

        is_active = 1;

        if (__j1939_send_lock(index, &primitive) < 0) {
            break;
        }
    }

    barrier();

    handle->already_tx = 0;

    return is_active;
}


/**
 * @brief
 * 
 * @return
 */
static int j1939_process_rx(uint8_t index) {
    j1939_handle *const handle = &__j1939_handles[index];
    int is_active = 0;
    int rx_upcount;
    int max_rx_per_tick;

    if (handle->state != ACTIVE) {
        return 0;
    }

    handle->already_rx = 1;

    barrier();

    max_rx_per_tick = j1939_rx_fifo_size(&handle->rx_fifo);

    for (rx_upcount = 0; rx_upcount < max_rx_per_tick; ++rx_upcount) {
        j1939_rx_info rx_info;

        if (j1939_rx_fifo_read(&handle->rx_fifo, &rx_info) < 0) {
            break;
        }

        is_active = 1;

        if (rx_info.type == J1939_RX_INFO_TYPE_FRAME) {

            if (handle->callbacks.rx_handler) {
                handle->callbacks.rx_handler(index, rx_info.PGN, rx_info.src_addr, rx_info.dst_addr, rx_info.msg_sz, &rx_info.payload[0], rx_info.time);
            }

        } else if (rx_info.type == J1939_RX_INFO_TYPE_REQUEST) {
            j1939_request_status status;

            if (handle->callbacks.request_handler) {
                status = handle->callbacks.request_handler(index, rx_info.PGN, rx_info.src_addr, rx_info.dst_addr, rx_info.time);
            } else {
                status = J1939_REQ_NOT_SUPPORTED;
            }

            if ((status != J1939_REQ_HANDLED) && (rx_info.dst_addr != J1939_GLOBAL_ADDRESS)) {
                __send_ACK(index, (j1939_ack_control)status, 0xFF, rx_info.src_addr, rx_info.PGN);
            }

        } else if (rx_info.type == J1939_RX_INFO_TYPE_MULTIPACKET && rx_info.sid != 255) {

            if (handle->callbacks.rx_handler) {
                handle->callbacks.rx_handler(index, rx_info.PGN, rx_info.src_addr, rx_info.dst_addr, rx_info.msg_sz, rx_info.payload_ptr, rx_info.time);
            }

            j1939_tp_mgr_close_session(index, &handle->tp_mgr_ctx, rx_info.sid);
        }
    }

    barrier();

    handle->already_rx = 0;

    return is_active;
}


/**
 * @brief
 */
static int j1939_notify_errors(uint8_t index, j1939_rx_tx_error_fifo *const fifo, j1939_callback_rx_tx_error_handler handler) {
    j1939_handle *const handle = &__j1939_handles[index];
    int is_active = 0;
    int upcount;
    int max_per_tick;

    if (handle->state != ACTIVE) {
        return 0;
    }

    max_per_tick = j1939_rx_tx_error_fifo_size(fifo);

    for (upcount = 0; upcount < max_per_tick; ++upcount) {
        j1939_rx_tx_error_info info;

        if (j1939_rx_tx_error_fifo_read(fifo, &info) < 0) {
            break;
        }

        if (handler) {
            handler(index, info.error, info.PGN, info.addr, info.msg_sz);
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
    j1939_handle *const handle = &__j1939_handles[index];
    uint32_t t_delta;
    int activities;

    if (!handle->oneshot) {
        handle->last_time = the_time;
        handle->oneshot = 1;
    }

    t_delta = the_time - handle->last_time;

    if (0 == t_delta) {
        return -1;
    }

    handle->last_time = the_time;
    activities = 0;

    j1939_state state = handle->state;

    if (state == ATEMPT_TO_CLAIM_ADDRESS) {
        if (handle->claim_timer > 0) {
            activities = 1;
            handle->claim_timer -= t_delta;
            if (handle->claim_timer <= 0) {
                handle->address = handle->preferred_address;

                barrier();

                handle->state = ACTIVE;

                if (handle->claim_handler != NULL) {
                    handle->claim_handler(index, handle->address, &handle->CA_name);
                    handle->claim_handler = NULL;
                }
            }
        }
    } else if (state == CANNOT_CLAIM_ADDRESS) {
        int handle_status;

        if (handle->cannot_claim_handler != NULL) {
            handle_status = handle->cannot_claim_handler(index, handle->preferred_address, &handle->CA_name);
            handle->cannot_claim_handler = NULL;
        } else {
            handle_status = 0;
        }

        if (handle_status) {
            // cannot claim handler has been handled so dont send "Cannot Claim Address" message
            handle->random_timer = 0;
        }

        handle->preferred_address = J1939_NULL_ADDRESS;

        if (handle->random_timer > 0) {
            activities = 1;
            handle->random_timer -= t_delta;
            if (handle->random_timer <= 0) {
                // "Cannot Claim Address" message specified in 4.2.2.2 of J1939-81
                __send_claim_address(index, J1939_NULL_ADDRESS);
            }
        }
    } else if (state == ACTIVE) {
        /* Error notifications */
        activities += j1939_notify_errors(index, &handle->rx_error_fifo, handle->callbacks.rx_error_handler);
        activities += j1939_notify_errors(index, &handle->tx_error_fifo, handle->callbacks.tx_error_handler);

        /* TP management processing */
        activities += j1939_tp_mgr_process(index, &handle->tp_mgr_ctx, t_delta);

        /* Transmition processing */
        activities += j1939_process_tx(index);

        /* Receiving processing */
        activities += j1939_process_rx(index);
    }

    if (activities > 0) {
        handle->preidle_timer = PREIDLE_TIMEOUT;
    } else {
        handle->preidle_timer -= t_delta;
        if (handle->preidle_timer < 0) {
            handle->preidle_timer = 0;
        }
    }

    return (handle->preidle_timer > 0);
}


/**
 * @brief
 * 
 * @return
 */
int j1939_process(uint8_t index) {
    const uint32_t the_time = j1939_bsp_get_time();
    return __j1939_process(index, the_time);
}


/**
 * @brief
 *
 * @param frame
 *
 * @return
 */
static int __rx_handle_PGN_claim_address(uint8_t index, const j1939_primitive * const frame, uint32_t time) {
    j1939_handle *const handle = &__j1939_handles[index];

    (void)time;

    if (handle->state == ATEMPT_TO_CLAIM_ADDRESS || handle->state == ACTIVE) {
        /* PDU1 format */
        const int is_ACLM_PGN = frame->PGN == J1939_STD_PGN_ACLM;
        const int is_our_addr =
            (frame->src_address != J1939_NULL_ADDRESS) &&
            (frame->src_address == handle->preferred_address);
        const j1939_CA_name *their_CA_name;
        int cannot_claim;

        if (!is_ACLM_PGN || !is_our_addr || frame->dlc != J1939_STD_PGN_ACLM_DLC) {
            return 0;
        }

        their_CA_name = (j1939_CA_name*) (&frame->payload[0]);
        cannot_claim = (handle->CA_name.name >= their_CA_name->name);

        /*
        SAE J1939-81-2017

        4.5.3.3 Response to Address Claims of Own Address

        A CA shall retransmit an address claim if it receives an address claim with a source address that matches its own and if
        its own NAME is of a lower value (higher priority) than the NAME in the claim it received. If the CA's NAME is of a higher
        value (lower priority) than the NAME in the claim it received, the CA shall not continue to use that address. (It may send a
        Cannot Claim Address message or it may attempt to claim a different address.)
        */

        if (cannot_claim) {
            handle->address = J1939_NULL_ADDRESS;
            // FIXME: random send_claim_address on "Cannot Claim Address"
            handle->random_timer = CLAIM_RANDOM;
            /* reset TP MGR in prior of Cannot Claim Address */
            handle->tp_mgr_ctx.reset = 1;

            barrier();

            handle->state = CANNOT_CLAIM_ADDRESS;
        } else {
            // do the reclaimation of the address
            __send_claim_address(index, handle->address);
        }

        return 1;
    }

    return 0;
}


/**
 * @brief
 *
 * @param frame
 *
 * @return
 */
static int __rx_handle_PGN_request(uint8_t index, const j1939_primitive * const frame, uint32_t time) {
    j1939_handle *const handle = &__j1939_handles[index];
    /* PDU1 format */
    const int is_RQST_PGN = frame->PGN == J1939_STD_PGN_RQST;
    const uint8_t dst_addr = frame->dest_address;
    const int is_our_addr = (dst_addr != J1939_NULL_ADDRESS) && ((dst_addr == handle->address) || (dst_addr == J1939_GLOBAL_ADDRESS));
    const j1939_payload_request *request;
    uint32_t requested_PGN;

    if (!is_RQST_PGN || !is_our_addr || frame->dlc != J1939_STD_PGN_RQST_DLC) {
        return 0;
    }

    request = ((j1939_payload_request*) & frame->payload[0]);
    requested_PGN = UNPACK_PGN(request->PGN);

    switch (requested_PGN) {
        case J1939_STD_PGN_ACLM:
            if (dst_addr == J1939_GLOBAL_ADDRESS) {
                const uint8_t preferred_address = handle->preferred_address;
                /*
                    SAE J1939-81-2017

                    4.5.3.1 Response to a Request for Address Claimed Sent to the Global Address

                    A CA shall always respond to a Request for Address Claimed directed to the global address with either an Address
                    Claimed message or if the CA has not been successful in claiming an address, a Cannot Claim Address message.
                */
                if (handle->state == CANNOT_CLAIM_ADDRESS) {
                    // FIXME: random send_claim_address if CANNOT_CLAIM_ADDRESS state has been set
                    handle->random_timer = CLAIM_RANDOM;
                } else if (preferred_address != J1939_NULL_ADDRESS) {
                    __send_claim_address(index, preferred_address);
                }
            } else if (handle->state == ACTIVE) {
                /*
                    SAE J1939-81-2017

                    4.5.3.2 Response to a Request for Address Claimed Sent to a Specific Address

                    A CA shall always respond to a Request for Address Claimed where the destination address of the request is the CA's
                    address. The response to the request, the Address Claimed message, shall be sent to the global address (255).
                */
                __send_claim_address(index, handle->address);
            }
            break;

        default:
            if (handle->state == ACTIVE) {
                if (handle->callbacks.request_handler != NULL) {
                __j1939_receive_notify(index, J1939_RX_INFO_TYPE_REQUEST,
                        requested_PGN,
                        frame->src_address,
                        dst_addr,
                        frame->dlc,
                        request,
                        time);
                } else {
                    /*
                    * A global request shall not be responded to with a NACK when a particular PGN is not supported by a node.
                    */
                    if (dst_addr != J1939_GLOBAL_ADDRESS) {
                        __send_ACK(index, J1939_ACK_NEGATIVE, 0xFF, frame->src_address, requested_PGN);
                    }
                }
            }
            break;
    }

    return 1;
}


/**
 * @brief
 *
 * @param frame
 *
 * @return
 */
int j1939_handle_receiving(uint8_t index, const j1939_primitive *const frame, uint32_t time) {
    j1939_handle *const handle = &__j1939_handles[index];
    uint8_t dst_addr;

    const j1939_state state = handle->state;

    if (state == NOT_STARTED || state == BUS_OFF) {
        return 0;
    }

    /** Network Management (J1939-81) */

    /*
     * "Address Claim" message handling
     */
    if (__rx_handle_PGN_claim_address(index, frame, time)) {
        return 1;
    }

    /*
     * "PGN request" message handling
     */
    if (__rx_handle_PGN_request(index, frame, time)) {
        return 1;
    }

    /*
     * Receiving messages on bus
     */

    if (state != ACTIVE) {
        return 0;
    }

    if (j1939_tp_mgr_rx_handler(index, &handle->tp_mgr_ctx, frame, time)) {
        return 1;
    }

    dst_addr = frame->dest_address;

    if ((dst_addr != handle->address) && (dst_addr != J1939_GLOBAL_ADDRESS)) {
        return 0;
    }

    /* append receiving data into fifo */
    __j1939_receive_notify(index,
                           J1939_RX_INFO_TYPE_FRAME,
                           frame->PGN,
                           frame->src_address,
                           dst_addr,
                           frame->dlc,
                           frame->payload,
                           time);

    return 1;
}
