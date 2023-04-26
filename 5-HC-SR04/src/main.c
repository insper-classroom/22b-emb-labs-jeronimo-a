#include <asf.h>
#include "conf_board.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* Pinos ECHO e TRIGGER */
#define ECHO_PIO		PIOD
#define ECHO_PIO_ID		ID_PIOD
#define ECHO_PIN		27
#define ECHO_PIN_MASK	(1 << ECHO_PIN)
#define TRIG_PIO		PIOA
#define TRIG_PIO_ID		ID_PIOA
#define TRIG_PIN		21
#define TRIG_PIN_MASK	(1 << TRIG_PIN)
#define BUT_PIO			PIOA
#define BUT_PIO_ID		ID_PIOA
#define BUT_PIN			11
#define BUT_PIN_MASK	(1 << BUT_PIN)

/* RTOS  */
#define TASK_OLED_STACK_SIZE						(1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY					(tskIDLE_PRIORITY)
#define TASK_PRINTCONSOLE_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_PRINTCONSOLE_STACK_PRIORITY            (tskIDLE_PRIORITY)
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/* prototypes */
void but_callback(void);
void echo_callback(void);
static void io_init(void);

/************************************************************************/
/* Semaphores				                                            */
/************************************************************************/

SemaphoreHandle_t echo_semaphore;

/************************************************************************/
/* Queues																*/
/************************************************************************/

QueueHandle_t echo_queue;

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

void but_callback(void) {
}

void echo_callback(void) {
	if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIN_MASK)) {  }	// fall edge
	else {  }												// rise edge
}

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
	
	uint32_t cont=0;
	for (;;)
	{		
		cont++;
		
		printf("%03d\n",cont);
		
		vTaskDelay(1000);
	}
}

static void task_count_echo(void *pvParameters) {
	
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

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

static void io_init(void) {
	
	// Inicializa o clock dos PIOs
	pmc_enable_periph_clk(ECHO_PIO_ID);
	pmc_enable_periph_clk(TRIG_PIO_ID);
	
	// Configuracao do ECHO e do BUT como entradas
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_PIN_MASK, 60);
	
	// Configuracao do TRIGGER como saida
	pio_configure(TRIG_PIO, PIO_OUTPUT_1, TRIG_PIN_MASK, PIO_DEFAULT);
	
	// Configura a interrupcao do ECHO e do BUT
	pio_handler_set(ECHO_PIO, ECHO_PIO_ID, ECHO_PIN_MASK, PIO_IT_EDGE, echo_callback);
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_PIN_MASK, PIO_IT_FALL_EDGE, but_callback);
					
	pio_enable_interrupt(BUT_PLACA_PIO, BUT_PLACA_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT_PLACA_PIO);
	
	/* configura prioridae */
	NVIC_EnableIRQ(BUT_PLACA_PIO_ID);
	NVIC_SetPriority(BUT_PLACA_PIO_ID, 4);
	
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/


int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	
	// Cria os semaforos
	echo_semaphore = xSemaphoreCreateBinary();
	if (echo_semaphore == NULL) {
		printf("Falha em criar semaforo echo\n");
	}
	
	// Cria as queues
	echo_queue = xQueueCreate(64, sizeof(uint32_t));
	if (echo_queue == NULL) {
		printf("Falha em criar a queue echo\n");
	}

	/* Create task to control oled */
	if (xTaskCreate(task_oled, "oled", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
	  printf("Failed to create oled task\r\n");
	}
	
	if (xTaskCreate(task_printConsole, "task_printConsole", TASK_PRINTCONSOLE_STACK_SIZE, NULL, TASK_PRINTCONSOLE_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create printConsole task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

  /* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
