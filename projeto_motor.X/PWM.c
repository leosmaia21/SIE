

#include <xc.h>
#include "PWM.h"
#include <stdint.h>
#include <math.h>
void PWMInit(void)
{
    TRISDbits.TRISD0 = 0;
    PORTDbits.RD0 = 1;

    OC1CONbits.OCM = 6;
    OC1CONbits.ON = 0;
    OC1CONbits.OC32 = 0;
    OC1RS = 0;
    OC1CONbits.ON = 1;
    T2CONbits.TON = 1; // Start the timer
}

void setPWM(float duty)
{  
    OC1RS=(uint16_t)((PR2+1)*duty/100.0);
}
