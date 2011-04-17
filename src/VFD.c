/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#include "VFD.h"

// Fuse bytes: (bit = 0 � programado)
// Low - nenhum bit programado - CKSEL+SUT+CKOUT+DIV8
// High - SPIEN e EESAVE programado
// Extended - Valor padr�o - � s� para bootloader
FUSES =
{
	.low = FUSE_CKSEL3,
	.high = (FUSE_SPIEN & FUSE_EESAVE & FUSE_BODLEVEL1 & FUSE_BODLEVEL0),
	.extended = EFUSE_DEFAULT,
};


static inline void HW_Init (void) __attribute__ ((always_inline));

int main (void)
{
	HW_Init();

	// enquanto a tens�o de entrada estiver abaixo de 8V e acima de 16V, n�o faz nada
	do
	{
		ADCSRA |= (1<<ADSC);
		while (ADCSRA & (1<<ADSC));

	} while ((ADC < MIN_VI_COUNT) || (ADC > MAX_VI_COUNT));

	VoltInp = ADC;

	ADMUX = ((1<<REFS0) | (1<<REFS1)) + VOUT_CH; 	// feedback do conversor boost
	ADCSRA |= (1<<ADIF) | (1<<ADIE);		// limpa o flag e habilita interrup��o do ADC

	tPIsat_params.w_kp_pu = eeprom_read_word(&ee_PI_kp);
	tPIsat_params.w_ki_pu = eeprom_read_word(&ee_PI_ki);
	tPIsat_params.sp = eeprom_read_word(&ee_PI_SP);

	ADCSRA |= (1<<ADSC);		// Inicia primeira leitura do ADC
	sei();


	uart_puts_P(PSTR("Inicio VFD\r"));
	vfd_setstring("Felipe Maimon");

	for(;;)
	{
		State_Machine();
	}
}


static inline void HW_Init (void)
{
	PORTC = 0x7C;	// Pull-ups on everything except ADC0 and ADC1;
	DDRC = 0;		// Everything is input;

	// PD4 as output - used as a trigger source for oscilloscope
	// PD5 as output - used to switch a load while debugging
	// PD6 (OC0A) as output.
	// All inputs with pull-up
	PORTD = (1<<PD3)| (1<<PD7);
	DDRD = (1<<PD2) | (1<<PD4) | (1<<PD5) | (1<<PD6);

	uart_init();
	vfd_init();

	// PS = 64 -> f = (18.432M/64) = 288 kHz -> ~20.6 ksps
	// Vref = Int 1.10V
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	ADMUX = ((1<<REFS0) | (1<<REFS1)) + VIN_CH; 	// ADC1	-> leitura da tens�o de entrada
	ADCSRA |= (1<<ADSC);					// Descarta a primeira leitura do ADC
	while (ADCSRA & (1<<ADSC));

	// Configura Timer 2 -> timeout da serial
	// PS = 1024 -> interrup��o a cada ~21.8 ms, sempre que o timer d� overflow
	// TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); -> s� ligar quando receber alguma coisa
	TIMSK2 = (1<<TOIE2);

	// Configura PWM
	// Tamb�m usa para gerar o refresh do max6921, com a interrup��o de overflow
	TCCR0A = (1<<COM0A1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS00);
}

