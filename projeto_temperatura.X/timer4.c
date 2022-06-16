#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/* Kernel includes */
#include "timer2.h"

#define PBCLOCK 40000000L

const uint16_t prescaler4[] = {1, 2, 4, 8, 16, 32, 64, 256};
const uint8_t size_of_prescaler4 = 8;

/********************************************************************
 * Function: 	Timer2Config
 * Precondition:
 * Input: 	    Timer frequency
 * Returns:      TIMER2_SUCCESS if it is Ok.
 * Side Effects:
 * Overview:     Configure timer 2
 ********************************************************************/
int8_t Timer4Config(uint32_t TimerFrequency) {

  /*Variables*/
  uint8_t i = 0;
  uint32_t pr4 = 0;

  /*Timer T3 configuration */
/*  do {
    if (i == size_of_prescaler4)
      return -1;
    pr4 = (PBCLOCK / (TimerFrequency * prescaler4) - 1);
    T4CONbits.TCKPS = i;
    i++;
  } while (pr4 > UINT16_MAX);
*/
  PR4 = pr4;
  /*   T3CONbits.TCKPS=3;
     PR3=PBCLOCK/(8*TimerFrequency)-1; */
  TMR4 = 0;

  T4CONbits.ON = 0;  // Stop timer
  IFS0bits.T4IF = 0; // Reset interrupt flag
  IPC4bits.T4IP = 3; // set interrupt priority (1..7) *** Make sure it matches
                     // iplx in isr declaration ***
  IEC0bits.T4IE = 0; // Disable T3 interrupts / Enable T2 interrupt
  T4CONbits.TON = 1;
  return TIMER2_SUCCESS;
}
