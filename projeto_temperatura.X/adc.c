

/* Standard includes */
#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/* Kernel includes */
#include "adc.h"

/********************************************************************
 * Function: 	AdcConfig()
 * Precondition:
 * Input:
 * Returns:      ADC_SUCCESS if it is Ok.
 * Side Effects:	Takes control of U1A TX and RX pins
 * Overview:     Configure ADC.
 ********************************************************************/
 void AdcConfig(void) {
  DDPCONbits.JTAGEN = 0;

  // Initialize ADC module
  // Polling mode, AN0 as input
  // Generic part
  AD1CON1bits.SSRC = 7; // Internal counter ends sampling and starts conversion
  AD1CON1bits.CLRASAM = 1; // Stop conversion when 1st A/D converter interrupt
                           // is generated and clears ASAM bit automatically
  AD1CON1bits.FORM = 0;     // Integer 16 bit output format
  AD1CON2bits.VCFG = 0;     // VR+=AVdd; VR-=AVss
  AD1CON2bits.SMPI = 1 - 1; // Number (+1) of consecutive conversions, stored in
                            // ADC1BUF0...ADCBUF{SMPI}
  AD1CON3bits.ADRC = 0;     // ADC uses internal RC clock
  AD1CON3bits.SAMC = 16;    // Sample time is 16TAD ( TAD = 100ns)
  // Set AN0 as input
  AD1CHSbits.CH0SA = 0;  // Select AN0 as input for A/D converter
  TRISBbits.TRISB0 = 1;  // Set AN0 to input mode
  AD1PCFGbits.PCFG0 = 0; // Set AN0 to analog mode
  // Enable module
  AD1CON1bits.ON = 1; // Enable A/D module (This must be the ***last instruction
                      // of configuration phase***)

 
}

/********************************************************************
 * Function: 	AdcStart()
 * Precondition:
 * Input:
 * Returns:      ADC_SUCCESS if it is Ok.
 * Overview:     Configure ADC.
 ********************************************************************/
void AdcStart(void) {
  AD1CON1bits.ASAM = 1;
  
}

/********************************************************************
 * Function: 	AdcStop()
 * Precondition:
 * Input:
 * Returns:      ADC_SUCCESS if it is Ok.
 * Overview:     Configure ADC.
 ********************************************************************/
void AdcStop(void) {
  AD1CON1bits.ASAM = 0; // Stop conversion
  
}
