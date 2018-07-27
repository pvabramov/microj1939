/**
 * @file j1939_tx_rx_queue.h
 * @brief
 */


#ifndef J1939_TX_RX_QUEUE_H
#define J1939_TX_RX_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "j1939_private.h"
  

///
#define J1939_RX_FIFO_SZ   32


/**
 * @brief
 */
typedef struct j1939_rx_fifo {
    unsigned head;
    unsigned tail;
    j1939_rx_info items[J1939_RX_FIFO_SZ];
} j1939_rx_fifo;


void j1939_rx_fifo_init(j1939_rx_fifo *const fifo);
int j1939_rx_fifo_write(j1939_rx_fifo *const fifo, const j1939_rx_info *const rx_info);
int j1939_rx_fifo_read(j1939_rx_fifo *const fifo, j1939_rx_info *const rx_info);


///
#define J1939_TX_FIFO_SZ    32


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


#ifdef __cplusplus
}
#endif

#endif /* J1939_TX_RX_QUEUE_H */

