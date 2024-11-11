#ifndef J1939_PRIORITY_H_
#define J1939_PRIORITY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum j1939_priority {
    J1939_PRIORITY_EMERGENCY    = 0,
    J1939_PRIORITY_HIGH1        = 1,
    J1939_PRIORITY_HIGH2        = 2,
    J1939_PRIORITY_CONTROL      = 3,
    J1939_PRIORITY_LOW1         = 4,
    J1939_PRIORITY_LOW2         = 5,
    J1939_PRIORITY_DEFAULT      = 6,
    J1939_PRIORITY_LOWEST       = 7
} j1939_priority;

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIORITY_H_ */
