#include "asf.h"

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      19
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  19
#define BUT_IDX_MASK (1 << BUT_IDX)

// Flags
volatile char but;

// Globais
int freq = 1;
int period_ms;

void io_init(void);

void callback_but_down(void) {
	if (pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)) {
		but = 0;
	} else {
		but = 1;
		freq++;
	}
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void) {

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);

	// DEfine interrupcao e associa a uma funcao de callback
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_IDX_MASK, PIO_IT_EDGE, callback_but_down);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();
	
	pio_clear(LED_PIO, LED_IDX_MASK);

	while(1) {
		// super loop
		// aplicacoes embarcadas no devem sair do while(1).
		if (but) { freq++; }
		period_ms = 1000 / freq;
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(period_ms / 2);
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(period_ms / 2);
	}
}
