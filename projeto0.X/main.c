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
  PWMInit();
  
   PORTEbits.RE7 = 0;

  while (1) {	       
   
   
  }
}


void __ISR(_TIMER_3_VECTOR, IPL4AUTO) T3Interrupt(void) {
    AD1CON1bits.ASAM = 1; // Start conversion
    

    while (IFS1bits.AD1IF == 0); // Wait fo EOC
        IFS1bits.AD1IF = 0;
    
    PORTEbits.RE7 = !PORTEbits.RE7;
    
    
   
    value = (int*)(&ADC1BUF0);
  
    
  /*  int *p = (int*)(&ADC1BUF0);
  int v = 0;
    
  int i;
  for(;p<=(int*)(&ADC1BUFF);p++){
    v +=*p;
  }

  v= v/16; */
  
    for (int i = sizeof(array)/sizeof(int)-1; i > 0; i--){
        array[i]=array[i-1];
    }
    array[0] = *value;
    
    sum = 0;
    
    for (int i = 0; i<=sizeof(array)/sizeof(int)-1;i++){
        sum = sum + array[i];
    }
   
    int x =sum/4; 
    setPWM(x); // configurar PWM
    //DCvalue = filterPWM(duty); // filtrar o valor DC do PWM
    IFS0bits.T3IF = 0; 
    
  
}