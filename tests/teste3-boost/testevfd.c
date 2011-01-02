/*
	Programa de testes. Testa o conversor boost.
	
	Com esse programa carregado no AVR, o led deverá piscar em 0.5Hz, ou seja,
	1s aceso e 1s apagado.

	Com um terminal aberto, todos os caracteres enviados serão ecoados de volta.
	
	Insira os valores de VI_CONV e VO_CONV lidos no teste anterior nos defines
	abaixo.

	
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

#define VOUT	0		// Canal do ADC da tensão de saída (normalmente 0)
#define VIN		1		// Canal do ADC da tensão de entrada (normalmente 1)

#define VI_MED	10.0	// Tensão de entrada medida
#define VI_CONV 326		// Leitura do ADC para esse medida

#define VO_MED	57.3	// Tensão de saída medida
#define VO_CONV 632		// Leitura do ADC para essa medida

// Tensão máxima de saída = 65 V
#define VO_MAX	(uint16_t)(65.0 * VO_CONV / VO_MED)

// Fatores de conversão
#define VI_CONV_FACTOR (uint16_t)((VI_MED * 10 * 256 / VI_CONV) + 0.5)
#define VO_CONV_FACTOR (uint16_t)((VO_MED * 10 * 256 / VO_CONV) + 0.5)

char buffer[10];

/*
 NAME:      | IntToStr
 PURPOSE:   | Converts an integer value to ASCII, with leading zeros
 ARGUMENTS: | Integer value (range 0-999), pointer to destination string buffer
 RETURNS:   | None
*/
void IntToStr(uint16_t IntV, char *Buff)
{	
	uint8_t Temp = 0;

	while (IntV >= 100)
	{
		Temp++;
		IntV -= 100;
	}

	Buff[0] = '0' + Temp;
	
	Temp = 0;
	
	while (IntV >= 10)
	{
		Temp++;
		IntV -= 10;
	}
		
	Buff[1] = '0' + Temp;
	Buff[2] = '0' + IntV;
	Buff[3] = '\0';
}


int main (void)
{
	char c;
	uint32_t tmp;

	PORTC = 0;	// Pull-ups on everything except ADC0 and ADC1;
	DDRC = 0;		// Everything is input;

	PORTD = (1<<PD7);
	DDRD = (1<<PD6)|(1<<PD7);

	uart_init();

	// PS = 64 -> f = (18.432M/64) = 288 kHz -> ~20.6 ksps
	// Vref = Int 1.10V
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	ADMUX = ((1<<REFS0) | (1<<REFS1)) + 1; 	// ADC1	-> leitura da tensão de entrada
	ADCSRA |= (1<<ADSC);					// Descarta a primeira leitura do ADC
	while (ADCSRA & (1<<ADSC));

	OCR1A = 0x464F;
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);

	OCR0A = 5;
	TCCR0A = (1<<COM0A1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS00);

	TIMSK1 = (1<<OCIE1A);

	sei();

	uart_puts("Teste VFD\r");

	while(1)
	{
		c = uart_getc();
		uart_putc(c);

		ADMUX = ((1<<REFS0) | (1<<REFS1)) + VOUT;
		ADCSRA |= (1<<ADSC);
		while (ADCSRA & (1<<ADSC));

		if (ADC > VO_MAX)
		{
			OCR0A = 5;

			uart_puts("Ops. Tensao alta\r");
		}


		switch (c)
		{
			case '0':
				ADMUX = ((1<<REFS0) | (1<<REFS1)) + VOUT;

				ADCSRA |= (1<<ADSC);
				while (ADCSRA & (1<<ADSC));

				tmp = ADC;
				IntToStr(tmp, buffer);
				uart_putc('-');
				uart_puts(buffer);
				uart_putc('\r');

				tmp *= VO_CONV_FACTOR;
				IntToStr(tmp / 256, buffer);
				uart_puts(buffer);
				uart_putc('\r');
				break;
			
			case '1':
				ADMUX = ((1<<REFS0) | (1<<REFS1)) + VIN;

				ADCSRA |= (1<<ADSC);
				while (ADCSRA & (1<<ADSC));

				tmp = ADC;
				IntToStr(tmp, buffer);
				uart_putc('-');
				uart_puts(buffer);
				uart_putc('\r');

				tmp *= VI_CONV_FACTOR;
				IntToStr(tmp / 256, buffer);
				uart_puts(buffer);
				uart_putc('\r');
				break;

			case 'a':
				if (OCR0A < 50)
					OCR0A++;
				break;
			
			case 'z':
				if (OCR0A > 0)
					OCR0A--;
				break;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	PIND = (1<<PD7);	// toggle PD7
}
