/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>

#include <J1939/j1939.h>


static const j1939_CA_name j1939_CA_test_controller = {
    .identity_number = 0,
    .manufacturer_code = 0,
    .ECU_instance = 0,
    .function = 0,
    .function_instance = 0,
    .vehicle_instance = 0,
    .vehicle_system = 0,
    .industry_group = 0,
    .arbitrary_address_capable = 0,
};


/**
 * @brief
 *
 * @param PGN
 * @param src_address
 * @param msg_sz
 * @param payload
 *
 * @return
 */
void __j1939_rx_handler(uint32_t PGN, uint8_t src_addr, uint16_t msg_sz, const void *const payload) {

}


static const j1939_callbacks callbacks = {
    .rx_handler = __j1939_rx_handler,
};


/*
 *
 */
int main(int argc, char** argv) {
    int n_ticks = 10;
    uint32_t n_tick = 0;
    int status = 0;

    j1939_initialize(&callbacks);
    j1939_configure(0x70, &j1939_CA_test_controller);

    status = j1939_claim_address(J1939_GLOBAL_ADDRESS /* means preffered */);
    if (status < 0) {
        // TODO: Cannot claim address
        return (EXIT_FAILURE);
    }

    while (n_ticks-- > 0) {
        const uint32_t t_now = n_tick;

        j1939_process(t_now);

        n_tick++;
    }

    return (EXIT_SUCCESS);
}

