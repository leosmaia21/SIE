

/* Standard includes */
#include <xc.h>
#include <stdlib.h>
#include <stdint.h>

/* Kernel includes */
#include "timer2.h"

#define PBCLOCK 40000000L

const uint16_t prescaler2[] = {1, 2, 4, 8, 16, 32, 64, 256};
const uint8_t size_of_prescaler2 = 8;

/********************************************************************
* Function: 	Timer2Config
* Precondition: 
* Input: 	    Timer frequency
* Returns:      TIMER2_SUCCESS if it is Ok.     
* Side Effects:	
* Overview:     Configure timer 2 	 
********************************************************************/	
int8_t Timer2Config(uint32_t TimerFrequency)
{
    
    /*Variables*/
    uint8_t i = 0;
    uint32_t pr2 = 0;

    /*Timer T3 configuration */
    do {
        if (i == size_of_prescaler2)
            return -1;
        pr2 = (PBCLOCK / (TimerFrequency * prescaler2[i])) - 1;
        T2CONbits.TCKPS = i;                                       /* Set prescaler3 */
        i++;
    } while (pr2 > UINT16_MAX);
    
    PR2 = (uint16_t)pr2;
    TMR2 = 0;                                                      /* Reset timer */
    
    T2CONbits.ON = 0;                                              /* Stop the timer */
    IPC2bits.T2IP = 4;                                             /* Interrupt Priority */
    IEC0bits.T2IE = 1;                                             /* Enable T2 interrupts */
    IFS0bits.T2IF = 0;                                             /* Reset interrupt flag */
    T2CONbits.ON = 1;
    return TIMER2_SUCCESS;
}

/********************************************************************
* Function: 	Timer2Start
* Precondition: 
* Input: 	   
* Returns:      TIMER2_SUCCESS if it is Ok.     
* Side Effects:	
* Overview:     Start the timer  	 
********************************************************************/	
int8_t Timer2Start(void)
{
    T2CONbits.TON = 1; 
    return TIMER2_SUCCESS;
}
 
/********************************************************************
* Function: 	Timer2Start
* Precondition: 
* Input: 	   
* Returns:      TIMER2_SUCCESS if it is Ok.     
* Side Effects:	
* Overview:     Stop the timer  	 
********************************************************************/
int8_t Timer2Stop(void)
{
    T2CONbits.TON = 0;
    return TIMER2_SUCCESS;
}


