
#include <stdio.h>
#include <stdlib.h>

#include <J1939/j1939.h>


#define CAN_INDEX   0


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
void __j1939_rx_handler(uint8_t index, uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time) {
    // Here do the logic on receiving
    if (index == CAN_INDEX) {
        // process receiving for CAN_INDEX = 0
    }
}


int __j1939_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const NAME) {
    is_ready = 1;
    return 0;
}

int __j1939_cannot_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const NAME) {
    is_ready = 0;

    if (address == 0x70U) {
        /* claim another address */
        j1939_configure(CAN_INDEX, 0x71, &j1939_CA_test_controller);
        j1939_claim_address(CAN_INDEX);

        return 1; /* dont' send Cannot Claim Address */
    }

    return 0;
}


static const j1939_callbacks callbacks = {
    .rx_handler = __j1939_rx_handler,
    .claim_handler = __j1939_claim_handler,
    .cannot_claim_handler = __j1939_cannot_claim_handler,
    // add some handlers here, see j1939_callbacks
};


volatile int exit = 0;
volatile int is_ready = 0;

/*
 *
 */
int main(int argc, char** argv) {
    j1939_initialize(CAN_INDEX, &callbacks);

    // specify the address and NAME
    j1939_configure(CAN_INDEX, 0x70, &j1939_CA_test_controller);
    // do the request to claim address
    j1939_claim_address(CAN_INDEX);

    while (!exit) {
        // call the process on every tick
        j1939_process(CAN_INDEX);

        if (is_ready) {
            // the address has been claimed, now we may to send messages
        }

        // wait for some time ...
    }

    return (EXIT_SUCCESS);
}
