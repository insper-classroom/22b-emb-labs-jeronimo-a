/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define OLED_LED1_PIO				PIOA						// periferico que controla o LED 1 no modulo OLED
#define OLED_LED1_PIO_ID			ID_PIOA						// ID do periferico PIOA
#define OLED_LED1_PIO_IDX			0							// ID do pino conectado ao LED 1 do modulo OLED
#define OLED_LED1_PIO_IDX_MASK		(1 << OLED_LED1_PIO_IDX)	// mascara para controlarmos o LED 1 do modulo OLED

#define OLED_LED2_PIO				PIOC						// periferico que controla o LED 2 no modulo OLED
#define OLED_LED2_PIO_ID			ID_PIOC						// ID do periferico PIOC
#define OLED_LED2_PIO_IDX			30							// ID do pino conectado ao LED 2 do modulo OLED
#define OLED_LED2_PIO_IDX_MASK		(1 << OLED_LED2_PIO_IDX)	// mascara para controlarmos o LED 2 do modulo OLED

#define OLED_LED3_PIO				PIOD						// periferico que controla o LED 3 no modulo OLED
#define OLED_LED3_PIO_ID			ID_PIOD						// ID do periferico PIOD
#define OLED_LED3_PIO_IDX			11							// ID do pino conectado ao LED 3 do modulo OLED
#define OLED_LED3_PIO_IDX_MASK		(1 << OLED_LED3_PIO_IDX)	// mascara para controlarmos o LED 3 do modulo OLED

#define OLED_SW1_PIO				PIOD						// periferico que controla o SW 1 no modulo OLED
#define OLED_SW1_PIO_ID				ID_PIOD						// ID do periferico PIOD
#define OLED_SW1_PIO_IDX			28							// ID do pino conectado ao SW 1 do modulo OLED
#define OLED_SW1_PIO_IDX_MASK		(1 << OLED_SW1_PIO_IDX)		// mascara para controlarmos o SW 1 do modulo OLED

#define OLED_SW2_PIO				PIOD						// periferico que controla o SW 1 no modulo OLED
#define OLED_SW2_PIO_ID				ID_PIOD						// ID do periferico PIOD
#define OLED_SW2_PIO_IDX			30							// ID do pino conectado ao SW 1 do modulo OLED
#define OLED_SW2_PIO_IDX_MASK		(1 << OLED_SW2_PIO_IDX)		// mascara para controlarmos o SW 1 do modulo OLED

#define OLED_SW3_PIO				PIOA						// periferico que controla o SW 3 no modulo OLED
#define OLED_SW3_PIO_ID				ID_PIOA						// ID do periferico PIOA
#define OLED_SW3_PIO_IDX			19							// ID do pino conectado ao SW 3 do modulo OLED
#define OLED_SW3_PIO_IDX_MASK		(1 << OLED_SW3_PIO_IDX)		// mascara para controlarmos o SW 3 do modulo OLED

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	// inicializa o clock da placa
	sysclk_init();
	
	// desativa o WatchDog timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// habilita o PIOA, PIOC e PIOD
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	// define os pinos dos LEDs como outputs
	pio_set_output(OLED_LED1_PIO, OLED_LED1_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(OLED_LED2_PIO, OLED_LED2_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(OLED_LED3_PIO, OLED_LED3_PIO_IDX_MASK, 0, 0, 0);
	
	// define os pinos dos switches como input e os torna pull up
	pio_set_input(OLED_SW1_PIO, OLED_SW1_PIO_IDX_MASK, 0);
	pio_set_input(OLED_SW2_PIO, OLED_SW2_PIO_IDX_MASK, 0);
	pio_set_input(OLED_SW3_PIO, OLED_SW3_PIO_IDX_MASK, 0);
	pio_pull_up(OLED_SW1_PIO, OLED_SW1_PIO_IDX_MASK, 1);
	pio_pull_up(OLED_SW2_PIO, OLED_SW2_PIO_IDX_MASK, 1);
	pio_pull_up(OLED_SW3_PIO, OLED_SW3_PIO_IDX_MASK, 1);
	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {	
	
	// leitura dos botoes
	int SW1 = pio_get(OLED_SW1_PIO, PIO_INPUT, OLED_SW1_PIO_IDX_MASK);
	int SW2 = pio_get(OLED_SW2_PIO, PIO_INPUT, OLED_SW2_PIO_IDX_MASK);
	int SW3 = pio_get(OLED_SW3_PIO, PIO_INPUT, OLED_SW3_PIO_IDX_MASK);
	
	// SW e LED 1
	if (SW1) { pio_set(OLED_LED1_PIO, OLED_LED1_PIO_IDX_MASK); }
	if (!SW1) { pio_clear(OLED_LED1_PIO, OLED_LED1_PIO_IDX_MASK); }
		
	// SW e LED 2
	if (SW2) { pio_set(OLED_LED2_PIO, OLED_LED2_PIO_IDX_MASK); }
	if (!SW2) { pio_clear(OLED_LED2_PIO, OLED_LED2_PIO_IDX_MASK); }
			
	// SW e LED 3
	if (SW3) { pio_set(OLED_LED3_PIO, OLED_LED3_PIO_IDX_MASK); }
	if (!SW3) { pio_clear(OLED_LED3_PIO, OLED_LED3_PIO_IDX_MASK); }
	
	delay_ms(1);
  }
  return 0;
}
