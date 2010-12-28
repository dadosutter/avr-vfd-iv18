/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#ifndef _VFD_SEGS_H_
#define _VFD_SEGS_H_

// You need to define the segments for your VFD for each one of these chars
// Use 0 (zero) for the ones that cannot describe

// These are for the IV-18 VFD. They only have 7 segs + 1 dp, so only an
// uint8_t is needed. Change the data type as needed. You might need to 
// change the vfd_setstring to handle different data sizes.
const uint8_t VFD_Segs[] PROGMEM = 
{
	0x00,		// '*' (Not defined)
	0x00,		// '+' (Not defined)
	0x01,		// ',' (Same as '.')
	0x02,		// '-'
	0x01,		// '.'
	0x00,		// '/' (Not defined)
	0xFC,		// '0'
	0x60,		// '1'
	0xDA,		// '2
	0xF2,		// '3
	0x66,		// '4
	0xB6,		// '5
	0xBE,		// '6
	0xE0,		// '7
	0xFE,		// '8
	0xE6,		// '9'
	0x00,		// ':' (Not defined)
	0x00,		// ';' (Not defined)
	0x00,		// '<' (Not defined)
	0x00,		// '=' (Not defined)
	0x00,		// '>' (Not defined)
	0x00,		// '?' (Not defined)
	0x00,		// '@' (Not defined)
	0xEE,		// 'A' (+ 'a')
	0x3E,		// 'B' (+ 'b')
	0x9C,		// 'C' (+ 'c')
	0x7A,		// 'D' (+ 'd')
	0x9E,		// 'E' (+ 'e')
	0x8E,		// 'F' (+ 'f')
	0xF6,		// 'G' (+ 'g')
	0x6E,		// 'H' (+ 'h')
	0x60,		// 'I' (+ 'i')
	0x78,		// 'J' (+ 'j')
	0xAE,		// 'K' (+ 'k')
	0x1C,		// 'L' (+ 'l')
	0xAA,		// 'M' (+ 'm')
	0x2A,		// 'N' (+ 'n')
	0xFC,		// 'O' (+ 'o')
	0xCE,		// 'P' (+ 'p')
	0xE6,		// 'Q' (+ 'q')
	0x8C,		// 'R' (+ 'r')
	0xB6,		// 'S' (+ 's')
	0x1E,		// 'T' (+ 't')
	0x7C,		// 'U' (+ 'u')
	0x54,		// 'V' (+ 'v')
	0x56,		// 'W' (+ 'w')
	0x92,		// 'X' (+ 'x')
	0x76,		// 'Y' (+ 'y')
	0xDA,		// 'Z' (+ 'z')
	0x00,		// '[' (Not defined)
	0x00,		// '\' (Not defined)
	0x00,		// ']' (Not defined)
	0x00,		// '^' (Not defined)
	0x10		// '_'
};

#endif
