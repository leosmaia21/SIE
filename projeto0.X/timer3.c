 /* Standard includes */
#include <xc.h>
#include <stdlib.h>
#include <stdint.h>

/* Kernel includes */
#include "timer2.h"

#define PBCLOCK 40000000L

const uint16_t prescaler3[] = {1, 2, 4, 8, 16, 32, 64, 256};
const uint8_t size_of_prescaler3 = 8;

/********************************************************************
* Function: 	Timer2Config
* Precondition: 
* Input: 	    Timer frequency
* Returns:      TIMER2_SUCCESS if it is Ok.     
* Side Effects:	
* Overview:     Configure timer 2 	 
********************************************************************/	
int8_t Timer3Config(uint32_t TimerFrequency)
{
    
    /*Variables*/
    uint8_t i = 0;
    uint32_t pr3 = 0;

    /*Timer T3 configuration */
    do {
        if (i == size_of_prescaler3)
            return -1;
        pr3 = (PBCLOCK / (TimerFrequency * prescaler3[i])) - 1;
        T3CONbits.TCKPS = i;                                       
        i++;
    } while (pr3 > UINT16_MAX);
   
    PR3 = (uint16_t)pr3; 
 /*   T3CONbits.TCKPS=3;
    PR3=PBCLOCK/(8*TimerFrequency)-1; */
    TMR3 = 0;            
                                            
    T3CONbits.ON = 0;   // Stop timer
    IFS0bits.T3IF = 0;    // Reset interrupt flag
    IPC3bits.T3IP = 4;    //set interrupt priority (1..7) *** Make sure it matches iplx in isr declaration ***
    IEC0bits.T3IE = 1;  // Disable T3 interrupts / Enable T2 interrupt
    T3CONbits.TON = 1;
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
int8_t Timer3Start(void)
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
int8_t Timer3Stop(void)
{
    T2CONbits.TON = 0;
    return TIMER2_SUCCESS;
}


