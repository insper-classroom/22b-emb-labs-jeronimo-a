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

#define LED_PIO				PIOC				// periferico que controla o LED
#define LED_PIO_ID			ID_PIOC				// ID do periferico PIOC
#define LED_PIO_IDX			8					// ID do LED no PIO
#define LED_PIO_IDX_MASK	(1 << LED_PIO_IDX)	// Mascara para controlarmos o LED

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
	
	// habilita o PIOC
	pmc_enable_periph_clk(LED_PIO_ID);
	
	// define o pino do LED como output
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
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
	
	// faz o LED piscar
	pio_set(LED_PIO, LED_PIO_IDX_MASK);		// coloca 1 no pino do LED (apaga o LED)
	delay_ms(1000);							// delay por software em ms
	pio_clear(LED_PIO, LED_PIO_IDX_MASK);	// coloca 0 no pino do LED (acende o LED)
	delay_ms(3000);							// delat por software em ms
	
  }
  return 0;
}
