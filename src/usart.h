/******************************************************************************

	This code came from Atmel's application note AVR341, ported to gcc and
	modified by me. So I think the Beerware license applies.
	
	"THE BEER-WARE LICENSE" (Revision 42):
	Felipe Maimon wrote this file. As long as you retain this notice you
	can do whatever you want with this stuff. If we meet some day, and you think
	this stuff is worth it, you can buy me a beer in return.

	Felipe Maimon

******************************************************************************/

#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "VFD.h"
#include "timeout.h"

#define USART_BAUD_RATE				115200UL
#define MY_UBRR						(((F_CPU / (USART_BAUD_RATE * 16UL))) - 1) 

/** Size of the circular receive buffer, must be power of 2 */
#define UART_RX_BUFFER_SIZE		32
#define UART_RX_BUFFER_MASK		( UART_RX_BUFFER_SIZE - 1)

#define RX			PD0
#define TX			PD1

void 				uart_init(void);
char	 			uart_getc(void);
void 				uart_putc(unsigned char data);
void 				uart_puts(const char *s);
void				uart_puts_P(const char *s );
unsigned char		uart_rx_buffer_empty(void);

#endif
