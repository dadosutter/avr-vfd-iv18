#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define USART_BAUD_RATE				115200UL
#define MY_UBRR						(((F_CPU / (USART_BAUD_RATE * 16UL))) - 1) 

/** Size of the circular receive buffer, must be power of 2 */
#define UART_RX_BUFFER_SIZE		32
#define UART_RX_BUFFER_MASK		( UART_RX_BUFFER_SIZE - 1)

#define UART_PORT	PORTD
#define UART_DDR	DDRD
#define RX			PD0
#define TX			PD1

void 				uart_init(void);
char	 			uart_getc(void);
void 				uart_putc(unsigned char data);
void 				uart_puts(const char *s);
void				uart_puts_P(const char *s );
unsigned char		uart_rx_buffer_empty(void);

#endif
