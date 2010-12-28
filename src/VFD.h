/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#ifndef _VFD_H_
#define _VFD_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "timeout.h"
#include "usart.h"
#include "vfd_fsm.h"
#include "boost.h"
#include "max6921.h"

#define FALSE	0
#define TRUE	1

// Pulse on PD4, oscilloscope trigger
#define OSC_TRIGGER 	PORTD |= (1<<PD4); PORTD &= ~(1<<PD4)

int main (void) __attribute__((OS_main));

#endif
