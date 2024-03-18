/**
 * @file j1939_tx_rx_fifo.c
 *
 * @brief
 */

#include <J1939/j1939_config.h>

#include <string.h>

#include <J1939/j1939_bsp.h>
#include <J1939/private/j1939_tx_rx_fifo.h>
#include <J1939/private/ring_buff.h>


/**
 * @brief
 *
 * @param fifo
 */
void j1939_rx_fifo_init(j1939_rx_fifo *const fifo) {
    memset(fifo, 0, sizeof(j1939_rx_fifo));
}


/**
 * @brief
 *
 * @param fifo
 *
 * @return
 */
unsigned j1939_rx_fifo_size(const j1939_rx_fifo *const fifo) {
    return ring_data_avail(J1939_RX_FIFO_SZ, fifo->head, fifo->tail);
}


/**
 * @brief
 *
 * @param fifo
 * @param rx_info
 *
 * @return
 */
int j1939_rx_fifo_write(j1939_rx_fifo *const fifo, const j1939_rx_info *const rx_info) {
    const unsigned avail = ring_space_avail(J1939_RX_FIFO_SZ, fifo->head, fifo->tail);

    if (avail == 0) {
        return -1;
    }

    do {
        int level = j1939_bsp_lock();

        /* write */
        fifo->items[fifo->head] = *rx_info;
        /* mark written */
        fifo->head = ring_wrap(J1939_RX_FIFO_SZ, fifo->head + 1);

        j1939_bsp_unlock(level);
    } while(0);

    return 0;
}


/**
 * @brief
 *
 * @param fifo
 * @param read_container
 *
 * @return
 */
int j1939_rx_fifo_read(j1939_rx_fifo *const fifo, j1939_rx_info *const rx_info) {
    const unsigned data_avail = ring_data_avail(J1939_RX_FIFO_SZ, fifo->head, fifo->tail);

    if (data_avail == 0) {
        return -1;
    }

    do {
        int level = j1939_bsp_lock();

        /* read */
        *rx_info = fifo->items[fifo->tail];
        /* mark read */
        fifo->tail = ring_wrap(J1939_RX_FIFO_SZ, fifo->tail + 1);

        j1939_bsp_unlock(level);
    } while (0);

    return 0;
}


/**
 * @brief
 *
 * @param fifo
 */
void j1939_tx_fifo_init(j1939_tx_fifo *const fifo) {
    memset(fifo, 0, sizeof(j1939_tx_fifo));
}


/**
 * @brief
 *
 * @param fifo
 *
 * @return
 */
unsigned j1939_tx_fifo_size(const j1939_tx_fifo *const fifo) {
    return ring_data_avail(J1939_TX_FIFO_SZ, fifo->head, fifo->tail);
}


/**
 * @brief
 *
 * @param fifo
 * @param primitive
 *
 * @return
 */
int j1939_tx_fifo_write(j1939_tx_fifo *const fifo, const j1939_primitive *const primitive) {
    const unsigned avail = ring_space_avail(J1939_TX_FIFO_SZ, fifo->head, fifo->tail);

    if (avail == 0) {
        return -1;
    }

    do {
        int level = j1939_bsp_lock();

        /* write */
        fifo->items[fifo->head] = *primitive;
        /* mark written */
        fifo->head = ring_wrap(J1939_TX_FIFO_SZ, fifo->head + 1);

        j1939_bsp_unlock(level);
    } while (0);

    return 0;
}


/**
 * @brief
 *
 * @param fifo
 * @param primitive
 *
 * @return
 */
int j1939_tx_fifo_read(j1939_tx_fifo *const fifo, j1939_primitive *const primitive) {
    const unsigned data_avail = ring_data_avail(J1939_TX_FIFO_SZ, fifo->head, fifo->tail);

    if (data_avail == 0) {
        return -1;
    }

    do {
        int level = j1939_bsp_lock();

        /* read */
        *primitive = fifo->items[fifo->tail];
        /* mark read */
        fifo->tail = ring_wrap(J1939_TX_FIFO_SZ, fifo->tail + 1);

        j1939_bsp_unlock(level);
    } while (0);

    return 0;
}


/**
 * @brief
 *
 * @param fifo
 */
void j1939_rx_tx_error_fifo_init(j1939_rx_tx_error_fifo *const fifo) {
    memset(fifo, 0, sizeof(j1939_rx_tx_error_fifo));
}


/**
 * @brief
 *
 * @param fifo
 *
 * @return
 */
unsigned j1939_rx_tx_error_fifo_size(const j1939_rx_tx_error_fifo *const fifo) {
    return ring_data_avail(J1939_RX_TX_ERROR_FIFO_SZ, fifo->head, fifo->tail);
}


/**
 * @brief
 *
 * @param fifo
 * @param info
 *
 * @return
 */
int j1939_rx_tx_error_fifo_write(j1939_rx_tx_error_fifo *const fifo, const j1939_rx_tx_error_info *const info) {
    const unsigned avail = ring_space_avail(J1939_RX_TX_ERROR_FIFO_SZ, fifo->head, fifo->tail);

    if (avail == 0)
        return -1;

    do {
        int level = j1939_bsp_lock();

        /* write */
        fifo->items[fifo->head] = *info;
        /* mark written */
        fifo->head = ring_wrap(J1939_RX_TX_ERROR_FIFO_SZ, fifo->head + 1);

        j1939_bsp_unlock(level);
    } while (0);

    return 0;
}


/**
 * @brief
 *
 * @param fifo
 * @param info
 *
 * @return
 */
int j1939_rx_tx_error_fifo_read(j1939_rx_tx_error_fifo *const fifo, j1939_rx_tx_error_info *const info) {
    const unsigned data_avail = ring_data_avail(J1939_RX_TX_ERROR_FIFO_SZ, fifo->head, fifo->tail);

    if (data_avail == 0)
        return -1;

    do {
        int level = j1939_bsp_lock();

        /* read */
        *info = fifo->items[fifo->tail];
        /* mark read */
        fifo->tail = ring_wrap(J1939_RX_TX_ERROR_FIFO_SZ, fifo->tail + 1);

        j1939_bsp_unlock(level);
    } while (0);

    return 0;
}
