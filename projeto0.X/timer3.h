

#ifndef TIMER3_H
#define	TIMER3_H

#include <stdint.h>

/* Define return codes */
#define TIMER2_SUCCESS 0
#define TIMER2_FAIL -1;

/* Define prototypes (public interface) */
int8_t Timer3Config(uint32_t TimerFrequency);
int8_t Timer3Start(void);
int8_t Timer3Stop(void);

#endif