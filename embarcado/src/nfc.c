/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <string.h>

#include "systick.h"
#include "snep.h"

volatile xQueueHandle xQueueNFCSend;
volatile xQueueHandle xQueueNFCReceive;


int usart_get_string(Usart *usart, char buffer[], int bufferlen, int timeout_ms) {
	long timestart = millis();
	uint32_t rx;
	uint32_t counter = 0;
	
	while(millis() - timestart < timeout_ms && counter < bufferlen - 1) {
		if(usart_read(usart, &rx) == 0) {
			//timestart = g_systimer; // reset timeout
			buffer[counter++] = rx;
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen, char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void usart_log(char* name, char* log) {
	usart_put_string(USART1, "[");
	usart_put_string(USART1, name);
	usart_put_string(USART1, "] ");
	usart_put_string(USART1, log);
	usart_put_string(USART1, "\r\n");
}

//void config_console(void) {
	//usart_serial_options_t config;
	//config.baudrate = 115200;
	//config.charlength = US_MR_CHRL_8_BIT;
	//config.paritytype = US_MR_PAR_NO;
	//config.stopbits = false;
	//usart_serial_init(USART1, &config);
	//usart_enable_tx(USART1);
	//usart_enable_rx(USART1);
//}
//


int taskNfc(void) {
	//PN532_HSU pn532hsu(Serial1);
	vTaskDelay(5000);
	pn532_config();
	pn532_begin();
	pn532_wakeup();
	
	snep_init();
	
	uint8_t ndefBuf[128];
	
	for(;;) {
		// it seems there are some issues to use NdefMessage to decode the received data from Android
		printf("Get a message from Android");
		int msgSize = snep_read(ndefBuf, sizeof(ndefBuf), 20000);
		if (msgSize > 0) {
			//NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
			//msg.print();
			printf("\nSuccess");
			} else {
			printf("failed");
		}
		vTaskDelay(1500);
	}
}
 


// Old
//int taskNfc (void) {
	///* Insert system clock initialization code here (sysclk_init()). */
	////board_init();
	////sysclk_init();
	////delay_init();
	//
	////config_console();
	//pn532_config();
	//systick_config();
	//
	//printf("[NFC] Nfc task init\n");
	//delay_s(2);
	//
	//pn532_begin();
	//pn532_wakeup();
	////delay_ms(100);
	//
	///* Initialize Queues */
	//xQueueNFCSend = xQueueCreate(1,sizeof(char[32]));
	//if (xQueueNFCSend == NULL) {
		//printf("Falha em criar a fila\n");
	//}
	//
	//xQueueNFCReceive = xQueueCreate(1,sizeof(int));
	//if (xQueueNFCSend == NULL) {
		//printf("Falha em criar a fila\n");
	//}
	//
	//uint32_t ver = pn532_get_firmware_version();
	//
	//DMSG_HEX(ver);
	//while(1) {
		//uint32_t val = 0;
		//usart_read(USART0, &val);
		//DMSG_HEX(val);
		//vTaskDelay(1000);
	//}
	//
	///* Insert application code here, after the board has been initialized. */
//}
