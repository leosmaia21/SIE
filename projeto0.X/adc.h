

#ifndef ADC_H
#define	ADC_H

#include <stdint.h>

/* Define return codes */
#define ADC_SUCCESS 0
#define ADC_FAIL -1;

/* Define prototypes (public interface) */
int8_t AdcConfig(void);
int8_t AdcStart(void);
int8_t AdcStop(void);

#endif
