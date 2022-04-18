

#ifndef TIMER2_H
#define	TIMER2_H

#include <stdint.h>

/* Define return codes */
#define TIMER2_SUCCESS 0
#define TIMER2_FAIL -1;

/* Define prototypes (public interface) */
int8_t Timer2Config(uint32_t TimerFrequency);
int8_t Timer2Start(void);
int8_t Timer2Stop(void);

#endif