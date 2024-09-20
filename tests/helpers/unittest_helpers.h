#ifndef UNITTEST_HELPERS_H_
#define UNITTEST_HELPERS_H_

#include <stdint.h>
#include <J1939/j1939.h>
#include <J1939/j1939_bsp.h>

#define CAN_INDEX       0


typedef struct unittest_j1939_rx_msg {
    uint8_t index;
    uint32_t PGN;
    uint8_t SA;
    uint8_t DA;
    uint16_t len;
    uint8_t data[J1939_MAX_DATA_SZ];
    uint32_t time;
} unittest_j1939_rx_msg;


typedef struct unittest_j1939_claim_msg {
    uint8_t index;
    uint8_t address;
    j1939_CA_name name;
} unittest_j1939_claim_msg;


typedef void (*callback_on_delay)(void);


int unittest_helpers_setup(uint8_t index);
void unittest_helpers_cleanup(void);

int unittest_get_output(j1939_primitive *f);

int unittest_post_input(uint8_t index, uint32_t PGN, uint8_t DA, uint8_t SA, uint8_t len, ...);
int unittest_get_input(unittest_j1939_rx_msg *m);

uint32_t unittest_get_time(void);
void unittest_add_time(uint32_t time);

int unittest_get_claim(unittest_j1939_claim_msg *msg);

void unittest_set_cannot_claim_status(int status);
int unittest_get_cannot_claim(unittest_j1939_claim_msg *msg);

void unittest_set_callback_on_delay(callback_on_delay cb);

#endif /* UNITTEST_HELPERS_H_ */
