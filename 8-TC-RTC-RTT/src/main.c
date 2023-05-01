
#include <asf.h>
#include "conf_board.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// LED 1 do OLED xPlained
#define LED1_PIO		PIOA
#define LED1_PIO_ID		ID_PIOA
#define LED1_IDX		0
#define LED1_IDX_MASK	(1 << LED1_IDX)

// LED 2 do OLED xPlained
#define LED2_PIO		PIOC
#define LED2_PIO_ID		ID_PIOC
#define LED2_IDX		30
#define LED2_IDX_MASK	(1 << LED2_IDX)

// LED 3 do OLED xPlained
#define LED3_PIO		PIOB
#define LED3_PIO_ID		ID_PIOB
#define LED3_IDX		2
#define LED3_IDX_MASK	(1 << LED3_IDX)

/** RTOS  */
#define TASK_OLED_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define TASK_PRINTCONSOLE_STACK_SIZE      	(1024*6/sizeof(portSTACK_TYPE))
#define TASK_PRINTCONSOLE_STACK_PRIORITY	(tskIDLE_PRIORITY)

#define TASK_LED_STACK_SIZE               	(1024*6/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            	(tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/* Semaforos */
SemaphoreHandle_t semaphore_led1;
SemaphoreHandle_t semaphore_led2;

/** prototypes */
void io_init(void);
void TC_init(Tc*, int, int, int);
void pin_toggle(Pio*, uint32_t);
static void RTT_init(float, uint32_t, uint32_t);





/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}





/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void TC1_Handler(void) {

	volatile uint32_t status = tc_get_status(TC0, 1);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(semaphore_led1, &xHigherPriorityTaskWoken);
}

void RTT_Handler(void) {

	uint32_t ul_status;
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(semaphore_led2, &xHigherPriorityTaskWoken);
	}
}





/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_oled(void *pvParameters) {
	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("Exemplo RTOS", 0, 0, &sysfont);
	gfx_mono_draw_string("oii", 0, 20, &sysfont);

	uint32_t cont=0;
	while (1) {}
}

static void task_led(void *pvParameters) {

	RTT_init(4, 16, RTT_MR_ALMIEN);

	while (1) {

		if (xSemaphoreTake(semaphore_led1, 0)) {
			pin_toggle(LED1_PIO, LED1_IDX_MASK);
		}

		if (xSemaphoreTake(semaphore_led2, 0)) {
			pin_toggle(LED2_PIO, LED2_IDX_MASK);
	  		RTT_init(4, 16, RTT_MR_ALMIEN);
		}

	}

}





/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void io_init() {

	// Inicializa clock dos PIOs
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);

	// Configura os PIOs para lidar com os pinos dos LEDs do OLED como saidas
	pio_configure(LED1_PIO, PIO_OUTPUT_1, LED1_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED2_PIO, PIO_OUTPUT_1, LED2_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED3_PIO, PIO_OUTPUT_1, LED3_IDX_MASK, PIO_DEFAULT);

}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	
	/** ATIVA PMC PCK6 TIMER_CLOCK1  */
	if(ul_tcclks == 0 )
	    pmc_enable_pck(PMC_PCK_6);
	
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
  	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

  uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
  rtt_sel_source(RTT, false);
  rtt_init(RTT, pllPreScale);
  
  if (rttIRQSource & RTT_MR_ALMIEN) {
	uint32_t ul_previous_time;
  	ul_previous_time = rtt_read_timer_value(RTT);
  	while (ul_previous_time == rtt_read_timer_value(RTT));
  	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
  }

  /* config NVIC */
  NVIC_DisableIRQ(RTT_IRQn);
  NVIC_ClearPendingIRQ(RTT_IRQn);
  NVIC_SetPriority(RTT_IRQn, 4);
  NVIC_EnableIRQ(RTT_IRQn);

  /* Enable RTT interrupt */
  if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
  else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
		  
}

void pin_toggle(Pio *pio, uint32_t mask) {
  if(pio_get_output_data_status(pio, mask))
    pio_clear(pio, mask);
  else
    pio_set(pio,mask);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}





/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {

	// Initialize the SAM system
	sysclk_init();
	board_init();

	// Initialize the console uart
	configure_console();

	// Inicializa o IO
	io_init();

	// Inicializa o TC
	TC_init(TC0, ID_TC1, 1, 2);
	tc_start(TC0, 1);

	/* Create task to control oled */
	if (xTaskCreate(task_oled, "oled", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create oled task\r\n");
	}

	if (xTaskCreate(task_led, "task_led", TASK_LED_STACK_SIZE, NULL, TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create led task\r\n");
	}

	/* Cria os semaforos e queues */
	semaphore_led1 = xSemaphoreCreateBinary();
	if (semaphore_led1 == NULL) {
		printf("Failed to create led1 semaphore");
	}

	semaphore_led2 = xSemaphoreCreateBinary();
	if (semaphore_led2 == NULL) {
		printf("Failed to create led2 semaphore");
	}

	printf("inicio\n");
	/* Start the scheduler. */
	vTaskStartScheduler();

  /* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
