
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <J1939/j1939.h>

#include "unittest_helpers.h"


#ifndef CA_ADDR
#define CA_ADDR     J1939_STD_ADDR_CAB_DISPLAY
#endif

#define PIPE_RD     0
#define PIPE_WR     1


static void __user_j1939_rx_handler(uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time);


callback_on_delay __on_delay = NULL;

static uint32_t __the_time = 0;
static int __sent_pipes[2] = { -1, -1 };
static int __recv_pipes[2] = { -1, -1 };
static const j1939_callbacks cb = {
    .rx_handler = __user_j1939_rx_handler,
};



int unittest_helpers_setup(void) {
    __the_time = 0;
    
    if (pipe2(__sent_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }
    
    if (pipe2(__recv_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }

    unittest_set_callback_on_delay(NULL);

    j1939_initialize(&cb);
    
    return 0;
}


void unittest_helpers_cleanup(void) {
    if (__sent_pipes[PIPE_RD] > 0) {
        close(__sent_pipes[PIPE_RD]);
    }

    __sent_pipes[PIPE_RD] = -1;

    if (__sent_pipes[PIPE_WR] > 0) {
        close(__sent_pipes[PIPE_WR]);
    }
    
    __sent_pipes[PIPE_WR] = -1;
    
    if (__recv_pipes[0] > 0) {
        close(__recv_pipes[0]);
    }

    __recv_pipes[PIPE_RD] = -1;

    if (__recv_pipes[1] > 0) {
        close(__recv_pipes[1]);
    }
    
    __recv_pipes[PIPE_WR] = -1;
}


void unittest_set_callback_on_delay(callback_on_delay cb) {
    __on_delay = cb;
}


uint32_t unittest_get_time(void) {
    return __the_time;
}


void unittest_add_time(uint32_t time) {
    __the_time += time;
}


void __user_j1939_rx_handler(uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time) {
    unittest_j1939_rx_msg msg;

    if (__recv_pipes[PIPE_WR] < 0)
        return;

    msg.PGN = PGN;
    msg.SA  = src_address;
    msg.DA = dst_address;
    msg.len = msg_sz;
    memcpy(msg.data, payload, msg_sz);
    msg.time = time;

    write(__recv_pipes[PIPE_WR], &msg, sizeof(unittest_j1939_rx_msg));
}


int j1939_bsp_CAN_send(const j1939_primitive *const primitive) {
    if (__sent_pipes[PIPE_WR] < 0)
        return -1;
        
    return write(__sent_pipes[PIPE_WR], primitive, sizeof(j1939_primitive));
}


int unittest_get_output(j1939_primitive *f) {
    j1939_primitive frame;
    
    if (__sent_pipes[PIPE_RD] < 0)
        return -1;
        
    if (read(__sent_pipes[PIPE_RD], &frame, sizeof(j1939_primitive)) < 0)
        return -2;
    
    if (f)
        *f = frame;
    
    return 0;
}


int unittest_post_input(uint32_t PGN, uint8_t DA, uint8_t SA, uint8_t len, ...) {
    va_list va;
    j1939_primitive frame;
    uint8_t i;

    va_start(va, len);

    for(i = 0;i < len; ++i) {
        frame.payload[i] = (uint8_t)va_arg(va, int);
    }

    va_end(va);

    frame.dlc           = len;
    frame.PGN.value     = j1939_is_PDU1(TREAT_AS_PGN(PGN)) ? PGN | DA : PGN;
    frame.src_address   = SA;
    frame.priority      = 7;

    return j1939_handle_receiving(&frame, __the_time);
}


int unittest_get_input(unittest_j1939_rx_msg *m) {
    unittest_j1939_rx_msg rx_msg;
    int sts;

    if (__recv_pipes[PIPE_RD] < 0)
        return -1;

    sts = read(__recv_pipes[PIPE_RD], &rx_msg, sizeof(unittest_j1939_rx_msg));
    if (m)
        *m = rx_msg;

    return sts;
}

