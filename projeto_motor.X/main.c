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
#define KP 1.9
#define KI 0.2
#define SAMPLING_FREQ 10
#define SAMPLES 4
#define MAX_INTEGRATOR 1000

volatile char direction = 0;
volatile char get_direction = 0;
volatile uint32_t count_pulses = 0;
volatile float current_rpm = 0;
volatile float error = 0;
volatile int32_t prev_error = 0.0;
volatile float proportional = 0.0;
volatile float integrator = 0;
volatile float u = 0.0;
volatile int32_t ref = 10.0;
volatile float newDuty = 0.0;
volatile uint32_t flagToPrint = 0;
volatile char toggle = 0;
volatile char arr[SAMPLES] = {0, 0, 0, 0};
volatile float rpm_average = 0;

float mean_filter(float new_sample)
{
  /* update the array of previous samples. The new sample is at position 0
   and the oldest sample is at position arr_size - 1 */
  uint32_t i;
  for (i = SAMPLES - 1; i > 0; i--)
  {
    arr[i] = arr[i - 1];
  }
  arr[0] = new_sample; // update new sample read by the ADC

  /* compute the mean of all the samples to ouput */
  float avg_sample = 0;
  for (i = 0; i < SAMPLES; i++)
  {
    avg_sample += arr[i];
  }
  avg_sample /= SAMPLES;

  return avg_sample;
}
int main(int argc, char **argv)
{

  TRISEbits.TRISE7 = 0;
  PORTEbits.RE7 = 0;
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
  setPWM(50);

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
    if (flagToPrint >= (20000 / 4))
    {
      printf("RPM: %.1f  , Ref: %d, erro: %.1f, u: %.1f,newduty: %.1f\r\n", current_rpm, ref, error, u, newDuty);

      // if(get_direction)
      //   printf("frente");
      // else
      //   printf("tras");
      flagToPrint = 0;
    }
  }
}
void __ISR(24) isr_uart1(void)
{

  IFS0bits.U1RXIF = 0;
}

void __ISR(_EXTERNAL_1_VECTOR, IPL5AUTO) ExtInt1ISR(void)
{
  count_pulses++;
  // printf("pulsos: %d",count_pulses);
  if (PORTDbits.RD2)
    get_direction = 1;
  else
    get_direction = -1;

  IFS0bits.INT1IF = 0; // Reset interrupt flag
}
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) T2Interrupt(void)
{
  flagToPrint++;
  IFS0bits.T2IF = 0;
}
void __ISR(_TIMER_3_VECTOR, IPL3AUTO) T3Interrupt(void)
{
  PORTEbits.RE7 = !PORTEbits.RE7;
  current_rpm = (float)(count_pulses * 60.0 * SAMPLING_FREQ / 420.0);
  rpm_average = mean_filter(current_rpm);
  //printf("rm: %.1f,count: %d\n\r", rpm_average, count_pulses);
  count_pulses = 0;
  error = ref - rpm_average;
  /* proporcional*/
  proportional = error * KP;

  /* integrador*/
  // integrator += error * (1.0 / SAMPLING_FREQ);
  integrator += error;
  if (integrator > MAX_INTEGRATOR)
  {
    integrator = MAX_INTEGRATOR;
  }

  if (integrator < -MAX_INTEGRATOR)
  {
    integrator = -MAX_INTEGRATOR;
  }

  prev_error = error;

  float limMinInt, limMaxInt;

  /* anti-wind-up limites*/
  //  if (pid->integrator > pid->limMaxInt) {
  //
  //        pid->integrator = pid->limMaxInt;
  //
  //    } else if (pid->integrator < pid->limMinInt) {
  //
  //        pid->integrator = pid->limMinInt;
  //
  //    }

  // u = proportional + (integrator*KI);
  u = proportional;
  int y;
  if (toggle)
  {

    // OC1RS = PR2 / 2 + PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 50% -- 100%
    newDuty = (69 - u) * (50.0 / 69.0);
    // printf("\n\r y: %d",y);
  }
  else
  {
    newDuty = (u + 69) * (50.0 / 69.0);

    // OC1RS = PR2 / 2 - PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 0% -- 50%
  }
  // printf("u: %.1f: duty %.1f\r\n",u,newDuty);
  if (newDuty > 100)
  {
    newDuty = 100.0;
  }
  if (newDuty < 0)
  {
    newDuty = 0.0;
  }
  setPWM(newDuty);
  //  setPWM(50-newDuty);

  // OC1RS = PR2 / 2 - PR2 / 2 * ((float)newDuty / 100); // Duty-Cycle -> 0% -- 50%
  // setPWM(50-newDuty);

  // setPWM(newDuty);

  IFS0bits.T3IF = 0;
}