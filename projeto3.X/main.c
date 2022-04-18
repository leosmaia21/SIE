
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
/* Kernel includes */
#include "adc.h"
#include "uart.h"
//#include "oc1.h"
#include "timer2.h"
#include "config_bits.h"

#define SYSCLK  80000000L // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz
  float value;
  float res; 
uint32_t adcValue1, adcValue2;
float adc1_10_18(uint32_t );
float adc1_18_30(uint32_t );
float adc2_30_46(uint32_t );
float adc2_40_60(uint32_t );

int adcAverage(void);

int main(int argc, char** argv){
    TRISAbits.TRISA3 = 0;  // A3 as output
  UartInit(PBCLOCK, 115200);
    __XC_UART = 1;
            
    INTCONSET = _INTCON_MVEC_MASK;
   
    __builtin_enable_interrupts();
     /* Timer configurations */
    Timer2Config(75000);
    Timer2Start();
     
   AdcConfig();
   AdcStart();
  PWMInit();
   
  printf("\nStart Main\n\n");
   
 
    while(1){
       
        AD1CHSbits.CH0SA=0;

        adcValue1 =adcAverage();
        AD1CHSbits.CH0SA=1;
        adcValue2=adcAverage();
        // Output result
         printf("adc1: %d adc2: %d \n\r",adcValue1,adcValue2);
       if(adcValue1<3300 && adcValue1>=1900){
            
            value =adc1_10_18(adcValue1);
             PORTAbits.RA3 = 1;
           
            
       }
       else if( adcValue1<1900 && adcValue1>=90){
            PORTAbits.RA3 = 1; 
            value =adc1_18_30(adcValue1);
            //printf("Distance : %f\r",value);
           
            
       }else  if(adcValue2 <=3300 && adcValue2 >=1010 ){
             
            
               value=adc2_30_46(adcValue2);
         
           
       }else if (adcValue2 <1010 && adcValue2 >=13) {
            PORTAbits.RA3 = 1; 
              
            value=adc2_40_60(adcValue2);
           
            
        } 
        
        uint32_t x=(uint32_t)(value*10);
        if(x >=580|| x<100){
            printf("\rE");
        PORTAbits.RA3 = 0;
       }else{
          PORTAbits.RA3 = 1;
             printf("\rDistance: %lu mm", x);
       }
        setPWM((600-(int)(value*10))/5);
            
        
    }
    
    return 0;

}


float adc1_10_18(uint32_t value){
    
    return (-3.7034*((float)value/1000) +21.99);
    
    
}

float adc1_18_30(uint32_t value){
    
    return (-9.9447*((float)value/1000)+29.308);
}

float adc2_30_46(uint32_t value) {
    return (-6.5617*((float)value/1000) +51.976);
}

float adc2_40_60(uint32_t value){
    
    return (-12.892*((float)value/1000)+58.567);
}


int adcAverage(void){
AD1CON1bits.ASAM = 1; // Start conversion

while(IFS1bits.AD1IF == 0);

  int *p = (int*)(&ADC1BUF0);
  int v = 0;
    
  int i;
  for(;p<=(int*)(&ADC1BUFF);p++){
    v +=*p;
  }

  v= v/16;
  v=(v*3300)/1023;
    
IFS1bits.AD1IF = 0;

  return v;
}


