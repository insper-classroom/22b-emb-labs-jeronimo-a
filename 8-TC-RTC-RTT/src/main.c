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

#define TASK_PRINTCONSOLE_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_PRINTCONSOLE_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/** prototypes */
void io_init(void);

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

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_oled(void *pvParameters) {
  gfx_mono_ssd1306_init();
  gfx_mono_draw_string("Exemplo RTOS", 0, 0, &sysfont);
  gfx_mono_draw_string("oii", 0, 20, &sysfont);

	uint32_t cont=0;
	for (;;)
	{
		char buf[3];
		
		cont++;
		
		sprintf(buf,"%03d",cont);
		gfx_mono_draw_string(buf, 0, 20, &sysfont);
				
		vTaskDelay(1000);
	}
}


static void task_printConsole(void *pvParameters) {

	char state = 0;
	
	uint32_t cont=0;
	for (;;)
	{		
		cont++;
		
		printf("%03d\n",cont);

		if (state) {
			pio_set(LED1_PIO, LED1_IDX_MASK);
			pio_set(LED2_PIO, LED2_IDX_MASK);
			pio_set(LED3_PIO, LED3_IDX_MASK);
			state = 0;
		} else {
			pio_clear(LED1_PIO, LED1_IDX_MASK);
			pio_clear(LED2_PIO, LED2_IDX_MASK);
			pio_clear(LED3_PIO, LED3_IDX_MASK);
			state = 1;
		}

		vTaskDelay(1000);
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

	/* Create task to control oled */
	if (xTaskCreate(task_oled, "oled", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create oled task\r\n");
	}
	
	if (xTaskCreate(task_printConsole, "task_printConsole", TASK_PRINTCONSOLE_STACK_SIZE, NULL, TASK_PRINTCONSOLE_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create printConsole task\r\n");
	}

	printf("inicio\n");
	/* Start the scheduler. */
	vTaskStartScheduler();

  /* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
