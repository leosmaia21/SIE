#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attribs.h>
#include <xc.h>

#include "PWM.h"
#include "adc.h"
#include "config_bits.h"
#include "timer2.h"
#include "timer3.h"
#include "timer4.h"
#include "uart.h"

#define SYSCLK 80000000L   // System clock frequency, in Hz
#define PBCLOCK 40000000L  // Peripheral Bus Clock frequency, in Hz
#define KP 1.1
#define KI 0.9
#define SAMPLING_FREQ 300
#define SAMPLES 20
#define MAX_INTEGRATOR 100
#define MAX_RPM 75.0

const float h = 1.0 / SAMPLING_FREQ;
volatile char direction = 0;
volatile int8_t get_direction = 0;
volatile uint32_t count_pulses = 0;
float current_rpm = 0.0;
float rpm_average = 0;
float error = 0;
float prev_error = 0.0;
float proportional = 0.0;
float integrator = 0;
int32_t u = 0;
uint32_t ref = 33;
float newDuty = 0.0;
char toggle = 0;
float arr[SAMPLES];
volatile char flagFrom10hz = 0;
volatile float ang_total = 0;
char stop = 0;
volatile uint32_t newTMR=0;
char show=0;

float mean_filter(float new_sample);
double time = 0;
uint32_t time_perCycle = 0;
const uint32_t freq_pre_scale = 40000000 / 256;

typedef enum { showAll, showOneLine, changeDirection, setRPM, showMenu, stopMotor, invalid } menu;

float mean_filter(float);
int main(int argc, char** argv) {
    menu m = showMenu;
    memset(arr, 0.0, SAMPLES);
    TRISEbits.TRISE7 = 0;
    PORTEbits.RE7 = 0;
    // UART Interrupt configuration
    // IEC0bits.U1RXIE = 1;  // Enable RX interrupts
    // IPC6bits.U1IP = 2;    // Uart interrupt priority/ Init UART and redirect
    if (UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 0;  // If Led active error initializing UART
        while (1)
            ;
    }
    // uart1_config(115200, 8, 0, 1);
    __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/

    INTCONSET = _INTCON_MVEC_MASK;
    __builtin_enable_interrupts();

    Timer2Config(20000);
    Timer3Config(SAMPLING_FREQ);
    Timer4Config(40000);
    PWMInit();
    setPWM(50);

    TRISEbits.TRISE8 = 1;  // Set pin as input
    TRISDbits.TRISD2 = 1;  // D2 as digital input

    /* Configuração interrupcao UART */
    INTCONbits.INT1EP = 1;  // Generate interrupts on {rising edge-1 falling edge - 0}
    IFS0bits.INT1IF = 0;    // Reset int flag
    IPC1bits.INT1IP = 2;    // Set interrupt priority (1..7) *** Set equal to ilpx above
    IEC0bits.INT1IE = 1;    // Enable Int1 interrupts
    char c;
    char countNewRPM = 0;

    char new[2];
    while (1) {
        if (flagFrom10hz) {
            c = GetChar();
            time += h;
           // current_rpm = (float)(count_pulses * 60.0 * SAMPLING_FREQ / 420.0);
            current_rpm =60.0/(((float) newTMR / freq_pre_scale) * 420.0) ;
            rpm_average = mean_filter(current_rpm);
          
            // printf("rpm_average: %.1f,current_rpm:
            // %.1f\n\r", rpm_average, current_rpm);
            count_pulses = 0;
            error = ref - rpm_average;
            /* proporcional*/
            proportional = error * KP;

            /* integrador*/
            integrator += ((0.5 * (prev_error + error)) * h);
            prev_error = error;
            if (integrator > MAX_INTEGRATOR) {
                integrator = MAX_INTEGRATOR;
            }

            if (integrator < -MAX_INTEGRATOR) {
                integrator = -MAX_INTEGRATOR;
            }

            prev_error = error;

            u = round(proportional + (integrator * KI));

            if (toggle) {
                newDuty = (MAX_RPM - u) * (50.0 / MAX_RPM);
            } else {
                newDuty = (u + MAX_RPM) * (50.0 / MAX_RPM);
            }

            if (newDuty > 100) {
                newDuty = 100.0;
            }
            if (newDuty < 0) {
                newDuty = 0.0;
            }
             setPWM(newDuty);
           /* if (stop == 1 || ref < 10 || ref > 50) {
                setPWM(50);
            } else {
                setPWM(newDuty);
            }
            switch (m) {
                case showMenu:
                    if(show){
                    printf("\e[1;1H\e[2J");
                    printf(
                        "*-----*Motor goes Vrum Vrum!*-----*\n\r"
                        "d - Alterar direção, r - Mudar velocidade\n\r"
                        "s - Mostrar só o último valor ou mostrar tudo\n\r"
                        "g - Parar ou continuar, m -  Mostrar menu\n\r");
                    show=0;
                    }
                    break;
                case showOneLine:
                    if(show){
                    printf("\e[1;1H\e[2J");
                    printf(
                        "T: %.1f, RPM:%.1f, Ref:%u, "
                        "erro:%.1f, u: %d, Direcao:"
                        "%d, Posicao:"
                        "%.1f\r",
                        time, rpm_average, ref, error, u, get_direction, ang_total);
                    show=0;
                    }
                    break;

                case setRPM:
                    if(show){
                    printf("\e[1;1H\e[2J");
                    printf("Insira o novo RPM:");
                    if (countNewRPM < 2) {
                        // PutChar(new[countNewRPM]);
                    }
                    // m = showOneLine;
                    show=0;
                    }
                    break;
                case stopMotor:
                    printf("\e[1;1H\e[2J");
                    printf("Motor Stopped");
                    break;
                case invalid:
                    printf("\e[1;1H\e[2J");
                    printf("RPM invalido");
                    break;

                default:
                    break;
            }

            switch (c) {
                case 'd':
                    toggle = !toggle;
                    stop = 0;

                    break;
                case 's':
                    show=1;
                    if (m == showAll) {
                        m = showOneLine;
                    } else if (m == showOneLine) {
                        m = showAll;
                    } else {
                        m = showOneLine;
                    }
                    stop = 0;

                    break;
                case 'v':
                    m = showOneLine;
                    stop = 0;

                    break;
                case 'm':
                    show=1;
                    m = showMenu;
                    stop = 0;

                    break;
                case 'r':
                    show=1;
                    m = setRPM;
                    countNewRPM = 0;
                    stop = 0;

                    break;
                case 'g':
                    if (m != stopMotor) {
                        m = stopMotor;
                        stop = 1;
                    } else {
                        integrator = 0;
                        stop = 0;
                        m = showOneLine;
                    }
                default:
                    break;
            }
            if (m == setRPM && c >= '0' && c <= '9') {
                // PutChar(c);
                char x = c - '0';
                new[countNewRPM] = x;
                countNewRPM++;
                if (countNewRPM == 2) {
                    //  m = showOneLine;
                    ref = new[0] * 10 + new[1];
                    if (ref < 10 || ref > 50) {
                        m = invalid;
                    } else {
                        m = showOneLine;
                        // integrator=0;
                    }
                    countNewRPM = 0;
                    show=1;
                }
            } */
            printf("rpm: %.1f\n\r",rpm_average);
            flagFrom10hz = 0;
        }
        
        switch (m) {
            case showAll:
                // printf("\e[1;1H\e[2J");
                printf(
                    "T: %.1f, RPM:%.1f, Ref:%u, "
                    "erro:%.1f, u: %d, Direcao:"
                    "%d, Posicao:"
                    "%.1f\n\r",
                    time, rpm_average, ref, error, u, get_direction, ang_total);
                break;
        }
    }
}
// void __ISR(_UART_1_VECTOR, IPL5AUTO) ISR_UART1(void) {
//     printf("time %.1f", time);
//     IFS0bits.U1RXIF = 0;
// }

void __ISR(_EXTERNAL_1_VECTOR, IPL2AUTO) ExtInt1ISR(void) {
    count_pulses++;
   // current_rpm =60.0/( ((long double)(TMR4) / freq_pre_scale) * 420.0) ;
    //long double oba= ((long double)TMR4/freq_pre_scale);
    //printf("\n\r oba: %f",oba);n
    newTMR=TMR4;
    TMR4 = 0;
    float angle = count_pulses * 360.0 / 420;

    ang_total = ang_total + angle;
    if (ang_total >= 360) {
        ang_total = 0;
    }
    // printf("countISR: %lu\n\r",count_pulses);
    //  printf("pulsos: %d",count_pulses);
    if (PORTDbits.RD2) {
        get_direction = 1;
        // printf("frente\n\r");
    } else {
        get_direction = -1;
        // printf("tras\n\r");
    }
    IFS0bits.INT1IF = 0;  // Reset interrupt flag
}
/* void __ISR(_TIMER_2_VECTOR, IPL4AUTO) T2Interrupt(void) {

 IFS0bits.T2IF = 0;
}*/
void __ISR(_TIMER_3_VECTOR, IPL3AUTO) T3Interrupt(void) {
    PORTEbits.RE7 = !PORTEbits.RE7;
    // flagToPrint;
    flagFrom10hz = 1;

    IFS0bits.T3IF = 0;
}
float mean_filter(float new_sample) {
    /* update the array of previous samples. The new sample is at position
     0 and the oldest sample is at position arr_size - 1 */
    uint32_t i;
    for (i = SAMPLES - 1; i > 0; i--) {
        arr[i] = arr[i - 1];
    }
    arr[0] = new_sample;  // update new sample read by the ADC

    /* compute the mean of all the samples to ouput */
    float avg_sample = 0;
    for (i = 0; i < SAMPLES; i++) {
        avg_sample += arr[i];
    }
    avg_sample /= SAMPLES;

    return avg_sample;
}