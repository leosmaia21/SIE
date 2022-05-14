#include "PWM.h"
#include "adc.h"
#include "config_bits.h"
#include "timer2.h"
#include "timer3.h"
#include "uart.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/attribs.h>
#include <xc.h>
#include <string.h>

#define SYSCLK 80000000L  // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz
#define KP 1.25
#define KI 1.0
#define SAMPLING_FREQ 10
#define SAMPLES 6
#define MAX_INTEGRATOR 1000
#define MAX_RPM 75.0

const float h = 1.0 / SAMPLING_FREQ;
volatile char direction = 0;
volatile char get_direction = 0;
volatile uint32_t count_pulses = 0;
float current_rpm = 0;
float error = 0;
float prev_error = 0.0;
float proportional = 0.0;
float integrator = 0;
int32_t u = 0;
uint32_t ref = 35;
float newDuty = 0.0;
volatile uint32_t flagToPrint = 0;
char toggle = 0;
float arr[SAMPLES];
float rpm_average = 0;
volatile char flagFrom10hz = 0;
volatile float ang_total = 0;
float mean_filter(float new_sample);
double time=0;


float mean_filter(float new_sample){
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
  memset(arr, 0.0, SAMPLES);
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
  setPWM(100);

  TRISEbits.TRISE8 = 1; // Set pin as input
  TRISDbits.TRISD2 = 1; // D2 as digital input

  /* Configuração interrupcao UART */
  INTCONbits.INT1EP =
      1;               // Generate interrupts on {rising edge-1 falling edge - 0}
  IFS0bits.INT1IF = 0; // Reset int flag
  IPC1bits.INT1IP =
      5;               // Set interrupt priority (1..7) *** Set equal to ilpx above
  IEC0bits.INT1IE = 1; // Enable Int1 interrupts

  while (1)
  {
    if (flagFrom10hz)
    {
        
      time+=h;
      current_rpm = (float)(count_pulses * 60.0 * SAMPLING_FREQ / 420.0);
      rpm_average = mean_filter(current_rpm);

      // printf("rpm_average: %.1f,current_rpm: %.1f\n\r", rpm_average,
      // current_rpm);
      count_pulses = 0;
      error = ref - rpm_average;
      /* proporcional*/
      proportional = error * KP;

      /* integrador*/
      integrator += ((0.5 * (prev_error + error)) * h);
      prev_error = error;
      if (integrator > MAX_INTEGRATOR)
      {
        integrator = MAX_INTEGRATOR;
      }

      if (integrator < -MAX_INTEGRATOR)
      {
        integrator = -MAX_INTEGRATOR;
      }

      prev_error = error;

      u = round(proportional + (integrator * KI));

      if (toggle)
      {
        newDuty = (MAX_RPM - u) * (50.0 / MAX_RPM);
      }
      else
      {
        newDuty = (u + MAX_RPM) * (50.0 / MAX_RPM);
      }

      if (newDuty > 100)
      {
        newDuty = 100.0;
      }
      if (newDuty < 0)
      {
        newDuty = 0.0;
      }
      // setPWM(100);
      setPWM(newDuty);
      printf("T: %.1f,RPM: %.1f  , Ref: %u, erro: %.1f, u: %d,newduty: %.1f,angle: "
             "%.1f\r\n",
             time,rpm_average, ref, error, u, newDuty, ang_total);
      flagFrom10hz = 0;
    }

    if (flagToPrint)
    {

      flagToPrint = 0;
    }
  }
}
void __ISR(24) isr_uart1(void) { IFS0bits.U1RXIF = 0; }

void __ISR(_EXTERNAL_1_VECTOR, IPL5AUTO) ExtInt1ISR(void)
{
  count_pulses++;
  float angle = count_pulses * 360.0 / 420;
  ang_total = ang_total + angle;
  if (ang_total >= 360)
    ang_total = 0;
  // printf("countISR: %lu\n\r",count_pulses);
  //  printf("pulsos: %d",count_pulses);
  if (PORTDbits.RD2)
    get_direction = 1;
  else
    get_direction = -1;

  IFS0bits.INT1IF = 0; // Reset interrupt flag
}
/* void __ISR(_TIMER_2_VECTOR, IPL4AUTO) T2Interrupt(void) {

 IFS0bits.T2IF = 0;
}*/
void __ISR(_TIMER_3_VECTOR, IPL3AUTO) T3Interrupt(void)
{

  PORTEbits.RE7 = !PORTEbits.RE7;
  // flagToPrint;
  flagFrom10hz = 1;

  IFS0bits.T3IF = 0;
}