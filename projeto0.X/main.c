#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/attribs.h>
#include <xc.h>
#include "adc.h"
#include "uart.h"
#include "PWM.h"
#include "config_bits.h"
#include "timer2.h"
#include "timer3.h"

#define SYSCLK  80000000L // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz
float filterPWM(int);
  int count = 0;
  float voltage;
  float DCvalue;
  float sample = 0;
  int duty;


  float res;
int DC;
int firstRun = 1;
int samples = 8;
uint32_t DCupdate = 0;
volatile uint32_t *ADCPointer;
volatile uint32_t ADCValue;
uint16_t freq;
const uint16_t minFreq = 100;
const uint16_t maxFreq = 500;
char str[8];
int array[4]={0, 0, 0, 0};
int sum;
  
  
  
volatile uint32_t *value;
  
  
/* void __ISR(_TIMER_2_VECTOR, IPL3AUTO) T2Interrupt(void) { 
    IFS0bits.T2IF = 0; 
}
*/
float filterPWM(int dutycycle) {

  float DCvoltage = (3.3 * (((float)(dutycycle)) / 100));

  return DCvoltage;
}



int main(int argc, char **argv) {
     
    
  

  
  TRISEbits.TRISE7=0;
  // Init UART and redirect tdin/stdot/stderr to UART
  if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
    PORTAbits.RA3 = 0; // If Led active error initializing UART
    while (1);
  }
  AdcConfig();
  __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/

  
 INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();

  
  Timer3Config(500);
 
  Timer2Config(2000);
 
  
   PORTEbits.RE7 = 0;

  while (1) {	       
    /*  if (count < 4) {
        sample += v;
        count++;
      } else {
        sample = sample / 4;
        count = 0;
        duty = (sample * 100) / 1023;
        voltage = (sample * 3.3) / 1023;
        printf("Voltagem de entrada: %f\n\r", voltage);
        printf("duty cycle: %d\n\r", duty);

        setPWM(duty); // configurar PWM

        DCvalue = filterPWM(duty); // filtrar o valor DC do PWM
  sample=0;
        printf("Conversão para adc: %f\n\r", (float)DCvalue);
      }*/
   
  }
}


void __ISR(_TIMER_3_VECTOR, IPL4AUTO) T3Interrupt(void) {
    
     IFS1bits.AD1IF = 0;
    AD1CON1bits.ASAM = 1; // Start conversion

    while (IFS1bits.AD1IF == 0); // Wait fo EOC
       
    PORTEbits.RE7 = !PORTEbits.RE7;
    
    value = (&ADC1BUF0);
   
    //v = v / 16;
   
    setPWM(*value); // configurar PWM
    //DCvalue = filterPWM(duty); // filtrar o valor DC do PWM
    IFS0bits.T3IF = 0; 
    
   /*   IFS1bits.AD1IF = 0; // Reset interrupt flag
    
    AD1CON1bits.ASAM = 1; // Start conversion
    while (IFS1bits.AD1IF == 0); // Wait for EOC
    PORTEbits.RE7 = !PORTEbits.RE7 ;
    ADCPointer = &ADC1BUF0;
    
    for(; ADCPointer <= (&ADC1BUFF); ADCPointer+=4)
    {
        ADCValue = ADCValue + *ADCPointer;
    }
    
    for (int i = sizeof(array)/sizeof(int)-1; i > 0; i--){
        array[i]=array[i-1];
    }
    array[0] = ADCValue/samples;
    
    sum = 0;
    
    for (int i = 0; i<=sizeof(array)/sizeof(int)-1;i++){
        sum = sum + array[i];
    }
    // Convert to 0..3.3V 
    res = ((sum/(sizeof(array)/sizeof(int)))*3.3)/1023;
    // Convert 0..3.3V to 0..100%
    DC = res*100/3.3;
    DCupdate = sum/(sizeof(array)/sizeof(int));
            
    setPWM(DCupdate);
    ADCValue = 0;
    // Reset interrupt flag
    IFS0bits.T3IF = 0;
    *
    */
}