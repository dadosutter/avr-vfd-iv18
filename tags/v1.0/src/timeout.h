/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "VFD.h"

// Defines para timeout da serial
#define TO_TICKS					(5000 / 22)		// 5000 ms

extern volatile uint8_t TOTicks;
extern volatile uint8_t TimeOut;

#endif
