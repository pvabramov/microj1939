#ifndef J1939_RAND_H_
#define J1939_RAND_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void jsrand(int32_t seed);

int32_t jrand(void);
int32_t jrandr(int32_t min, int32_t max);

#ifdef __cplusplus
}
#endif

#endif /* J1939_RAND_H_ */
