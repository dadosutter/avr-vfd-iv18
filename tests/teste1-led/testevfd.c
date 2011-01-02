#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main (void)
{
	PORTC = 0x7C;	// Pull-ups on everything except ADC0 and ADC1;
	DDRC = 0;		// Everything is input;

	DDRD = (1<<PD6)|(1<<PD7);

	while(1)
	{
		PORTD ^= (1<<PD7);
		
		_delay_ms(1000);
	}
}
