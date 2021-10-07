/**
 * @file j1939_tx_rx_queue.h
 * 
 * @brief
 */


#ifndef J1939_TX_RX_QUEUE_H
#define J1939_TX_RX_QUEUE_H

#include "j1939_private.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief
 */
typedef struct j1939_rx_fifo {
    unsigned head;
    unsigned tail;
    j1939_rx_info items[J1939_RX_FIFO_SZ];
} j1939_rx_fifo;


void j1939_rx_fifo_init(j1939_rx_fifo *const fifo);
unsigned j1939_rx_fifo_size(const j1939_rx_fifo *const fifo);
int j1939_rx_fifo_write(j1939_rx_fifo *const fifo, const j1939_rx_info *const rx_info);
int j1939_rx_fifo_read(j1939_rx_fifo *const fifo, j1939_rx_info *const rx_info);


/**
 * @brief
 */
typedef struct j1939_tx_fifo {
    unsigned head;
    unsigned tail;
    j1939_primitive items[J1939_TX_FIFO_SZ];
} j1939_tx_fifo;


void j1939_tx_fifo_init(j1939_tx_fifo *const fifo);
unsigned j1939_tx_fifo_size(const j1939_tx_fifo *const fifo);
int j1939_tx_fifo_write(j1939_tx_fifo *const fifo, const j1939_primitive *const primitive);
int j1939_tx_fifo_read(j1939_tx_fifo *const fifo, j1939_primitive *const primitive);


/**
 * @brief
 */
typedef struct j1939_rx_tx_error_fifo {
    unsigned head;
    unsigned tail;
    j1939_rx_tx_error_info items[J1939_RX_TX_ERROR_FIFO_SZ];
} j1939_rx_tx_error_fifo;


void j1939_rx_tx_error_fifo_init(j1939_rx_tx_error_fifo *const fifo);
unsigned j1939_rx_tx_error_fifo_size(const j1939_rx_tx_error_fifo *const fifo);
int j1939_rx_tx_error_fifo_write(j1939_rx_tx_error_fifo *const fifo, const j1939_rx_tx_error_info *const info);
int j1939_rx_tx_error_fifo_read(j1939_rx_tx_error_fifo *const fifo, j1939_rx_tx_error_info *const info);

#ifdef __cplusplus
}
#endif

#endif /* J1939_TX_RX_QUEUE_H */

