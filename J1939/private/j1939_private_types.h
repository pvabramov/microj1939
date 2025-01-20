#ifndef J1939_PRIVATE_TYPES_H_
#define J1939_PRIVATE_TYPES_H_

#include <J1939/j1939_types.h>
#include <J1939/j1939_bsp.h>

#include "j1939_tx_rx_fifo_types.h"
#include "j1939_tp_mgr_types.h"

#include "j1939_phandle.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 */
typedef enum j1939_state {
    FATAL_ERROR = -1,
    NOT_STARTED = 0,
    INITIALIZED,
    ATEMPT_TO_CLAIM_ADDRESS,
    CANNOT_CLAIM_ADDRESS,
    ACTIVE,
    BUS_OFF,
} j1939_state;


/**
 * @brief
 */
typedef struct j1939_handle {
    int index;

    int slave_mode;

    int oneshot;
    uint32_t last_time;

    j1939_canlink canlink;
    j1939_bsp bsp;

    volatile j1939_claim_status claim_status;
    volatile j1939_state state;
    volatile int already_tx;
    volatile int already_rx;

    volatile uint8_t address;
    volatile uint8_t preferred_address;

    j1939_CA_name CA_name;

    j1939_tp_mgr_ctx tp_mgr_ctx;
    j1939_rx_fifo rx_fifo;
    j1939_tx_fifo tx_fifo;
    j1939_rx_tx_error_fifo rx_error_fifo;
    j1939_rx_tx_error_fifo tx_error_fifo;

    j1939_callback_claim_handler claim_handler;
    j1939_callback_claim_handler cannot_claim_handler;

    j1939_callbacks callbacks;
    int preidle_timer;
    int claim_timer;
    int random_timer;
} j1939_handle;


#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_TYPES_H_ */
