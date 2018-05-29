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

volatile xQueueHandle xQueueNfc;
volatile xQueueHandle xQueueUsartBuffer;
volatile xTimerHandle xTimerNfc;
volatile alarm = 0;

void USART0_Handler(void){
	uint32_t ret = usart_get_status(USART0);
	char c;

	// Verifica por qual motivo entrou na interrup?cao
	//  - Dado dispon?vel para leitura
	if(ret & US_IER_RXRDY){
		usart_serial_getchar(USART0, &c);
		xQueueSendFromISR(xQueueUsartBuffer, &c, NULL);
		// -  Transmissoa finalizada
		} else if(ret & US_IER_TXRDY){

	}
}

void decodeMessage(uint8_t * data, int numBytes) {
	uint32_t szPos;
	uint8_t id[32];
	int c = 0;
	
	memset(id, 0, 32);
	for ( szPos = 0; szPos < numBytes; szPos++) {
		if (data[szPos] <= 0x1F || data[szPos] < 48 || data[szPos] > 57) {
		} else {
			id[c] = (char)data[szPos];
			c++;
		}
	}

	id[c] = '\0';
	
	xQueueSend(xQueueNfc, id, portMAX_DELAY);
	return;
}

void flash_led(){
	for (int i = 0; i < 6; i++) {
		LED_Toggle(LED0);
		delay_ms(10);
	}
	LED_Off(LED0);
}

void onTimeout( xTimerHandle xTimer ) {
	printf("Timer callback!!\n");
	alarm = 1;
}



int taskNfc(void) {
	//PN532_HSU pn532hsu(Serial1);
	uint8_t ndefBuf[128];
	uint8_t decodedMessage[32];
	uint8_t c;
	uint32_t h, m, s;
	int msgSize;
	uint8_t b;
	
	xQueueUsartBuffer = xQueueCreate(1024, sizeof(uint8_t));
	if (xQueueUsartBuffer == NULL) {
		printf("Falha em criar a fila\n");
	}
	
	xQueueNfc = xQueueCreate(4, sizeof(uint8_t[32]));
	if (xQueueNfc == NULL) {
		printf("Falha em criar a fila\n");
	}
	
	xTimerNfc = xTimerCreate("receiveTimeout", 5000, pdTRUE, NULL, onTimeout);
	
	vTaskDelay(1000);
	pn532_config(1);
	snep_init();
	printf("[NFC] Starting...\n");
	xTimerStart(xTimerNfc, 0);
	
	for(;;) {
		xTimerReset(xTimerNfc, 0);
		alarm = 0;
		msgSize = snep_read(ndefBuf, sizeof(ndefBuf), 0);
		// Chegou algo!
		if (msgSize > 0) {
			xTimerReset(xTimerNfc, 0);
			// decodeMessage é responsável por colocar a string na fila!
			decodeMessage(ndefBuf, msgSize);
		}
		
		//decodeMessage(&ndefBuf, msgSize, &decodedMessage);
		
		//if (xQueueReceive(xQueueNFCReceive, &c, portMAX_DELAY)) {
			////printf("%c (%d)\n", c, c);
			//rvb[counter] = c;
			//counter++;
			//
			//if ((char) c == '\0' || counter == 9) {
				//printf("[NFC] EOS reached\n");
				//printf("[NFC] ID is %s\n", rvb);
				//counter = 0;
				//memset(rvb, 0, 32);
				//// ignores backspace entry;
			//}
		//}
	
		//printf("[Nfc] running...\n");
		//if(usart_is_rx_ready(USART0)) {
			//LED_Toggle(LED0);
			//usart_serial_getchar(USART0, &b);
			//printf("%x\n", b);
		//}
		
		// it seems there are some issues to use NdefMessage to decode the received data from Android
		//printf("Get a message from Android\n");
		//int msgSize = snep_read(ndefBuf, sizeof(ndefBuf), 20000);
		//if (msgSize > 0) {
			////NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
			////msg.print();
			//printf("\nSuccess");
			//} else {
			//printf("failed");
		//}
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
