
#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_tx_rx_fifo.h>

#include <J1939/private/j1939_notify.h>

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
int __j1939_receive_notify(j1939_phandle phandle, uint32_t type, uint32_t PGN, uint8_t src_addr, uint8_t dst_addr, uint16_t msg_sz, const void *const payload, uint32_t time) {
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

    return j1939_rx_fifo_write(&phandle->rx_fifo, &rx_info);
}


/**
 * @brief
 * 
 * @param error
 * @param PGN
 * @param dst_addr
 * @param msg_sz
 */
int __j1939_tx_error_notify(j1939_phandle phandle, j1939_rx_tx_errno error, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz) {
    if (phandle->callbacks.tx_error_handler) {
        const j1939_rx_tx_error_info info = {
            .error = error,
            .PGN = PGN,
            .addr = dst_addr,
            .msg_sz = msg_sz
        };
        return j1939_rx_tx_error_fifo_write(&phandle->tx_error_fifo, &info);
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
int __j1939_rx_error_notify(j1939_phandle phandle, j1939_rx_tx_errno error, uint32_t PGN, uint8_t src_addr, uint16_t msg_sz) {
    if (phandle->callbacks.rx_error_handler) {
        const j1939_rx_tx_error_info info = {
            .error = error,
            .PGN = PGN,
            .addr = src_addr,
            .msg_sz = msg_sz
        };
        return j1939_rx_tx_error_fifo_write(&phandle->rx_error_fifo, &info);
    }
    return 0;
}
