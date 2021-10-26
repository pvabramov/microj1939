#ifndef J1939_CONFIG_H_
#define J1939_CONFIG_H_

#include <j1939_conf.h>

///
#ifndef J1939_CAN_BAUDRATE
#   define J1939_CAN_BAUDRATE               (250000)
#endif

///
#ifndef J1939_RX_FIFO_SZ
#   define J1939_RX_FIFO_SZ                 (32)
#endif

///
#ifndef J1939_TX_FIFO_SZ
#   define J1939_TX_FIFO_SZ                 (32)
#endif

///
#ifndef J1939_RX_TX_ERROR_FIFO_SZ
#   define J1939_RX_TX_ERROR_FIFO_SZ        (32)
#endif

///
#ifndef J1939_TP_RX_SESSIONS_NUM
#   define J1939_TP_SESSIONS_NUM            (32)
#endif

///
#ifndef J1939_TP_TX_SESSIONS_NUM
#   define J1939_TP_SESSIONS_NUM            (32)
#endif

#undef J1939_TP_SESSIONS_NUM
#define J1939_TP_SESSIONS_NUM               (J1939_TP_RX_SESSIONS_NUM + J1939_TP_TX_SESSIONS_NUM + 1 /* for TX BAM session */)

#if (J1939_TP_RX_SESSIONS_NUM > 254)
#   error "J1939_TP_SESSIONS_NUM: A maximum sum of RX and TX sessions should be 254"
#endif

///
#ifndef J1939_TP_MGR_MAX_PACKETS_PER_CTS
#   define J1939_TP_MGR_MAX_PACKETS_PER_CTS    (8)
#endif

///
#ifndef J1939_MAX_DATA_SZ
#   define J1939_MAX_DATA_SZ                (1785)
#endif

#endif /* J1939_CONFIG_H_ */
