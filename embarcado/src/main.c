#include "asf.h"
#include "main.h"
#include "wifi.h"
#include "nfc.h"
#include "systick.h"
#include "conf_board.h"
#include <string.h>
//#include "bsp/include/nm_bsp.h"
//#include "driver/include/m2m_wifi.h"
//#include "socket/include/socket.h"

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- WINC1500 weather client example --"STRING_EOL	\
	"-- "BOARD_NAME " --"STRING_EOL	\
	"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL
/**
 * \brief Configure UART console.
 */

// PROTOTYPES
void configure_console(void);
int validateId(char* id);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);



// FUNCTIONS

void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

int validateId(char* id) {
	int isValid = 0;
	xQueueSend(xQueueWifiSend, id, 50);
	xQueueReceive(xQueueWifiReceive, &isValid, portMAX_DELAY);
	return isValid;
}

#if !(SAMV71 || SAME70)
/**
 * \brief Handler for System Tick interrupt.
 */
void SysTick_Handler(void) {
	g_systimer++;
	xPortSysTickHandler();
}
#endif

/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void)
{
}

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/**
 * \brief This task, when activated, send every ten seconds on debug UART
 * the whole report of free heap and total tasks status
 */
static void taskMonitor(void *pvParameters) {
	static portCHAR szList[256];
	UNUSED(pvParameters);
	
	for (;;) {
		vTaskDelay(5000);
		//printf("\n--- task ## %u", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		//printf(szList);
	}
}

static void taskMain(void *pvParameters) {
	static portCHAR szList[256];
	UNUSED(pvParameters);
	int counter = 0;
	
	char id[] = "testeee";
	int isValid = 0;		
	
	for (;;) {
		printf("[Main] Main running...\n");
		//isValid = validateId(id); // FAZ O REQUEST E RETORNA SE O ID E VALIDO OU NAO

		//isValid == 1
			//? printf("[Main] ID: %s is VALID\n", id)
			//: printf("[Main] ID: %s is INVALID\n", id);
			
		vTaskDelay(10000);	
	}
}

static void taskLed(void *pvParameters) {
	UNUSED(pvParameters);
		
	for (;;) {
		LED_Toggle(LED0);
		printf("[LED] Rodando...\n");
		vTaskDelay(5000);
	}
}


/**
 * \brief Main application function.
 *
 * Initialize system, UART console, network then start weather client.
 *
 * \return Program return value.
 */
int main(void) {
	uint8_t wifiIsConnected = 0;
	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	configure_console();
	printf(STRING_HEADER);
	
	delay_s(5);
	
	//wifiInit();
	//wifiIsConnected = wifiInit();
	//if (wifiIsConnected) { 
		//printf("[WIFI] Connected com sucesso!\n");	
	//}
		/* Create task to monitor processor activity */
		
	if (xTaskCreate(taskMonitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
	TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}
	
	//if (xTaskCreate(taskWifi, "Wifi", TASK_LED_STACK_SIZE, NULL,
	//TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create Wifi task\r\n");
	//}

	if (xTaskCreate(taskMain, "MAIN", TASK_MONITOR_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Main task\r\n");
	}
	
	if (xTaskCreate(taskNfc, "NFC", TASK_MONITOR_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create NFC task\r\n");
	}
	
	if (xTaskCreate(taskLed, "LED", TASK_MONITOR_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create LED task\r\n");
	}
	
	vTaskStartScheduler();

	return 0;
}

