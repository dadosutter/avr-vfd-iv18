/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */
 
#include "max6921.h"
#include "VFD_segs.h"

// Just need to store the segs. The grid value will be generated automatically
static			uint8_t vfd_buffer[VFD_BUFFER_SIZE];
// vfd_buffer_length -> How many bytes there are in the buffer
static			uint8_t vfd_buffer_length;

static volatile	int8_t vfd_mux_first;
static volatile int8_t vfd_mux_last;
static volatile int8_t vfd_next_char;
static volatile uint16_t curgrid = (1 << (VFD_GRIDS - VFD_SPECIAL_CHAR - 1));
static			uint8_t vfd_scroll = VFD_SCROLLSPEED;
static volatile vfd_char CurChar;

void vfd_init(void)
{
	uint8_t i;

	// All pins, except MISO (PB4), as outputs. Pull-up on MISO;
	// PB0 is load, PB1 is blank -> MAX6921
	// Setup PB2 as a output, so I won't interfere with SPI
	VFD_PORT |= (1<<VFD_DO) | (1<<VFD_BLANK) ;
	VFD_DDR |= (1<<VFD_LOAD) | (1<<VFD_BLANK) | (1<<VFD_DI) | (1<<VFD_CLK) | (1<<VFD_SS);

	// Timer 1 overflow interrupt only when i need multiplexing
	// TIMSK1 = (1<<TOIE1);

	// Setup timer 1 for PWM on blank pin (PB1/OC1A)
	// 8 bit PWM - Inverting logic, so 0xFF is maximum brightness
	// PS = 8 - > Frequency is 9000 Hz
	OCR1AL = 0xFF;
	TCCR1A = (1<<COM1A1) | (1<<COM1A0) | (1<<WGM10);
	TCCR1B = (1<<WGM12) | (1<<CS11);

	// Configura SPI para modo 0 - Master - fck/32 -> 576 kHz
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
	SPSR = (1<<SPI2X);

	// Send 0 to max6921 3 times to clear it's shift register
	for(i = 0; i < 3; i++)
	{
		SPDR = 0;
		while(!(SPSR & (1<<SPIF)));
	}
	i = SPDR;		// Clear SPIF flag

	VFD_PORT |= (1<<VFD_LOAD);
	VFD_PORT &= ~(1<<VFD_LOAD);
}


void vfd_set(uint8_t segs, uint16_t grids)
{
	// disable the refresh interrupts
	TIMSK1 &= ~(1<<TOIE1);
	
	// Disable blank
	SET_PWM;

	CurChar.segs = segs;
	CurChar.grids = grids;

	// Start transmitting
	SPDR = CurChar.bytes[VFD_LAST_CHAR];
	SPCR |= (1<<SPIE);
}

void vfd_setchar(char data, uint16_t grids)
{
	switch (data)
	{
		case 'a'...'z':
			data -= 0x20;		// It's lower case, so make upper case
								// Don't break, as its now a valid char
		case '*'...'_':			// This is the whole range of valid chars
			data = pgm_read_byte(&VFD_Segs[data - '*']);
			break;

		default:
			data = 0x00;
			break;
	}

	vfd_set(data, grids);
}

void vfd_brightness(uint8_t duty)
{
	OCR1A = duty;

	// with OCR = 0, there is a narrow pulse on OC1A
	// so if is 0, sets to a regular low output 
	if (0 == duty)
	{
		SET_BLANK;
	}
	else
	{
		SET_PWM;
	}

}

void vfd_setstring(const char *str)
{
	uint8_t curchar;
	uint8_t length = 0;

	TIMSK1 &= ~(1<<TOIE1);

	while ((*str))
	{
		curchar = *(str++);

		if ((curchar == '.') && (length > 0))
		{
			vfd_buffer[length-1] += 1;
		}
		else
		{
			switch (curchar)
			{
				case 'a'...'z':
					curchar -= 0x20;	// It's lower case, so make upper case
										// Don't break, as its now a valid char
				case '*'...'_':			// This is the whole range of valid chars
					vfd_buffer[length] = pgm_read_byte(&VFD_Segs[curchar - '*']);
					break;

				default:
					vfd_buffer[length] = 0x00;
					break;
			}
			length++;
		}
	}

	vfd_mux_first = 0;
	vfd_mux_last = 7;
	vfd_next_char = 0;
	curgrid = (1 << (VFD_GRIDS - VFD_SPECIAL_CHAR - 1));

	if (length != 0)
	{
		vfd_buffer_length = length;

		// fill with blanks, so it won't change brightness with shorter strings
		for(curchar = length; curchar < 8; curchar++)
		{
			vfd_buffer[curchar] = 0;
		}	
	}

	// Enable refresh interrupts
	TIMSK1 = (1<<TOIE1);
}

void vfd_scrollspeed(uint8_t speed)
{
	vfd_scroll = speed;
}

// Executed at 9 kHz (Brightness PWM)
ISR(TIMER1_OVF_vect)
{
	static uint8_t ticks;
	static uint16_t scrl_ticks = VFD_SCROLLSPEED * VFD_SCRLTIMEBASE;

	ticks++;
	if (ticks > 8)		// The part inside the if is at 1 kHz
	{	
		ticks = 0;

		if ((vfd_buffer_length > 8) && (vfd_scroll != 0))
		{
			if ((scrl_ticks--) == 0)
			{
				scrl_ticks = vfd_scroll * VFD_SCRLTIMEBASE;

				vfd_mux_first++;
				if (vfd_mux_first > vfd_buffer_length)
				{
					vfd_mux_first = -(VFD_GRIDS - VFD_SPECIAL_CHAR - 1);
				}
				vfd_next_char = vfd_mux_first;
				vfd_mux_last = vfd_mux_first + 8 - 1;
				curgrid = (1 << (VFD_GRIDS - VFD_SPECIAL_CHAR - 1));
			}
		}

		if ((vfd_next_char < 0)||(vfd_next_char >= vfd_buffer_length))
		{
			CurChar.segs = 0;
		}
		else
		{
			CurChar.segs = vfd_buffer[vfd_next_char];
		}
		CurChar.grids = curgrid;

		vfd_next_char++;
		curgrid = curgrid >> 1;

		if (curgrid == 0)
		{
			curgrid = (1 << (VFD_GRIDS - VFD_SPECIAL_CHAR - 1));
		}

		if (vfd_next_char > vfd_mux_last)
		{
			vfd_next_char = vfd_mux_first;
		}

		// Start transmitting
		SPDR = CurChar.bytes[VFD_LAST_CHAR];
		SPCR |= (1<<SPIE);
	}
}

// Transmits 1 character to VFD
// This ISR is executed at the end of trasmission
ISR(SPI_STC_vect)
{
	static uint8_t i = VFD_LAST_CHAR;
	static uint8_t end;

	// If last trasmitted was the last one
	if (1 == end)
	{
		end = 0;

		// Pulse Load and disable interrupts
		VFD_PORT |= (1<<VFD_LOAD);
		VFD_PORT &= ~(1<<VFD_LOAD);
		SPCR &= ~(1<<SPIE);
	}
	else
	{
		// Trasmit next byte
		i--;
		SPDR = CurChar.bytes[i];
	
		// If this is the last to be trasmitted
		if (0 == i)
		{
			i = VFD_LAST_CHAR;
			end = 1;			// Set flag
		}
	}
}
