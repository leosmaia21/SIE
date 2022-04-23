#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/attribs.h>
#include <xc.h>
/* Kernel includes */
#include "adc.h"
#include "uart.h"
//#include "oc1.h"
#include "PWM.h"
#include "config_bits.h"
#include "timer2.h"
#include "timer3.h"

#define SYSCLK 80000000L  // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz

void __ISR(_TIMER_3_VECTOR, IPL5AUTO) T3Interrupt(void) {

    PORTBbits.RB7 = 1; // If Led active error initializing UART
  AD1CON1bits.ASAM = 1; // start convertion

  IFS0bits.T3IF = 0;
}
void __ISR(_TIMER_2_VECTOR, IPL5AUTO) T2Interrupt(void) { IFS0bits.T2IF = 0; }

float filterPWM(int dutycycle) {

  float DCvoltage = (3.3 * (((float)(dutycycle)) / 100));

  return DCvoltage;
}

int main(int argc, char **argv) {

  TRISAbits.TRISA3 = 0; // A3 as output
  TRISBbits.TRISB7=0;
  TRISDbits.TRISD5 = 0; // D5 a output
  ODCDbits.ODCD5 = 1;   // D5 as open-drain

  // Init UART and redirect tdin/stdot/stderr to UART
  if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
    PORTAbits.RA3 = 0; // If Led active error initializing UART
    while (1)
      ;
  }
  __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/

  /* Set Interrupt Controller for multi-vector mode */
  INTCONSET = _INTCON_MVEC_MASK;

  /* Enable Interrupt Exceptions */
  // set the CP0 status IE bit high to turn on interrupts globally
  __builtin_enable_interrupts();

  Timer3Config(400);
  Timer3Start();
  Timer2Config(2000);
  Timer2Start();
  PWMInit();
  AdcConfig();
  AdcStart();
  int count = 0;
  float voltage;
  float DCvalue;
  float sample = 0;
  int duty;
  while (1) {
    while (IFS1bits.AD1IF == 0); // Wait fo EOC
    IFS1bits.AD1IF = 0;

    int *p = (int *)(&ADC1BUF0);
    int v = 0;

    int i;
    for (; p <= (int *)(&ADC1BUFF); p++) {
      v += *p;
    }
    v = v / 16;
    duty = (v * 100) / 1023;
    voltage = (v * 3.3) / 1023;
    printf("Voltagem de entrada: %f\n\r", voltage);
    printf("duty cycle: %d\n\r", duty);

    setPWM(duty); // configurar PWM

    DCvalue = filterPWM(duty); // filtrar o valor DC do PWM
			       
    PORTBbits.RB7 = 0; 
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
