#ifndef UNITTEST_HELPERS_H_
#define UNITTEST_HELPERS_H_

#include <stdint.h>
#include <J1939/j1939.h>


typedef struct unittest_j1939_rx_msg {
    uint32_t PGN;
    uint8_t SA;
    uint16_t len;
    uint8_t data[J1939_MAX_DATA_SZ];
} unittest_j1939_rx_msg;


int unittest_helpers_setup(void);
void unittest_helpers_cleanup(void);

int unittest_get_output(j1939_primitive *f);

int unittest_post_input(uint32_t PGN, uint8_t DA, uint8_t SA, uint8_t len, ...);
int unittest_get_input(unittest_j1939_rx_msg *m);

uint32_t unittest_get_time(void);
void unittest_add_time(uint32_t time);

#endif /* UNITTEST_HELPERS_H_ */

