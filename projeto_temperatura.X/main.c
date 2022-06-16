#include "PWM.h"
#include "adc.h"
#include "config_bits.h"
#include "timer2.h"
#include "timer3.h"
#include "timer4.h"
#include "uart.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attribs.h>
#include <sys/types.h>
#include <xc.h>

#define SYSCLK 80000000L  // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz
#define KP 1.2
#define KI 0.9

#define SAMPLING_FREQ 1 // frequencia real = 0.2
#define TIMER3_FREQ 5
#define SAMPLES 4
#define MAX_INTEGRATOR 100
#define MAX_RPM 75.0
#define room_temperature(v) ((-v * 33.2837) + 79.7840)
//#define termocouple_temperature(v) ((v / 1000.0) * 24354.99 + 0.5014)

const float h = 1.0 / SAMPLING_FREQ;
float proportional = 0.0;
float integrator = 0;
float error = 0;
float prev_error = 0;
float u = 0.0;
float prevU = 0;
float ref = 75;
float newDuty = 0.0;
float termopar = 0;
float ambiente = 0.0;
float real_temp = 0.0;
float mean_filter(float new_sample);

const float KCR = 0.04; // valor da aparesentacao 1.1
const float PCR = 0.002;

const float TI = PCR / 1.2;
const float K = KCR * 0.45;

float s0 = K * (1 + (h / TI));
float s1 = -K;

float mean_filter(float);
float termocouple_temperature(float);

volatile uint32_t count_timer_3 = 0;

int main(int argc, char **argv) {
    // memset(arr, 0.0, SAMPLES);
    TRISEbits.TRISE7 = 0;
    PORTEbits.RE7 = 0;
    // UART Interrupt configuration
    // IEC0bits.U1RXIE = 1;  // Enable RX interrupts
    // IPC6bits.U1IP = 2;    // Uart interrupt priority/ Init UART and redirect
    if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 0; // If Led active error initializing UART
        while (1)
            ;
    }
    // uart1_config(115200, 8, 0, 1);
    __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/

    INTCONSET = _INTCON_MVEC_MASK;
    __builtin_enable_interrupts();

    Timer2Config(20000);
    Timer3Config(TIMER3_FREQ);
    // Timer4Config(40000);
    AdcConfig();
    PWMInit();
    setPWM(50);

    TRISEbits.TRISE8 = 1; // Set pin as input
    TRISDbits.TRISD2 = 1; // D2 as digital input
    float uToPrint = 0;
    float s0 = K * (1 + (h / TI));
    float s1 = -K;
    while (1) {
        if (count_timer_3) {
            float adc0 = 0;
            AD1CHSbits.CH0SA = 0; // Select AN0 as input for A/D converter
            AdcStart();
            while (IFS1bits.AD1IF == 0)
                ;
            IFS1bits.AD1IF = 0;
            termopar = (ADC1BUF0 * 3.3) / 1023;
            adc0 = termopar;
            AD1CHSbits.CH0SA = 1; // Select AN1 as input for A/D converter
            AdcStart();
            while (IFS1bits.AD1IF == 0)
                ;
            IFS1bits.AD1IF = 0;
            float adc1;
            ambiente = (ADC1BUF0 * 3.3 / 1023);
            adc1 = ambiente;
            // ambiente =  3.3- ambiente;
            ambiente = room_temperature((3.3 - ambiente));
            termopar = termocouple_temperature(termopar);
            real_temp = termopar + ambiente;

            error = ref - real_temp;

            u = (s0 * error) + (s1 * prev_error) + prevU;
            //u = error * 10.0;
            uToPrint=u;
            newDuty = u;
             if (u < 0.0)
                 u = 0.0;
             if (u > 100.0)
                 u = 100.0;
            prevU = u;
            prev_error = error;

            setPWM(u);
            printf("ADC0:%.3f,ADC1:%.3f  temp ambiente:%.1f  temp "
                    "resistencia:%.1f,u:%.1f,Erro:%.1f\r\n",
                    adc0, adc1, ambiente, real_temp, u, error);
            count_timer_3 = 0;
        }
    }
}

float termocouple_temperature(float value) {
    float y;
    y = value - 0.499;
    y /= 542.47;
    y = y * 24355.0 + 0.5014;
    return y;
}
const uint32_t divider = (uint32_t) (TIMER3_FREQ / SAMPLING_FREQ);
volatile int x = 0;

void __ISR(_TIMER_3_VECTOR, IPL3AUTO) T3Interrupt(void) {
    x++;
    if ((x % divider) == 0)
        count_timer_3 = 1;
    IFS0bits.T3IF = 0;
}
// float mean_filter(float new_sample) {
/*update the array of previous samples. The new sample is at position
 0 and the oldest sample is at position arr_size - 1 */
/*   uint32_t i;
   for (i = SAMPLES - 1; i > 0; i--) {
     arr[i] = arr[i - 1];
   }
   arr[0] = new_sample; // update new sample read by the ADC


   float avg_sample = 0;
   for (i = 0; i < SAMPLES; i++) {
     avg_sample += arr[i];
   }
   avg_sample /= SAMPLES;

   return avg_sample;
 }*/
