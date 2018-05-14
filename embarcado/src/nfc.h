/*
 * nfc.h
 *
 * Created: 5/14/2018 4:28:49 PM
 *  Author: Frederico
 */ 


#ifndef NFC_H_
#define NFC_H_


int taskNfc(void);
extern xQueueHandle xQueueNFCSend;
extern xQueueHandle xQueueNFCReceive;

#endif /* NFC_H_ */