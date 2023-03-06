#include <stdio.h>
#include <asf.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

#define BUZZ_PIO      PIOC
#define BUZZ_PIO_ID   ID_PIOC
#define BUZZ_IDX      19
#define BUZZ_IDX_MASK (1 << BUZZ_IDX)

#define OLED_SW1_PIO				PIOD						// periferico que controla o SW 1 no modulo OLED
#define OLED_SW1_PIO_ID				ID_PIOD						// ID do periferico PIOD
#define OLED_SW1_IDX				28							// ID do pino conectado ao SW 1 do modulo OLED
#define OLED_SW1_IDX_MASK			(1 << OLED_SW1_IDX)			// mascara para controlarmos o SW 1 do modulo OLED

#define OLED_SW2_PIO				PIOC						// periferico que controla o SW 2 no modulo OLED
#define OLED_SW2_PIO_ID				ID_PIOC						// ID do periferico PIOC
#define OLED_SW2_IDX				31							// ID do pino conectado ao SW 2 do modulo OLED
#define OLED_SW2_IDX_MASK			(1 << OLED_SW2_IDX)			// mascara para controlarmos o SW 2 do modulo OLED

#define OLED_SW3_PIO				PIOA						// periferico que controla o SW 3 no modulo OLED
#define OLED_SW3_PIO_ID				ID_PIOA						// ID do periferico PIOA
#define OLED_SW3_IDX				19							// ID do pino conectado ao SW 3 do modulo OLED
#define OLED_SW3_IDX_MASK			(1 << OLED_SW3_IDX)			// mascara para controlarmos o SW 3 do modulo OLED

// flags dos botoes
volatile char DIMINUIR = 0;
volatile char SW1 = 0;
volatile char SW2 = 0;
volatile char SW3 = 0;

// globais
char reduziu = 0;
int contagem_SW1 = 0;
int contagem_reducao_SW1;
const char end_str[] = " Hz  ";
const int variacao_por_clique = 1;
const int frequencia_maxima = 1000;
const int frequencia_minima = 1;
int meio_periodo_ms;
int frequencia = 5;
char piscando = 1;

void update_oled() {
	char frequencia_str[5];
	sprintf(frequencia_str, "%d", frequencia);
	strcat(frequencia_str, end_str);
	gfx_mono_draw_string(frequencia_str, 50,16, &sysfont);
}

void callback_sw1() {
	if (pio_get(OLED_SW1_PIO, PIO_INPUT, OLED_SW1_IDX_MASK)) {
		SW1 = 0;
		if (contagem_SW1 < contagem_reducao_SW1) {
			if (frequencia <= frequencia_maxima - variacao_por_clique) {
				frequencia += variacao_por_clique;
			}
		}
	} else {
		SW1 = 1;
	}
}

void callback_sw2() {
	if (pio_get(OLED_SW2_PIO, PIO_INPUT, OLED_SW2_IDX_MASK)) {
		SW2 = 0;
		piscando = !piscando;
	} else {
		SW2 = 1;
	}
}

void callback_sw3() {
	if (pio_get(OLED_SW3_PIO, PIO_INPUT, OLED_SW3_IDX_MASK)) {
		SW3 = 0;
	} else {
		SW3 = 1;
		if (frequencia >= frequencia_minima + variacao_por_clique) {
			frequencia -= variacao_por_clique;
		}
	}
}


void pio_init() {
	
	// habilita os PIOs que usaremos
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	// define os pinos dos switches como input com pullup e debounce
	pio_configure(PIOA, PIO_INPUT, OLED_SW3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(PIOC, OLED_SW2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(PIOD, OLED_SW1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// define o LED como output
	pio_configure(PIOC, PIO_OUTPUT_0, LED_IDX_MASK | BUZZ_IDX_MASK, PIO_DEFAULT);
	
	// define interrupcoes e associa a uma funcao de callback
	pio_handler_set(PIOA, ID_PIOA, OLED_SW3_IDX_MASK, PIO_IT_EDGE, callback_sw3);
	pio_handler_set(PIOC, ID_PIOC, OLED_SW2_IDX_MASK, PIO_IT_EDGE, callback_sw2);
	pio_handler_set(PIOD, ID_PIOD, OLED_SW1_IDX_MASK, PIO_IT_EDGE, callback_sw1);
	
	// ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(PIOA, OLED_SW3_IDX_MASK);
	pio_enable_interrupt(PIOC, OLED_SW2_IDX_MASK);
	pio_enable_interrupt(PIOD, OLED_SW1_IDX_MASK);
	pio_get_interrupt_status(PIOA);
	pio_get_interrupt_status(PIOC);
	pio_get_interrupt_status(PIOD);
	
	// habilita interrupcoes nos PIOS dos botoes com prioridade 4
	NVIC_EnableIRQ(ID_PIOA);
	NVIC_EnableIRQ(ID_PIOC);
	NVIC_EnableIRQ(ID_PIOD);
	NVIC_SetPriority(ID_PIOA, 4);
	NVIC_SetPriority(ID_PIOC, 4);
	NVIC_SetPriority(ID_PIOD, 4);
	
}


void init() {
	
	board_init();
	sysclk_init();
	delay_init();
	
	pio_init();

	// Init OLED
	gfx_mono_ssd1306_init();
	
	// desenhos estaticos no OLED
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	update_oled();
}



int main (void) {
	
	init();
  
	while(1) {
		
		contagem_reducao_SW1 = 2 * frequencia / 3;
		if (contagem_reducao_SW1 == 0) {
			contagem_reducao_SW1 = 1;
		}
		
		if (SW1) {
			contagem_SW1++;
			if (contagem_SW1 >= contagem_reducao_SW1 && frequencia >= frequencia_minima + variacao_por_clique && !reduziu) {
				frequencia -= variacao_por_clique;
				reduziu = 1;
			}
		} else {
			reduziu = 0;
			contagem_SW1 = 0;
		}
		
		if (piscando) {
			update_oled();
			meio_periodo_ms = 1000 / (2 * frequencia);
			pio_set(PIOC, LED_IDX_MASK | BUZZ_IDX_MASK);
			delay_ms(meio_periodo_ms / 2);
			update_oled();
			delay_ms(meio_periodo_ms / 2);
			update_oled();
			pio_clear(PIOC, LED_IDX_MASK | BUZZ_IDX_MASK);
			delay_ms(meio_periodo_ms / 2);
			update_oled();
			delay_ms(meio_periodo_ms / 2);
		} else {
			pio_set(PIOC, LED_IDX_MASK | BUZZ_IDX_MASK);
		}
	}
}
