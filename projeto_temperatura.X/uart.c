/*
 * File:   uart.c
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
 *      2019-01-28: updated to MPLAB X IDE v5.\0 + XC32 v2.15
 */

/* Standard includes */
#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/* Kernel includes */
#include "uart.h"

#define SYSCLK 80000000L   // System clock frequency, in Hz
#define PBCLOCK 40000000L  // Peripheral Bus Clock frequency, in Hz

int UartInit(uint64_t pbclock, uint32_t br) {
    if (pbclock != 40000000L) return UART_PBCLOCK_NOT_SUP;  // Todo: add support to common pbclock values.

    // In the following are used the table values provided in the datasheet
    // PIC32MX Family Reference Manual, DS61107E-pages 21-14 and following
    switch (br) {
        case 115200:
            U1ABRG = 21;
            U1AMODEbits.BRGH = 0;
            break;

        case 9600:
            U1ABRG = 259;
            U1AMODEbits.BRGH = 0;
            break;
        default:
            return UART_BR_NOT_SUP;  // Baudrate not supported
    }

    // Common configuration settings
    U1AMODEbits.SIDL = 0;    // Continue operation in idle mode
    U1AMODEbits.IREN = 0;    // Disable Irda
    U1AMODEbits.UEN = 0;     // Only use TX and RX pints. No HW flow control
    U1AMODEbits.WAKE = 0;    // Wake -up on start bit disabled
    U1AMODEbits.LPBACK = 0;  // No loopback
    U1AMODEbits.ABAUD = 0;   // No autobaud
    U1AMODEbits.RXINV = 0;   // Idle logic value is 1
    U1AMODEbits.PDSEL = 0;   // 8 bit data, no parity
    U1AMODEbits.STSEL = 0;   // 1 stop bit
    U1STAbits.ADM_EN = 0;    // No automatic address detection
    U1STAbits.UTXISEL = 0;   // Interrupt when TX buffer has at least 1 empty position
    U1STAbits.UTXINV = 0;    // Idle logic value is 1

    // Configuration done. Enable.
    U1AMODEbits.ON = 1;
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    return UART_SUCCESS;
}
void uart1_config(uint32_t baudrate, uint8_t databits, uint8_t parity, uint8_t stopbits) {
    U1MODEbits.BRGH = 1;  // Divide o clock por 4
    if (baudrate == 9600 || baudrate == 115200) {
        U1BRG = PBCLOCK / (4 * baudrate) - 1;
    } else
        U1BRG = PBCLOCK / (4 * 9600) - 1;  // default baudrate 9600

    if (databits == 8) {
        if (parity == 0) {
            U1MODEbits.PDSEL = 0;  // 8 bit data, no parity
        } else if (parity == 1) {
            U1MODEbits.PDSEL = 2;  // 8 bit data, odd parity
        } else if (parity == 2) {
            U1MODEbits.PDSEL = 1;  // 8 bit data, even parity
        }
    } else if (databits == 9) {
        U1MODEbits.PDSEL = 3;  // 9 bit data, no parity
    } else {
        U1MODEbits.PDSEL = 2;
    }

    if (stopbits == 1 || stopbits == 2) {
        U1MODEbits.STSEL = stopbits - 1;
    } else {
        U1MODEbits.STSEL = 1;  // default 2 stop bit
    }

    U1STAbits.URXISEL = 0;  // Enable interrupts when a new character is
                            // available in the RX FIFO
    IEC0bits.U1RXIE = 1;    // Enable RX interrupts
    IPC6bits.U1IP = 5;      // Set interrupt priority

    U1STAbits.UTXEN = 1;  // Enable TX
    U1STAbits.URXEN = 1;  // Enable RX
    U1MODEbits.ON = 1;    // Enable UART
}

/********************************************************************
 * Function: 	UartClose()
 * Precondition:
 * Input: 		None
 * Output:		None.
 * Side Effects:	None.
 * Overview:     Closes UART connection.
 * Note:		 	No function currently
 ********************************************************************/
int UartClose(void) {
    U1AMODEbits.ON = 0;
    U1STAbits.UTXEN = 0;
    U1STAbits.URXEN = 0;
    return UART_SUCCESS;
}

/********************************************************************
 * Function: 	GetChar()
 * Precondition: UART initialized
 * Input: 		None
 * Output:		UART_SUCESS: If there is some data
 *              UART_FAIL: if there is no data.
 * Side Effects:	None.
 * Overview:     Gets the data from UART RX FIFO.
 * Note:		 	None.
 ********************************************************************/
char GetChar() {
    char dummy;

    if (U1STAbits.OERR || U1STAbits.FERR || U1STAbits.PERR)  // receive errors?
    {
        dummy = U1RXREG;     // dummy read to clear FERR/PERR
        U1STAbits.OERR = 0;  // clear OERR to keep receiving
    }

    if (U1STAbits.URXDA) {
        return U1ARXREG;  // get data from UART RX FIFO
    }
}

/********************************************************************
 * Function: 	PutChar()
 * Precondition:
 * Input: 		None
 * Output:		None
 * Side Effects:	None.
 * Overview:     Puts the data into UART tx reg for transmission.
 * Note:		 	None.
 ********************************************************************/
void PutChar(uint8_t txChar) {
    while (U1STAbits.UTXBF)
        ;  // wait for TX buffer to be empty
    U1ATXREG = txChar;
}
/***************************************End Of
 * File*************************************/
