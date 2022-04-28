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
    
    T3CONbits.ON = 0;   // Stop timer
    T3CONbits.TCKPS = 3;  //Pre-scaler
    PR3 = PBCLOCK/(8*TimerFrequency)-1;  // Compute PR value depending on the chosen frequency 
    TMR3 = 0;


    
    IPC3bits.T3IP = 4;  // Priority 4
    IEC0bits.T3IE = 1;  // Interrupt enable
    IFS0bits.T3IF = 0;  // Reset interrupt flag
    
    T3CONbits.TON=1; // Start the timer
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


