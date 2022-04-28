

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
    
    TRISDbits.TRISD0 = 0;
    PORTDbits.RD0 = 1;
  
    // Set timer
    T2CONbits.ON = 0; // Stop timer
    IPC2bits.T2IP = 2;  // Priority 2
    T2CONbits.TCKPS = 1; //Select pre-scaler
    T2CONbits.T32 = 0; // 16 bit timer operation
    PR2=9999; // Compute PR value 2000 Hz
    TMR2=0;

    // Set OC1
    OC1CONbits.OCM = 6; // OCM = 0b110 : OC1 in PWM mode,
    OC1CONbits.OCTSEL=0; // Timer 2 is clock source of OCM
    OC1CONbits.ON=1;     // Enable OC1

    // Start PWM generation
    T2CONbits.TON=1; // Start the timer
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


