/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#include "timeout.h"

volatile uint8_t TOTicks;
volatile uint8_t TimeOut;

// Gera os timeouts para a porta serial
ISR(TIMER2_OVF_vect)
{
	if (TOTicks++ == TO_TICKS)
	{
		TOTicks = 0;
		TimeOut	= TRUE;
	}
}
