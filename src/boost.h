/******************************************************************************

	Most of the code below is from Artur (adebeun2) from AVRFreaks.net. I've 
	modified it a little bit to suit my needs

	http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=241636#241636

	Felipe Maimon

******************************************************************************/

#ifndef _BOOST_H_
#define _BOOST_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "VFD.h"

#define VOUT_CH			0
#define VIN_CH			1

// Tensões máximas e mínimas de entrada
#define VI_CONV_FACT	(12.0 * 1024) / (12 + 330) / 1.1
#define MIN_VI_COUNT	(uint16_t)(8 * VI_CONV_FACT + 0.5)
#define MAX_VI_COUNT	(uint16_t)(16 * VI_CONV_FACT + 0.5)

// Tensão de saída
#define VO_CONV_FACT	(3.9 * 1024UL) / (3.9 + 330) / 1.1

/* typedefs */
typedef struct /* saturating PI controller coefficients */
{
  volatile int16_t w_kp_pu;   /* per-unit proportional gain term. must be limited between 0 and +(4PU-1) */
  volatile int16_t w_ki_pu;   /* per-unit integral gain term. must be limited between 0 and +(4PU-1) */
  int32_t l_integrator_dpu;   /* accumulator for discrete-time integrator. must be limited to +/-(16DPU-1) */
  volatile int16_t sp;
  volatile int16_t pv;
} tPIsat;

#define RENORMALISE         (13)                /* exponent to renormalise after pu calculation */
#define ONE_PU              (8192)              /* = 2^13 */
#define HALF_PU_MINUS_1     (0x0FFF)
#define TWO_PU_MINUS_1      (0x3FFF)
#define THREE_PU_MINUS_1    (0x5FFF)
#define FOUR_PU_MINUS_1     (0x7FFF)
#define TWO_DPU_MINUS_1     (0x07FFFFFF)
#define THREE_DPU_MINUS_1   (0x0BFFFFFF)
#define FOUR_DPU_MINUS_1    (0x0FFFFFFF)
#define EIGHT_DPU_MINUS_1   (0x1FFFFFFF)
#define TWELVE_DPU_MINUS_1  (0x2FFFFFFF)
#define SIXTEEN_DPU_MINUS_1 (0x3FFFFFFF)

/* return x limited to between ll and ul */
#define sat_l(x, ll, ul) ( (x) >= (int32_t)(ul) ) ? (int32_t)(ul) : ( ( (x) <= (int32_t)(ll) ) ? (int32_t)(ll) : (x) )
#define dtos(x) ( ( (x) << ( 16 - RENORMALISE) ) >> 16 ) 
#define stod(x) ( (int32_t)(x) << RENORMALISE ) 


// Variables
extern tPIsat tPIsat_params;
extern volatile uint16_t VoltInp;

extern uint16_t EEMEM ee_PI_SP;
extern uint16_t EEMEM ee_PI_kp;
extern uint16_t EEMEM ee_PI_ki;

#endif
