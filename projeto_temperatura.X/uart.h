/* 
 * File:   uart.h
 * Author: Paulo Pedreiras
 *
 * Created on Jan 28, 2019
 * MPLAB X IDE v5.10 + XC32 v2.15
 *
 * Target: Digilent chipKIT MAx32 board 
 * 
 * Overview:
 *          Set of functions to handle the UART       
 
 * Notes: Partially based on the bootloader code from Microchip
 * 
 * Revisions:
 *      2017-10-25: initial release
 *      2019-01-28: update to MPLAB X IDE v5.\0 + XC32 v2.15
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

/* Define return codes */
#define UART_SUCCESS 0
#define UART_FAIL -1;
#define UART_BR_NOT_SUP -2
#define UART_PBCLOCK_NOT_SUP -3

/* Define prototypes (public interface) */
int UartInit(uint64_t pbclock, uint32_t br);
void uart1_config(uint32_t baudrate, uint8_t databits, uint8_t parity, uint8_t stopbits);
int UartClose(void);
char GetChar();
void PutChar(uint8_t txChar);

#endif
