
#include "j1939_bsp.h"
#include "j1939_tp_mgr.h"
#include "j1939_tx_rx_fifo.h"
#include "j1939_private.h"


/**
 * @brief
 */
typedef enum {
    FATAL_ERROR = -1,
    NOT_STARTED = 0,
    INITIALIZED,
    ATEMPT_TO_CLAIM_ADDRESS,
    CANNOT_CLAIM_ADDRESS,
    ACTIVE,
    BUS_OFF,
} j1939_drv_state;


/**
 * @brief
 */
struct j1939_drv_ctx {
    j1939_drv_state state;

    j1939_CA_name CA_name;
    uint8_t address;
    uint8_t preferred_address;

    j1939_tp_mgr_ctx tp_mgr_ctx;
    j1939_rx_fifo rx_fifo;
    j1939_tx_fifo tx_fifo;

    j1939_callbacks callbacks;
};


/**
 * @brief
 */
static struct j1939_drv_ctx __j1939_ctx = {
    .state = NOT_STARTED,
    .address = J1939_NULL_ADDRESS,
    .preferred_address = J1939_NULL_ADDRESS,
};


/**
 * @brief
 * 
 * @param primitive
 * @return 
 */
int __j1939_send_lock(const j1939_primitive * const primitive) {
    int sts = 0;
    int level = j1939_bsp_lock();

    if (j1939_bsp_CAN_send(primitive) < 0) {
        sts = j1939_tx_fifo_write(&__j1939_ctx.tx_fifo, primitive);
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
int __j1939_receive_notify(uint32_t type, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz, const void *const payload) {
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
    rx_info.PGN = PGN;

    if (type == J1939_RX_INFO_TYPE_MULTIPACKET) {
        rx_info.payload_ptr = payload;
    } else {
        memcpy(rx_info.payload, payload, 8);
    }

    return j1939_rx_fifo_write(&__j1939_ctx.rx_fifo, &rx_info);
}


/**
 * @brief
 * 
 * @param address
 */
static inline void __send_claim_address(uint8_t address) {
    const j1939_primitive aclm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACLM, J1939_GENERIC_PRIORITY,
                              address, J1939_GLOBAL_ADDRESS, J1939_STD_PGN_ACLM_DLC, &__j1939_ctx.CA_name);
    __j1939_send_lock(&aclm_primitive);
}


/**
 * @brief
 * 
 * @param ack_type
 * @param gf
 * @param originator_sa
 * @param PGN
 */
static inline void __send_ACK(j1939_ack_control ack_type, uint8_t gf, uint8_t originator, PGN_format PGN) {
    const j1939_payload_ack ack_body = {
        .__reserved__ = { 0xFF, 0xFF},
        .control = ack_type,
        .group_function = gf,
        .address = originator,
        .PGN = PGN,
    };

    /*
     * The Acknowledgment PGN response uses a global destination address even though the PGN that
     * causes Acknowledgment was sent to a specific destination address.
     */
    const j1939_primitive ackm_primitive =
        j1939_primitive_build(J1939_STD_PGN_ACKM, J1939_GENERIC_PRIORITY,
                              __j1939_ctx.address, J1939_GLOBAL_ADDRESS,
                              J1939_STD_PGN_ACKM_DLC, &ack_body);
    __j1939_send_lock(&ackm_primitive);
}


/**
 * @brief
 */
void j1939_initialize(const j1939_callbacks * const callbacks) {
    memset(&__j1939_ctx, 0, sizeof (__j1939_ctx));

    __j1939_ctx.callbacks = *callbacks;

    j1939_rx_fifo_init(&__j1939_ctx.rx_fifo);
    j1939_tx_fifo_init(&__j1939_ctx.tx_fifo);
    j1939_tp_mgr_init(&__j1939_ctx.tp_mgr_ctx);

    __j1939_ctx.address = J1939_NULL_ADDRESS;
    __j1939_ctx.preferred_address = J1939_NULL_ADDRESS;

    __j1939_ctx.state = NOT_STARTED;
}


/**
 * @brief
 * 
 * @param drv_conf
 */
void j1939_configure(uint8_t preferred_address, const j1939_CA_name * const CA_name) {
    if (!CA_name || __j1939_ctx.state != NOT_STARTED)
        return;

    __j1939_ctx.CA_name = *CA_name;

    __j1939_ctx.address = J1939_NULL_ADDRESS;
    __j1939_ctx.preferred_address = preferred_address;

    __j1939_ctx.state = INITIALIZED;
}


/**
 * @brief
 * 
 * @return 
 */
uint8_t j1939_get_address(void) {
    return __j1939_ctx.address;
}


/**
 * @brief
 * 
 * @param address
 * 
 * @return 
 */
int j1939_claim_address(uint8_t address) {
    int level;

    if (address == 254)
        return -1;

    /* set a preferred address for communication with Network Management */
    if (address != J1939_GLOBAL_ADDRESS)
        __j1939_ctx.preferred_address = address;

    __j1939_ctx.state = ATEMPT_TO_CLAIM_ADDRESS;

    /* tell everybody for attempting to claim an address */
    __send_claim_address(__j1939_ctx.preferred_address);

    /* wait for 250 ms */
    j1939_bsp_mdelay(250);

    level = j1939_bsp_lock();

    if (__j1939_ctx.state == CANNOT_CLAIM_ADDRESS) {
        j1939_bsp_unlock(level);
        return -2;
    }

    __j1939_ctx.address = __j1939_ctx.preferred_address;
    __j1939_ctx.state = ACTIVE;

    j1939_bsp_unlock(level);

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
int j1939_sendmsg_p(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint8_t priority) {
    if (__j1939_ctx.state != ACTIVE)
        return -1;

    if (msg_sz <= 8) {        
        j1939_primitive primitive =
            j1939_primitive_build(PGN,
                                  priority,
                                  __j1939_ctx.address, dst_addr,
                                  msg_sz,
                                  payload);
        return __j1939_send_lock(&primitive);
    }

    return j1939_tp_mgr_open_tx_session(&__j1939_ctx.tp_mgr_ctx, PGN, dst_addr, msg_sz, payload);
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
int j1939_sendmsg(uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload) {
    return j1939_sendmsg_p(PGN, dst_addr, msg_sz, payload, J1939_GENERIC_PRIORITY);
}


/**
 * @brief
 * 
 * @param time_ms
 */
void j1939_tick(uint32_t t_delta) {
    static const int max_rx_per_tick = 10;
    int rx_upcount;
    int tx_downcount;

    j1939_tp_mgr_process(&__j1939_ctx.tp_mgr_ctx, t_delta);

    /* Transmition processing */
    for (tx_downcount = j1939_tx_fifo_size(&__j1939_ctx.tx_fifo); tx_downcount > 0; --tx_downcount) {
        int read_sts;
        j1939_primitive primitive;

        read_sts = j1939_tx_fifo_read(&__j1939_ctx.tx_fifo, &primitive);
        if (read_sts < 0)
            break;

        __j1939_send_lock(&primitive);
    }

    /* Receiving processing */
    for (rx_upcount = 0; rx_upcount < max_rx_per_tick; ++rx_upcount) {
        int read_sts;
        j1939_rx_info rx_info;

        read_sts = j1939_rx_fifo_read(&__j1939_ctx.rx_fifo, &rx_info);
        if (read_sts < 0)
            break;

        if (rx_info.type == J1939_RX_INFO_TYPE_FRAME) {
            if (__j1939_ctx.callbacks.rx_handler)
                __j1939_ctx.callbacks.rx_handler(rx_info.PGN, rx_info.src_addr, rx_info.msg_sz, &rx_info.payload[0]);
        } else if (rx_info.type == J1939_RX_INFO_TYPE_MULTIPACKET && rx_info.sid != 255) {
            if (__j1939_ctx.callbacks.rx_handler)
                __j1939_ctx.callbacks.rx_handler(rx_info.PGN, rx_info.src_addr, rx_info.msg_sz, rx_info.payload_ptr);
            j1939_tp_mgr_close_session(&__j1939_ctx.tp_mgr_ctx, rx_info.sid);
        }
    }
}


/**
 * @brief
 * 
 * @param frame
 * 
 * @return 
 */
static int __rx_handle_PGN_claim_address(const j1939_primitive * const frame) {
    /* PDU1 format */
    const int is_ACLM_PGN = j1939_PGN_code_get(frame->PGN) == J1939_STD_PGN_ACLM;
    const int is_our_addr =
        (frame->src_address != J1939_NULL_ADDRESS) &&
        (frame->src_address == __j1939_ctx.preferred_address);
    const j1939_CA_name *their_CA_name;
    int cannot_claim;
    j1939_drv_state new_state;
    int address;

    if (!is_ACLM_PGN || !is_our_addr || frame->dlc != J1939_STD_PGN_ACLM_DLC)
        return 0;

    their_CA_name = (j1939_CA_name*) (&frame->payload[0]);
    cannot_claim = (__j1939_ctx.CA_name.name > their_CA_name->name);

    if (cannot_claim) {
        new_state = CANNOT_CLAIM_ADDRESS;
        address = (J1939_NULL_ADDRESS); // "Cannot Claim Address" message specified in 4.2.2.2 of J1939-81

        /* reset TP MGR in prior of Cannot Claim Address */
        __j1939_ctx.tp_mgr_ctx.reset = 1;
    } else {
        new_state = ACTIVE;
        address = __j1939_ctx.preferred_address;
    }

    __j1939_ctx.state = new_state;
    __j1939_ctx.address = address;

    // FIXME: random send_claim_address on "Cannot Claim Address"
    __send_claim_address(address);

    return 1;
}


/**
 * @brief
 * 
 * @param frame
 * 
 * @return 
 */
static int __rx_handle_PGN_request(const j1939_primitive * const frame) {
    /* PDU1 format */
    const int is_RQST_PGN = j1939_PGN_code_get(frame->PGN) == J1939_STD_PGN_RQST;
    const int is_our_addr =
        (frame->PGN.dest_address != J1939_NULL_ADDRESS) &&
        ((frame->PGN.dest_address == __j1939_ctx.address) ||
        (frame->PGN.dest_address == J1939_GLOBAL_ADDRESS));
    const j1939_payload_request *request;

    if (!is_RQST_PGN || !is_our_addr || frame->dlc != J1939_STD_PGN_RQST_DLC)
        return 0;

    request = ((j1939_payload_request*) & frame->payload[0]);

    switch (j1939_PGN_code_get(request->PGN)) {
        case J1939_STD_PGN_ACLM:
            /*
             * The address claim message is an exception to the requirements on request messages specified in SAE J1939­21.
             * (SAE J1939­21 defines that a request message which is directed to a specific address be responded to with the
             * destination set to the requester.)
             */
            // FIXME: random send_claim_address if CANNOT_CLAIM_ADDRESS state has been set
            __send_claim_address((__j1939_ctx.state == INITIALIZED) ? __j1939_ctx.preferred_address : __j1939_ctx.address);
            break;

            /* XXX: we don't support any PGN yet */
        default:
            /*
             * A global request shall not be responded to with a NACK when a particular PGN is not supported by a node.
             */
            if (frame->PGN.dest_address != J1939_GLOBAL_ADDRESS)
                __send_ACK(J1939_ACK_NEGATIVE, 0xFF, frame->src_address, frame->PGN);
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
int j1939_process_rx(const j1939_primitive *const frame) {
    if (__j1939_ctx.state == NOT_STARTED || __j1939_ctx.state == BUS_OFF)
        return 0;

    /** Network Management (J1939-81) */

    /*
     * "Address Claim" message handling
     */
    if (__rx_handle_PGN_claim_address(frame))
        return 1;

    /* we are still attempting to claim an address, so dont handle any requests */
    if (__j1939_ctx.state == ATEMPT_TO_CLAIM_ADDRESS)
        return 0;

    /*
     * "PGN request" message handling
     */
    if (__rx_handle_PGN_request(frame))
        return 1;

    /*
     * Receiving messages on bus
     */

    if (__j1939_ctx.state != ACTIVE)
        return 0;

    if (j1939_tp_mgr_rx_handler(&__j1939_ctx.tp_mgr_ctx, frame))
        return 1;
    
    if (j1939_is_PDU1(frame->PGN) && (frame->PGN.dest_address != __j1939_ctx.address && frame->PGN.dest_address != J1939_GLOBAL_ADDRESS))
        return 0;

    /* append receiving data into fifo */
    __j1939_receive_notify(J1939_RX_INFO_TYPE_FRAME,
                           j1939_PGN_code_get(frame->PGN),
                           frame->src_address,
                           frame->dlc,
                           frame->payload);

    return 1;
}
