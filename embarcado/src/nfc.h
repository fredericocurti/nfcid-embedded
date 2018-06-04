/*
 * nfc.h
 *
 * Created: 5/14/2018 4:28:49 PM
 *  Author: Frederico
 */ 


#ifndef NFC_H_
#define NFC_H_

#define USART_NFC USART2

#define YEAR        2018
#define MOUNTH      3
#define DAY         19
#define WEEK        12
#define HOUR        15
#define MINUTE      1
#define SECOND      0


void flash_led(void);
void decodeMessage(uint8_t * data, int numBytes);
void USART0_Handler(void);
int taskNfc(void);
extern xQueueHandle xQueueNfc;
extern xQueueHandle xQueueUsartBuffer;
extern xTimerHandle xTimerNfc;
extern alarm;

#endif /* NFC_H_ */