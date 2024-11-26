#include <J1939/j1939_network.h>

#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_rand.h>
#include <J1939/private/j1939_notify.h>

#include <J1939/private/j1939_network_msg.h>
#include <J1939/private/j1939_network.h>


#define INITIAL_CLAIM_ADDRESS_TIMEOUT   (J1939_CLAIM_ADDRESS_TIMEOUT + 1)
#define CLAIM_RANDOM                    jrandr(3, 153)


static int __rx_handle_PGN_claim_address(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time);
static int __rx_handle_PGN_request(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time);


int j1939_network_setup(j1939_phandle phandle, uint8_t preferred_address, const j1939_CA_name *const name) {
    const j1939_state state = phandle->state;

    if (!name || (state != NOT_STARTED && state != CANNOT_CLAIM_ADDRESS)) {
        return -1;
    }

    phandle->claim_status = CLAIM_ADDRESS_UNKNOWN;

    barrier();

    phandle->CA_name = *name;

    phandle->address = J1939_NULL_ADDRESS;
    phandle->preferred_address = preferred_address;

    barrier();

    phandle->state = INITIALIZED;

    return 0;
}


int j1939_network_claim_address(j1939_phandle phandle) {

    if (phandle->state != INITIALIZED) {
        return -1;
    }

    phandle->claim_status = CLAIM_ADDRESS_PROCESSING;

    barrier();

    /* run callbacks only one time */
    phandle->claim_handler = phandle->callbacks.claim_handler;
    phandle->cannot_claim_handler = phandle->callbacks.cannot_claim_handler;

    phandle->claim_timer = INITIAL_CLAIM_ADDRESS_TIMEOUT; /* claim address timeout */

    barrier();

    phandle->state = ATEMPT_TO_CLAIM_ADDRESS;

    return 0;
}


int j1939_network_rx_handler(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time) {

    if (__rx_handle_PGN_claim_address(phandle, frame, time)) {
        return 1;
    }

    if (__rx_handle_PGN_request(phandle, frame, time)) {
        return 1;
    }

    return 0;
}


int j1939_network_process(j1939_phandle phandle, uint32_t t_delta) {
    const j1939_state state = phandle->state;
    int is_active = 0;

    if (state == ATEMPT_TO_CLAIM_ADDRESS) {
        if (phandle->claim_timer > 0) {

            is_active = 1;

            if (phandle->claim_timer == INITIAL_CLAIM_ADDRESS_TIMEOUT) {
                phandle->claim_timer = J1939_CLAIM_ADDRESS_TIMEOUT;
                /* tell everybody for attempting to claim an address */
                __send_Claim_Address(phandle, phandle->preferred_address);
            }

            phandle->claim_timer -= t_delta;

            if (phandle->claim_timer <= 0) {
                phandle->address = phandle->preferred_address;

                barrier();

                phandle->claim_status = CLAIM_ADDRESS_SUCCESS;
                phandle->state = ACTIVE;

                barrier();

                if (phandle->claim_handler != NULL) {
                    phandle->claim_handler(phandle->index, phandle->address, &phandle->CA_name);
                    phandle->claim_handler = NULL;
                }
            }
        }
    } else if (state == CANNOT_CLAIM_ADDRESS) {
        int handle_status;

        if (phandle->cannot_claim_handler != NULL) {
            is_active = 1;

            handle_status = phandle->cannot_claim_handler(phandle->index, phandle->preferred_address, &phandle->CA_name);
            phandle->cannot_claim_handler = NULL;
        } else {
            handle_status = 0;
        }

        if (handle_status) {
            // cannot claim handler has been handled so dont send "Cannot Claim Address" message
            phandle->random_timer = 0;
        }

        phandle->preferred_address = J1939_NULL_ADDRESS;

        if (phandle->random_timer > 0) {
            is_active = 1;

            phandle->random_timer -= t_delta;

            if (phandle->random_timer <= 0) {
                phandle->claim_status = CLAIM_ADDRESS_FAILED;

                barrier();

                // "Cannot Claim Address" message specified in 4.2.2.2 of J1939-81
                __send_Claim_Address(phandle, J1939_NULL_ADDRESS);
            }
        }
    }

    return is_active;
}


int j1939_network_rx_process(j1939_phandle phandle, const j1939_rx_info *const rx_info) {

    if (rx_info->type != J1939_RX_INFO_TYPE_REQUEST) {
        return 0;
    }

    j1939_request_status status;

    if (phandle->callbacks.request_handler) {
        status = phandle->callbacks.request_handler(phandle->index, rx_info->PGN, rx_info->src_addr, rx_info->dst_addr, rx_info->time);
    } else {
        status = J1939_REQ_NOT_SUPPORTED;
    }

    if ((status != J1939_REQ_HANDLED) && (rx_info->dst_addr != J1939_GLOBAL_ADDRESS)) {
        __send_ACK(phandle, (j1939_ack_control)status, 0xFF, rx_info->src_addr, rx_info->PGN);
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
static int __rx_handle_PGN_claim_address(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time) {
    const j1939_state state = phandle->state;
    (void)time;

    if (state == ATEMPT_TO_CLAIM_ADDRESS || state == ACTIVE) {
        /* PDU1 format */
        const int is_ACLM_PGN = frame->PGN == J1939_STD_PGN_ACLM;
        const int is_our_addr =
            (frame->src_address != J1939_NULL_ADDRESS) &&
            (frame->src_address == phandle->preferred_address);

        j1939_CA_name their_CA_name;
        int cannot_claim;

        if (!is_ACLM_PGN || !is_our_addr || frame->dlc != J1939_STD_PGN_ACLM_DLC) {
            return 0;
        }

        their_CA_name.hname[0] = *((uint32_t*) (&frame->payload[0]));
        their_CA_name.hname[1] = *((uint32_t*) (&frame->payload[4]));

        cannot_claim = (phandle->CA_name.name >= their_CA_name.name);
        /*
        SAE J1939-81-2017

        4.5.3.3 Response to Address Claims of Own Address

        A CA shall retransmit an address claim if it receives an address claim with a source address that matches its own and if
        its own NAME is of a lower value (higher priority) than the NAME in the claim it received. If the CA's NAME is of a higher
        value (lower priority) than the NAME in the claim it received, the CA shall not continue to use that address. (It may send a
        Cannot Claim Address message or it may attempt to claim a different address.)
        */

        if (cannot_claim) {
            phandle->claim_status = CLAIM_ADDRESS_PROCESSING;
            phandle->address = J1939_NULL_ADDRESS;
            phandle->random_timer = CLAIM_RANDOM;
            /* reset TP MGR in prior of Cannot Claim Address */
            phandle->tp_mgr_ctx.reset = 1;

            barrier();

            phandle->state = CANNOT_CLAIM_ADDRESS;
        } else {
            // do the reclaimation of the address
            __send_Claim_Address(phandle, phandle->address);
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
static int __rx_handle_PGN_request(j1939_phandle phandle, const j1939_primitive * const frame, uint32_t time) {
    /* PDU1 format */
    const int is_RQST_PGN = frame->PGN == J1939_STD_PGN_RQST;
    const uint8_t dst_addr = frame->dest_address;
    const int is_our_addr = (dst_addr != J1939_NULL_ADDRESS) && ((dst_addr == phandle->address) || (dst_addr == J1939_GLOBAL_ADDRESS));
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
                const uint8_t preferred_address = phandle->preferred_address;
                /*
                    SAE J1939-81-2017

                    4.5.3.1 Response to a Request for Address Claimed Sent to the Global Address

                    A CA shall always respond to a Request for Address Claimed directed to the global address with either an Address
                    Claimed message or if the CA has not been successful in claiming an address, a Cannot Claim Address message.
                */
                if (phandle->state == CANNOT_CLAIM_ADDRESS) {
                    phandle->random_timer = CLAIM_RANDOM;
                } else if (preferred_address != J1939_NULL_ADDRESS) {
                    __send_Claim_Address(phandle, preferred_address);
                }
            } else if (phandle->state == ACTIVE) {
                /*
                    SAE J1939-81-2017

                    4.5.3.2 Response to a Request for Address Claimed Sent to a Specific Address

                    A CA shall always respond to a Request for Address Claimed where the destination address of the request is the CA's
                    address. The response to the request, the Address Claimed message, shall be sent to the global address (255).
                */
                __send_Claim_Address(phandle, phandle->address);
            }
            break;

        default:
            if (phandle->state == ACTIVE) {
                if (phandle->callbacks.request_handler != NULL) {
                    __j1939_receive_notify(phandle, J1939_RX_INFO_TYPE_REQUEST,
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
                        __send_ACK(phandle, J1939_ACK_NEGATIVE, 0xFF, frame->src_address, requested_PGN);
                    }
                }
            }
            break;
    }

    return 1;
}
