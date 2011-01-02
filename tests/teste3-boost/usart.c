#include "usart.h"

static volatile unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;

void uart_init(void)
{
	// Setup port
	UART_PORT |= (1<<RX);
	UART_DDR |= (1<<TX);

    UART_RxHead = 0;
    UART_RxTail = 0;
    
    /*Set baud rate */
    UBRR0H = (uint8_t)(MY_UBRR >> 8);
    UBRR0L = (uint8_t)(MY_UBRR);
    
    /*Enable receiver and transmitter plus corresponding interrupts*/
    UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
    
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
}

/*************************************************************************
Function: uart_getc()
Purpose:  return byte from ringbuffer or wait for one if there is none
Returns:  received byte from ringbuffer or 0 if there was a timeout
**************************************************************************/
char uart_getc(void)
{
    unsigned char tmptail;

	// Wait for new data or timeout
    while (UART_RxHead == UART_RxTail);
	
    /* calculate /store buffer index */
    tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART_RxTail = tmptail; 

	// Just for debugging
//	uart_putc(UART_RxBuf[tmptail]);

    return UART_RxBuf[tmptail];
}

/*************************************************************************
Function: uart_putc()
Purpose:  transmit a byte via UART (wait for it to be avaiable if it isn't)
Input:    byte to be transmitted
Returns:  none          
**************************************************************************/
void uart_putc(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}


/*************************************************************************
Function: uart_puts()
Purpose:  transmit string to UART
Input:    string to be transmitted
Returns:  none          
**************************************************************************/
void uart_puts(const char *s )
{
	while (*s) 
		uart_putc(*s++);
}


/*************************************************************************
Function: uart_puts_P()
Purpose:  transmit string from flash to UART
Input:    string to be transmitted
Returns:  none          
**************************************************************************/
void uart_puts_P(const char *s )
{
	while (pgm_read_byte(s) != 0x00)
		uart_putc(pgm_read_byte(s++));

}

/*************************************************************************
Function: uart_buffer_empty()
Purpose:  check if receive buffer is empty
Input:    none
Returns:  0 if RX buffer has data, 1 if RX buffer is emtpy
**************************************************************************/
unsigned char uart_rx_buffer_empty(void)
{
    if ( UART_RxHead == UART_RxTail ) {
        return 1;   /* no data available */
    }
	else {
		return 0;
	}
}/*uart_rx_buffer_empty*/


ISR(USART_RX_vect)
{
	unsigned char tmphead;
	unsigned char byte;
 
	/* read UART status register and UART data register */ 
	byte = UDR0;
      
	/* calculate buffer index */ 
	tmphead = ( UART_RxHead + 1) & UART_RX_BUFFER_MASK;
    
	/* store new index */
	UART_RxHead = tmphead;
	/* store received data in buffer */
	UART_RxBuf[tmphead] = byte;
}
