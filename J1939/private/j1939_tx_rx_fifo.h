/**
 * @file j1939_tx_rx_queue.h
 * 
 * @brief
 */


#ifndef J1939_TX_RX_QUEUE_H
#define J1939_TX_RX_QUEUE_H

#include "j1939_private_types.h"


#ifdef __cplusplus
extern "C" {
#endif

void j1939_rx_fifo_init(j1939_rx_fifo *const fifo);
unsigned j1939_rx_fifo_size(const j1939_rx_fifo *const fifo);
int j1939_rx_fifo_write(j1939_rx_fifo *const fifo, const j1939_rx_info *const rx_info);
int j1939_rx_fifo_read(j1939_rx_fifo *const fifo, j1939_rx_info *const rx_info);

void j1939_tx_fifo_init(j1939_tx_fifo *const fifo);
unsigned j1939_tx_fifo_size(const j1939_tx_fifo *const fifo);
int j1939_tx_fifo_write(j1939_tx_fifo *const fifo, const j1939_primitive *const primitive);
int j1939_tx_fifo_read(j1939_tx_fifo *const fifo, j1939_primitive *const primitive);

void j1939_rx_tx_error_fifo_init(j1939_rx_tx_error_fifo *const fifo);
unsigned j1939_rx_tx_error_fifo_size(const j1939_rx_tx_error_fifo *const fifo);
int j1939_rx_tx_error_fifo_write(j1939_rx_tx_error_fifo *const fifo, const j1939_rx_tx_error_info *const info);
int j1939_rx_tx_error_fifo_read(j1939_rx_tx_error_fifo *const fifo, j1939_rx_tx_error_info *const info);

#ifdef __cplusplus
}
#endif

#endif /* J1939_TX_RX_QUEUE_H */

