/******************************************************************************

	Most of the code below is from Artur (adebeun2) from AVRFreaks.net. I've 
	modified it a little bit to suit my needs.

	http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=241636#241636

	Felipe Maimon

******************************************************************************/

#include "boost.h"

// Variáveis globais do controle PI
		 tPIsat  tPIsat_params;
volatile uint16_t VoltInp;

uint16_t EEMEM ee_PI_SP = 30 * VO_CONV_FACT;
uint16_t EEMEM ee_PI_kp = 3000;
uint16_t EEMEM ee_PI_ki = 200;

static __inline__ int16_t fw_PI_controller (const int16_t w_error_pu,
											tPIsat *tPIsat_params,
											const int16_t w_ll,
											const int16_t w_ul) __attribute__ ((always_inline));


/* saturating PI controller */
static inline int16_t fw_PI_controller (const int16_t w_error_pu,
              		                  	tPIsat *tPIsat_params,
                    			      	const int16_t w_ll,
                         				const int16_t w_ul)
{
  int32_t l_temp_dpu;                 /* temporary variable */
  int32_t l_proportional_dpu;         /* proportional term, error(k)*Kp */
  int32_t l_integral_dpu;             /* integral term, error(k)*Kp*Ki* */

  /* calculate the proportional term: error(k) * Kp [DPU]
     NOTE: error(k) MUST have been saturated to +/-(4PU-1) BEFORE calling this routine */
  l_proportional_dpu = (int32_t)tPIsat_params->w_kp_pu * (int32_t)w_error_pu; /* (1) */
  /* saturate to +/-(4DPU-1), to allow for rescaling to single precision */
  l_proportional_dpu = sat_l(l_proportional_dpu, -FOUR_DPU_MINUS_1, FOUR_DPU_MINUS_1); /* (1) */

  /* calculate the integrator input term: error(k) * Kp * [DPU] */
  l_temp_dpu = (int32_t)tPIsat_params->w_ki_pu * dtos(l_proportional_dpu); /* (2) */
  /* saturate to +/-(8DPU-1), to prevent the integrator from overflowing */
  l_temp_dpu = sat_l(l_temp_dpu, -EIGHT_DPU_MINUS_1, EIGHT_DPU_MINUS_1); /* (2) */

  /* calculate the integral term: error(k)*Kp*Ki*(1+1/z)/(1-1/z). Because the integrator
     is saturated between +/-(16DPU-1), this term is bounded between +/-24DPU */
  l_integral_dpu = l_temp_dpu + tPIsat_params->l_integrator_dpu; /* (3) */

  /* update the integrator: integrator(k) = integrator(k-1) + 2*error(k)*Kp*Ki
     as l_temp_DPU is saturated to +/-(8DPU-1) this fits into a long word
     if the integrator is limited to +/-(16DPU-1) */
  tPIsat_params->l_integrator_dpu = l_integral_dpu + l_temp_dpu; /* (4) */

  /* saturate the integrator to +/-16DPU */
  tPIsat_params->l_integrator_dpu = sat_l(tPIsat_params->l_integrator_dpu, -SIXTEEN_DPU_MINUS_1, SIXTEEN_DPU_MINUS_1); /* (4) */

  /* calculate the output term: proportional(k) + integral(k) [DPU]
     this is bounded between +/-28DPU */
  l_temp_dpu = l_proportional_dpu + l_integral_dpu; /* (5) */
  /* saturate the output term between the lower limit and the upper limit */
  l_temp_dpu = sat_l(l_temp_dpu, stod(w_ll), stod(w_ul));
  l_temp_dpu = dtos(l_temp_dpu);

  /* assign it to the output variable */
  return((int16_t)l_temp_dpu);
} 

// Inicia nova conversão e faz o controle PI
ISR(ADC_vect)
{
	static uint8_t InpCounter;
	int16_t error;

	if (InpCounter++ == 0)
	{
		ADMUX = ((1<<REFS0) | (1<<REFS1)) + VOUT_CH;
		VoltInp = ADC;
		ADCSRA |= (1<<ADSC);
	}
	else
	{
		if (InpCounter > 250)
		{
			ADMUX = ((1<<REFS0) | (1<<REFS1)) + VIN_CH;
			InpCounter = 0;
		}

		ADCSRA |= (1<<ADSC);

		tPIsat_params.pv = ADC;
		error =  tPIsat_params.sp - tPIsat_params.pv;
		OCR0A = fw_PI_controller (error, &tPIsat_params, 0, 180);
	}
}
