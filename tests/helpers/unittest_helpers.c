
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


static void __user_j1939_rx_handler(uint8_t index, uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time);
static int __user_j1939_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const name);
static int __user_j1939_cannot_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const name);


callback_on_delay __on_delay = NULL;

static int cannot_claim_status = 0;

static uint32_t __the_time = 0;
static int __sent_pipes[2] = { -1, -1 };
static int __recv_pipes[2] = { -1, -1 };
static int __claim_pipes[2] = { -1, -1 };
static int __cannot_claim_pipes[2] = { -1, -1 };

static const j1939_callbacks cb = {
    .rx_handler = __user_j1939_rx_handler,
    .claim_handler = __user_j1939_claim_handler,
    .cannot_claim_handler = __user_j1939_cannot_claim_handler
};



int unittest_helpers_setup(uint8_t index) {
    __the_time = 0;
    
    if (pipe2(__sent_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }
    
    if (pipe2(__recv_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }

    if (pipe2(__claim_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }

    if (pipe2(__cannot_claim_pipes, O_DIRECT | O_NONBLOCK) < 0) {
        return -1;
    }

    unittest_set_callback_on_delay(NULL);
    unittest_set_cannot_claim_status(0);

    j1939_initialize(index, &cb);
    
    return 0;
}


static inline void close_quietly(int *fd) {
    if (fd == NULL) {
        return;
    }

    if (*fd > 0) {
        close(*fd);
    }

    *fd = -1;
}


void unittest_helpers_cleanup(void) {
    close_quietly(&__sent_pipes[PIPE_RD]);
    close_quietly(&__sent_pipes[PIPE_WR]);

    close_quietly(&__recv_pipes[PIPE_RD]);
    close_quietly(&__recv_pipes[PIPE_WR]);
    
    close_quietly(&__claim_pipes[PIPE_RD]);
    close_quietly(&__claim_pipes[PIPE_WR]);

    close_quietly(&__cannot_claim_pipes[PIPE_RD]);
    close_quietly(&__cannot_claim_pipes[PIPE_WR]);
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


static void __user_j1939_rx_handler(uint8_t index, uint32_t PGN, uint8_t src_address, uint8_t dst_address, uint16_t msg_sz, const void *const payload, uint32_t time) {
    unittest_j1939_rx_msg msg;

    if (__recv_pipes[PIPE_WR] < 0 || index != CAN_INDEX)
        return;

    msg.index = index;
    msg.PGN = PGN;
    msg.SA  = src_address;
    msg.DA = dst_address;
    msg.len = msg_sz;
    memcpy(msg.data, payload, msg_sz);
    msg.time = time;

    write(__recv_pipes[PIPE_WR], &msg, sizeof(unittest_j1939_rx_msg));
}


static int __user_j1939_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const name) {
    unittest_j1939_claim_msg msg;

    msg.index = index;
    msg.address = address;
    msg.name = *name;

    write(__claim_pipes[PIPE_WR], &msg, sizeof(msg));

    return 0;
}


static int __user_j1939_cannot_claim_handler(uint8_t index, uint8_t address, const j1939_CA_name *const name) {
    unittest_j1939_claim_msg msg;

    msg.index = index;
    msg.address = address;
    msg.name = *name;

    write(__cannot_claim_pipes[PIPE_WR], &msg, sizeof(msg));

    int status = cannot_claim_status;
    cannot_claim_status = 0;

    return status;
}


int j1939_bsp_CAN_send(uint8_t index, const j1939_primitive *const primitive) {
    if (__sent_pipes[PIPE_WR] < 0 || index != CAN_INDEX)
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


int unittest_post_input(uint8_t index, uint32_t PGN, uint8_t DA, uint8_t SA, uint8_t len, ...) {
    va_list va;
    j1939_primitive frame;
    uint8_t i;

    va_start(va, len);

    for(i = 0;i < len; ++i) {
        frame.payload[i] = (uint8_t)va_arg(va, int);
    }

    va_end(va);

    frame.dlc           = len;
    frame.PGN           = PGN;
    frame.dest_address  = j1939_is_PDU2(PGN) ? J1939_GLOBAL_ADDRESS : DA;
    frame.src_address   = SA;
    frame.priority      = 7;

    return j1939_handle_receiving(index, &frame, __the_time);
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

int unittest_get_claim(unittest_j1939_claim_msg *msg) {
    unittest_j1939_claim_msg out;

    if (__claim_pipes[PIPE_RD] < 0) {
        return -1;
    }

    int sts = read(__claim_pipes[PIPE_RD], &out, sizeof(out));
    if (sts < 0) {
        return sts;
    }

    if (msg) {
        *msg = out;
    }

    return 0;
}


void unittest_set_cannot_claim_status(int status) {
    cannot_claim_status = status;
}


int unittest_get_cannot_claim(unittest_j1939_claim_msg *msg) {
    unittest_j1939_claim_msg out;

    if (__cannot_claim_pipes[PIPE_RD] < 0) {
        return -1;
    }

    int sts = read(__cannot_claim_pipes[PIPE_RD], &out, sizeof(out));
    if (sts < 0) {
        return sts;
    }

    if (msg) {
        *msg = out;
    }

    return 0;
}
