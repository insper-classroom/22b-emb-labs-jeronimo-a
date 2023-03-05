#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

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
volatile char SW1 = 0;
volatile char SW2 = 0;
volatile char SW3 = 0;


void callback_sw1() {
	if (pio_get(OLED_SW1_PIO, PIO_INPUT, OLED_SW1_IDX_MASK)) {
		SW1 = 0;
	} else {
		SW1 = 1;
	}
}

void callback_sw2() {
	if (pio_get(OLED_SW2_PIO, PIO_INPUT, OLED_SW2_IDX_MASK)) {
		SW2 = 0;
		} else {
		SW2 = 1;
	}
}

void callback_sw3() {
	if (pio_get(OLED_SW3_PIO, PIO_INPUT, OLED_SW3_IDX_MASK)) {
		SW3 = 0;
		} else {
		SW3 = 1;
	}
}


void init() {
	
	board_init();
	sysclk_init();
	delay_init();
	
	// habilita os PIOs que usaremos
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	// define os pinos dos switches como input com pullup e debounce
	pio_configure(PIOA, PIO_INPUT, OLED_SW3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(PIOC, OLED_SW2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(PIOD, OLED_SW1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// define o LED como output
	pio_configure(PIOC, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
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

	// Init OLED
	gfx_mono_ssd1306_init();
	
	// desenhos estaticos no OLED
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_draw_string("teste", 50,16, &sysfont);
}



int main (void) {
	init();
  
	while(1) {
		
		
		if (SW1 || SW2 || SW3) {
			pio_clear(PIOC, LED_IDX_MASK);
			gfx_mono_draw_string("%c%c%c", 50,16, &sysfont);
		} else {
			pio_set(PIOC, LED_IDX_MASK);
		}
	}
}
