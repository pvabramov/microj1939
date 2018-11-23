
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <J1939/j1939_bsp.h>

#include "unittest_helpers.h"


extern callback_on_delay __on_delay;

static pthread_mutex_t __lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


int j1939_bsp_lock(void) {
    pthread_mutex_lock(&__lock);
    return 1;
}


void j1939_bsp_unlock(int level) {
    (void)level;
    pthread_mutex_unlock(&__lock);
}


void j1939_bsp_mdelay(uint32_t ms) {
#if 0
    while (ms > 0) {
        usleep(1000);
        --ms;
    }
#endif
    if (__on_delay) {
        __on_delay();
    }

    unittest_add_time(ms);
}


uint32_t j1939_bsp_get_time() {
#if 0
    struct timespec res;
    uint32_t time;

    clock_gettime(CLOCK_MONOTONIC, &res);

    time  = res.tv_sec  * 1000;
    time += res.tv_nsec / 1000000;

    return time;
#else
    return unittest_get_time();
#endif
}

