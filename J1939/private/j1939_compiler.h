#ifndef J1939_PRIVATE_COMPILER_H_
#define J1939_PRIVATE_COMPILER_H_

#if defined(__GNUC__)

/* Optimization barrier */
#ifndef barrier
/* The "volatile" is due to gcc bugs */
#define barrier()           __asm__ __volatile__("": : :"memory")
#endif

#ifndef __aligned
#define __aligned(x)        __attribute__((aligned(x)))
#endif

#else

#warning "This library doesn't support any compiler than gcc!"

#ifndef barrier
#define barrier()
#endif

#ifndef __aligned
#define __aligned(x)
#endif

#endif /* __GNUC__ */


#endif /* J1939_PRIVATE_COMPILER_H_ */
