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

#define SYSCLK 80000000L  // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz
#define KP 2.3
#define SAMPLING_FREQ 10
volatile char direction = 0;
volatile char get_direction = 0;
volatile char flagToPrint = 0;
volatile uint32_t count_pulses = 0;
volatile int32_t current_rpm = 0;
volatile int32_t error = 0;
volatile int32_t u = 0;
volatile int32_t ref = 50;
volatile float newDuty = 0;
volatile uint32_t teste = 0;
volatile char toggle = 0;
int main(int argc, char **argv)
{

  TRISEbits.TRISE7 = 0;
  // Init UART and redirect tdin/stdot/stderr to UART
  if (UartInit(PBCLOCK, 115200) != UART_SUCCESS)
  {
    PORTAbits.RA3 = 0; // If Led active error initializing UART
    while (1)
      ;
  }
  AdcConfig();
  __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/

  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();

  Timer2Config(20000);
  Timer3Config(SAMPLING_FREQ);
  PWMInit();
  setPWM(60);

  TRISEbits.TRISE8 = 1; // Set pin as input
  TRISDbits.TRISD2 = 1; // D2 as digital input

  /* Configuração interrupcao UART */
  INTCONbits.INT1EP = 1; // Generate interrupts on {rising edge-1 falling edge - 0}
  IFS0bits.INT1IF = 0;   // Reset int flag
  IPC1bits.INT1IP = 5;   // Set interrupt priority (1..7) *** Set equal to ilpx above
  IEC0bits.INT1IE = 1;   // Enable Int1 interrupts
  int x = 0;
  while (1)
  {
     printf("RPM: %d  , Ref: %d, erro: %d, newDuty: %.3f, teste: %.3f\r\n", current_rpm, ref, error, newDuty, teste);
      flagToPrint = 0;
    // if(get_direction)
    //   printf("frente");
    // else
    //   printf("tras");
  }
}
void __ISR(24) isr_uart1(void)
{
  teste++;
  printf("teste %d",teste);
  IFS0bits.U1RXIF = 0;
}

void __ISR(_EXTERNAL_1_VECTOR, IPL5AUTO) ExtInt1ISR(void)
{
  count_pulses++;
  if (PORTDbits.RD2)
    get_direction = 1;
  else
    get_direction = -1;

  IFS0bits.INT1IF = 0; // Reset interrupt flag
}
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) T2Interrupt(void)
{
  IFS0bits.T2IF = 0;
}
void __ISR(_TIMER_3_VECTOR, IPL3AUTO) T3Interrupt(void)
{
  current_rpm = count_pulses * 60 * SAMPLING_FREQ / 420;
  count_pulses = 0;
  error = ref - current_rpm;
  newDuty = (float)(error * KP);

  if (newDuty > 100)
  {
    newDuty = 100;
  }
  if (newDuty < 0)
  {
    newDuty = 0;
  }

  //  setPWM(50-newDuty);
  OC1RS = PR2 / 2 + PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 50% -- 100%

  // OC1RS = PR2 / 2 - PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 0% -- 50%
  // setPWM(50-newDuty);
  if (toggle == 1)
  {
    //  OC1RS = PR2 / 2 + PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 50% -- 100%
  }
  /*else
  {
    OC1RS = PR2 / 2 - PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 0% -- 50%
  } */

  // setPWM(newDuty);
  flagToPrint=1;
  IFS0bits.T3IF = 0;
}