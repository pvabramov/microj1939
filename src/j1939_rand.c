#include <J1939/j1939_config.h>
#include <J1939/private/j1939_rand.h>

#define RAND_LOCAL_MAX 2147483647L

static volatile uint32_t __jrand_next;

/*
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */

void jsrand(uint32_t seed) {
    if (seed == 0) {
        __jrand_next = J1939_RAND_SEED;
    } else {
        __jrand_next = seed;
    }
}

int32_t jrand(void) {
    return (__jrand_next = __jrand_next * 1103515245L + 12345L) % RAND_LOCAL_MAX;
}


int32_t jrandr(int32_t min, int32_t max) {
    return (int32_t) jrand() % (max - min + 1) + min;
}
