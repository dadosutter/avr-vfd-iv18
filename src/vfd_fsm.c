/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Felipe Maimon wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Felipe Maimon
 * ----------------------------------------------------------------------------
 */

#include "vfd_fsm.h"


// Só são usados aqui
enum estados CurState = SM_IDLE;

uint8_t TipoErro;
uint8_t Counter;
uint16_t tempu16;
uint8_t tempu8;
char buffer[33];		// 32 bytes + \0 terminador

void State_Machine (void)
{
	uint8_t uart_tmp;

	// Início da máquina de estados para leitura da serial
		
	// Se deu timeout
	if (TimeOut)
	{
		CurState = SM_ERROR;
		TipoErro = CB_ERROR_TO;
		TOTicks = 0;
		TimeOut = FALSE;
	}

	// Se tiver algo no buffer, executa a máquina de estados.
	else if (CurState != SM_IDLE)
	{
		// Reseta os timers e contador de timeout para a serial
		TCNT2 = 0;					// Zera o timer 2
		GTCCR |= (1<<PSRASY);		// Zera o prescaler do timer2
		TOTicks = 0;
		TimeOut = FALSE;
	}

	switch (CurState)
	{
		case SM_IDLE:
			if (uart_rx_buffer_empty() == 0)		// Se tem alguma coisa no buffer
			{
				CurState = SM_START;
				TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);	// Inicia Timer2 p/ Timeout
			}
			break;

		// Confere se o primeiro byte é 'b', já que todos os comandos começam com ele
		case SM_START:
			
			switch (uart_getc())
			{
				case CB_INICIO:
					CurState = SM_GET_CMD;
					break;

				case VFD_INICIO:
					CurState = SM_VFD_CMD;
					break;

				default:
					CurState = SM_ERROR;
					TipoErro = CB_ERROR_START;
					break;
			}
			break;
				
		// Segundo byte - byte de comando
		// Verifica se é leitura ou escrita
		case SM_GET_CMD:

			switch (uart_getc())
			{
				case CB_CMD_RD:
					CurState = SM_READ;
					break;

			// Se for escrita, também inicializa as variáveis para
			// leitura dos novos valores
				case CB_CMD_WR:
					CurState = SM_WRITE;
					Counter = 0;
					tempu16 = 0;
					break;

				default:
					CurState = SM_ERROR;
					TipoErro = CB_ERROR_CMD;
					break;
			}
			break;

		// Veririfica qual variável se quer escrever
		// Atualmente só pode mexer no set-point
		case SM_WRITE:
			switch (uart_getc())
			{
				case CB_WR_SP:
					CurState = SM_WR_SP;
					break;

				case CB_WR_KP:
					CurState = SM_WR_KP;
					break;

				case CB_WR_KI:
					CurState = SM_WR_KI;
					break;

				case CB_WR_LD:
					CurState = SM_WR_LD;
					break;

				default:
					CurState = SM_ERROR;
					TipoErro = CB_ERROR_WR;
					break;
			}
			break;

		// Lê os próximos 3 bytes que terão o valor do novo set point
		case SM_WR_SP:
			uart_tmp = uart_getc();

			// só aceita se for número
			if ((uart_tmp >='0') && (uart_tmp <= '9'))
			{
				tempu16 *= 10;
				tempu16 += uart_tmp - '0';		// converte para valor numérico
				Counter++;

				if (Counter > 2)				// Se já leu os 3 bytes
				{
					// verifica se está dentro da faixa aceitável
					if ((tempu16 >= 80 ) && (tempu16 <= 240))
					{
						// Converte e salva o valor do set-point novo
						// Teoricamente deveria multiplicar por 3,92, mas como
						// não dá, faz uma leve gambiarra para somente utilizar 
						// números inteiros.
						tempu16 = ((tempu16 * 136 + 25) / 50);

						ATOMIC_BLOCK(ATOMIC_FORCEON)
						{
							tPIsat_params.sp = tempu16;
						}

						OSC_TRIGGER;

						eeprom_write_word(&ee_PI_SP, tempu16);

						CurState = SM_END;
					}
					else 
					{
						CurState = SM_ERROR;
						TipoErro = CB_ERROR_SP;
					}
				}
			}
			else
			{
				CurState = SM_ERROR;
				TipoErro = CB_ERROR_SP;
			}
			break;

		// Lê os próximos 4 bytes que terão o valor do novo KP ou KI
		case SM_WR_KP: case SM_WR_KI:
			uart_tmp = uart_getc();

			// só aceita se for número
			if ((uart_tmp >='0') && (uart_tmp <= '9'))
			{
				tempu16 *= 10;
				tempu16 += uart_tmp - '0';		// converte para valor numérico
				Counter++;

				if (Counter > 3)				// Se já leu os 3 bytes
				{
					ATOMIC_BLOCK(ATOMIC_FORCEON)
					{
						if (SM_WR_KP == CurState)
						{
							tPIsat_params.w_kp_pu = tempu16;
						}
						else
						{
							tPIsat_params.w_ki_pu = tempu16;
						}
						tPIsat_params.l_integrator_dpu = 0;
					}

					if (SM_WR_KP == CurState)
					{
						eeprom_write_word(&ee_PI_kp, tempu16);
					}
					else
					{
						eeprom_write_word(&ee_PI_ki, tempu16);
					}

					CurState = SM_END;
				}
			}
			else
			{
				CurState = SM_ERROR;
				TipoErro = CB_ERROR_SP;
			}
			break;

		// Just toggle PD5
		case SM_WR_LD:
			OSC_TRIGGER;

			PIND |= (1<<PD5);			// Toggle PD5
			CurState = SM_END;
			break;

		// Em caso de leitura, verifica qual o valor que se quer ler
		case SM_READ:
			switch (uart_getc())
			{
				case CB_RD_SP:
					sprintf(buffer, "%03d", (tPIsat_params.sp * 50 + 68) / 136);
					break;

				case CB_RD_KP:
					sprintf(buffer, "%04d", tPIsat_params.w_kp_pu);
					break;

				case CB_RD_KI:
					sprintf(buffer, "%04d", tPIsat_params.w_ki_pu);
					break;

				case CB_RD_VI:
					sprintf(buffer, "%03d", (VoltInp * 10) / (uint8_t)(VI_CONV_FACT + 0.5) );
					break;

				case CB_RD_VO:
					ATOMIC_BLOCK(ATOMIC_FORCEON)
					{
						tempu16 = tPIsat_params.pv;
					}					
					sprintf(buffer, "%03d", (tempu16 * 10) / (uint8_t)(VO_CONV_FACT + 0.5));
					break;
				
				default:
					CurState = SM_ERROR;
					TipoErro = CB_ERROR_RD;
					break;
			}

			if (CurState != SM_ERROR)
			{
				uart_puts(buffer);
				CurState = SM_END;
			} 
			break;

		case SM_VFD_CMD:
			Counter = 0;
			tempu16 = 0;
			tempu8 = 0;
			switch (uart_getc())
			{
				case VFD_SET:
					CurState = SM_VFD_SET; 
					break;
				
				case VFD_SETCHAR:
					CurState = SM_VFD_SETCHAR;
					break;

				case VFD_CLEAR:
					vfd_clear();
					CurState = SM_END;
					break;

				case VFD_ALL:
					vfd_setall();
					CurState = SM_END;
					break;

				case VFD_STRING:
					CurState = SM_VFD_STR;
					break;

				case VFD_BRT:
					CurState = SM_VFD_BRT;
					break;

				case VFD_SCRL:
					CurState = SM_VFD_SCRL;
					break;
			}
			break;

		case SM_VFD_SET:
			uart_tmp = uart_getc();

			switch (uart_tmp)
			{
				case 'a'...'f':
					uart_tmp -= 0x20;
				case 'A'...'F':
					uart_tmp -= 'A';
					uart_tmp += 10;
					break;
				case '0'...'9':
					uart_tmp -= '0';
					break;
				default:
					CurState = SM_ERROR;
					TipoErro = VFD_ERROR_VAL;
					break;
			}
			if (Counter++ < 2)
			{
				tempu8 *= 16;
				tempu8 += uart_tmp;
			}
			else 
			{
				tempu16 *= 16;
				tempu16 += uart_tmp;

				if (Counter > 4)
				{
					if (tempu16 > 0x1FF)
					{
						CurState = SM_ERROR;
						TipoErro = VFD_ERROR_VAL;
					}
					else
					{
						vfd_set(tempu8, tempu16);
						CurState = SM_END;
					}
				}
			}
			break;

		case SM_VFD_SETCHAR:
			uart_tmp = uart_getc();

			if (Counter++ == 0)
			{
				tempu8 = uart_tmp;
			}
			else
			{
				switch (uart_tmp)
				{
					case 'a'...'f':
						uart_tmp -= 0x20;
					case 'A'...'F':
						uart_tmp -= 'A';
						uart_tmp += 10;
						break;
					case '0'...'9':
						uart_tmp -= '0';
						break;
					default:
						CurState = SM_ERROR;
						TipoErro = VFD_ERROR_VAL;
						break;
				}

				tempu16 *= 16;
				tempu16 += uart_tmp;

				if (Counter > 3)
				{
					if (tempu16 > 0x1FF)
					{
						CurState = SM_ERROR;
						TipoErro = VFD_ERROR_VAL;
					}
					else
					{
						vfd_setchar(tempu8, tempu16);
						CurState = SM_END;
					}
				}
			}
			break;

		case SM_VFD_BRT:
			uart_tmp = uart_getc();

			switch (uart_tmp)
			{
				case 'a'...'f':
					uart_tmp -= 0x20;
				case 'A'...'F':
					uart_tmp -= 'A';
					uart_tmp += 10;
					break;
				case '0'...'9':
					uart_tmp -= '0';
					break;
				default:
					CurState = SM_ERROR;
					TipoErro = VFD_ERROR_VAL;
					break;
			}
			tempu8 *= 16;
			tempu8 += uart_tmp;
			Counter++;

			if (Counter > 1)
			{
				vfd_brightness(tempu8);
				CurState = SM_END;
			}
			break;

		case SM_VFD_SCRL:
			uart_tmp = uart_getc();

			switch (uart_tmp)
			{
				case 'a'...'f':
					uart_tmp -= 0x20;
				case 'A'...'F':
					uart_tmp -= 'A';
					uart_tmp += 10;
					break;
				case '0'...'9':
					uart_tmp -= '0';
					break;
				default:
					CurState = SM_ERROR;
					TipoErro = VFD_ERROR_VAL;
					break;
			}
			tempu8 *= 16;
			tempu8 += uart_tmp;
			Counter++;

			if (Counter > 1)
			{
				vfd_scrollspeed(tempu8);
				CurState = SM_END;
			}
			break;

		case SM_VFD_STR:
			uart_tmp = uart_getc();

			if (uart_tmp == 0x0D)		// 0x0D = Enter
			{
				buffer[Counter] = 0;

				vfd_setstring(buffer);
				CurState = SM_END;
			}
			else
			{
				buffer[Counter] = uart_tmp;
				Counter++;

				if (Counter > 32)
				{
					CurState = SM_ERROR;
					TipoErro = VFD_ERROR_STR;
				}
			}
			break;

		case SM_END:
			TCCR2B = 0;					// Para Timer2
			uart_putc(CB_FINAL_MSG);
			CurState = SM_IDLE;
			break;

		case SM_ERROR:
			TCCR2B = 0;					// Para Timer2
			uart_putc(CB_ERROR_MSG);
			uart_putc(TipoErro + '0');
			CurState = SM_IDLE;
			break;

	}
}
