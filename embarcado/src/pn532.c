#include "pn532.h"
#include "systick.h"
#include "nfc.h"
#include <string.h>

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

void DMSG(char* log) {
	usart_put_string(USART1, "[pn532] ");
	usart_put_string(USART1, log);
	usart_put_string(USART1, "\r\n");
}

void DMSG_HEX(uint32_t val) {
	printf("%x ",  (uint8_t) val);
}

void DMSG_INT(uint32_t val) {
	printf("%d\n", val);
}


/**
@brief receive data .
@param buf --> return value buffer.
len --> length expect to receive.
timeout --> time of reveiving
@retval number of received bytes, 0 means no data received.
*/


int8_t receive(uint8_t *buf, int len, uint16_t timeout) {
	int read_bytes = 0;
	uint8_t ret;
	uint8_t b;
	unsigned long start_millis;
	alarm = 0;

	xTimerReset(xTimerNfc, 0);
	//start_millis = millis();
	while (read_bytes < len && !alarm) {
		if (xQueueReceive(xQueueUsartBuffer, &b, 0) == pdTRUE) {
			//xTimerReset(xTimerNfc, 0);
			//start_millis = millis();
			buf[read_bytes] = b;
			read_bytes++;
			printf("%x ", b);
			//start_millis = millis();
		}
		//} else {
		//}
		//if (timeout != 0 && time <= 0){

		//}
		//if (timeout != 0 && (millis() - start_millis) >= timeout) {

		//}
	}
	
	if (alarm) {
		alarm = 0;
		return PN532_TIMEOUT;
	}
	
	
		
	//while (read_bytes < len) {
		//start_millis = millis();
		//do {
			//ret = 0;
			//usart_getchar(USART0, &ret);
			//
			//if (ret >= 0) {
				//break;
			//}
		//} while((timeout == 0) || ((millis()- start_millis ) < timeout));
		//
		//if (ret < 0) {
			//if (read_bytes) {
				//return read_bytes;
			//} else {
				//return PN532_TIMEOUT;
			//}
		//}
		//
		//buf[read_bytes] = (uint8_t)ret;
		//DMSG_HEX(ret);
		//read_bytes++;
	//}

	return read_bytes;
}

int8_t pn532_read_ack_frame() {
	const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
	uint8_t ackBuf[sizeof(PN532_ACK)];
	
	printf("\nAck: ");
	if( receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 ){
		printf("[readAckFrame] Timeout\n");
		return PN532_TIMEOUT;
	}
	
	if( memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)) ){
		printf("[readAckFrame] Invalid\n");
		return PN532_INVALID_ACK;
	}
	
	printf("[readAckFrame] Valid ack!\n");
	
	return 0;
}

void pn532_config() {
	usart_serial_options_t config;
	config.baudrate = 115200;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = US_MR_NBSTOP_1_BIT;
	
	sysclk_enable_peripheral_clock(ID_USART2);
	sysclk_enable_peripheral_clock(ID_PIOD);
	
	usart_serial_init(USART_NFC, &config);

	// RX - PB0  TX - PB1
	pio_configure(PIOD, PIO_PERIPH_B, (1 << 15), PIO_DEFAULT); // RX
	pio_configure(PIOD, PIO_PERIPH_B, (1 << 16), PIO_DEFAULT); // TX
	
	usart_disable_interrupt(USART_NFC, 0xffffffff);
	
	usart_enable_tx(USART_NFC);
	usart_enable_rx(USART_NFC);

	usart_enable_interrupt(USART_NFC, US_IER_RXRDY);
	NVIC_SetPriority(ID_USART2, 4);
	NVIC_EnableIRQ(ID_USART2);
}

void pn532_begin() {
	printf("[pn532] begin\n");
}


void pn532_wakeup() {
	if(usart_is_tx_ready(USART_NFC)){
		printf("[pn532] sending wakeup!\n");
	}
	
	usart_putchar(USART_NFC, 0x55);
	usart_putchar(USART_NFC, 0x55);
	usart_putchar(USART_NFC, 0x0);
	usart_putchar(USART_NFC, 0x0);
	usart_putchar(USART_NFC, 0x0);
	usart_write(USART_NFC, 0x0);
	usart_write(USART_NFC, 0x0);
	usart_write(USART_NFC, 0x0);
	usart_write(USART_NFC, 0x0);
	usart_write(USART_NFC, 0x0);

	/** dump serial buffer */
	if(usart_is_rx_ready(USART_NFC)){
		DMSG("Dump serial buffer: ");
	}
	
	while(usart_is_rx_ready(USART_NFC)){
		uint8_t ret;
		usart_getchar(USART_NFC, &ret);
		DMSG_HEX(ret);
	}
	
	printf("[pn532] Wakeup command sent!\n");
	return;
}


int8_t pn532_write_command(uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen) {
	/** dump serial buffer */
	if(usart_is_rx_ready(USART_NFC)){
		DMSG("Dump serial buffer: ");
	}
	
	while(usart_is_rx_ready(USART_NFC)){
		uint32_t ret;
		usart_serial_getchar(USART_NFC, &ret);
		DMSG_HEX(ret);
	}
	
	xQueueReset(xQueueUsartBuffer);
	command = header[0];
	
	usart_putchar(USART_NFC, (uint8_t) PN532_PREAMBLE);
	usart_putchar(USART_NFC, (uint8_t) PN532_STARTCODE1);
	usart_putchar(USART_NFC, (uint8_t) PN532_STARTCODE2);
	
	uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
	usart_putchar(USART_NFC, (uint8_t) length);
	usart_putchar(USART_NFC, (uint8_t) ~length + 1);         // checksum of length
	
	usart_putchar(USART_NFC, (uint8_t) PN532_HOSTTOPN532);
	uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

	usart_serial_write_packet(USART_NFC, header, hlen);
	
	for (uint8_t i = 0; i < hlen; i++) {
		sum += header[i];
		//DMSG_HEX(header[i]);
	}

	usart_serial_write_packet(USART_NFC, body, blen);
	for (uint8_t i = 0; i < blen; i++) {
		sum += body[i];
		//DMSG_HEX(body[i]);
	}

	uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
	usart_putchar(USART_NFC, checksum);
	usart_putchar(USART_NFC, (uint8_t) PN532_POSTAMBLE);

	printf("\nWrite: ");
	for (uint8_t i = 0; i < hlen; i++) {
		DMSG_HEX(header[i]);
	}
	
	return pn532_read_ack_frame();
}

int16_t pn532_read_response(uint8_t buf[], uint8_t len, uint16_t timeout)
{
	uint8_t tmp[3];
	
	//delay_ms(100);
	printf("\nRead: ");
	
	/** Frame Preamble and Start Code */
	if(receive(tmp, 3, timeout)<=0) {
		printf("timeout!\n");
		return PN532_TIMEOUT;
	}
	
	if(0 != tmp[0] || 0!= tmp[1] || 0xFF != tmp[2]){
		DMSG("Preamble error");
		return PN532_INVALID_FRAME;
	}
	
	/** receive length and check */
	uint8_t length[2];
	if(receive(length, 2, timeout) <= 0){
		return PN532_TIMEOUT;
	}
	if( 0 != (uint8_t)(length[0] + length[1]) ){
		DMSG("Length error");
		return PN532_INVALID_FRAME;
	}
	length[0] -= 2;
	if( length[0] > len){
		return PN532_NO_SPACE;
	}
	
	/** receive command byte */
	uint8_t cmd = command + 1;               // response command
	if(receive(tmp, 2, timeout) <= 0){
		return PN532_TIMEOUT;
	}
	if( PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]){
		DMSG("Command error");
		return PN532_INVALID_FRAME;
	}
	
	if(receive(buf, length[0], timeout) != length[0]){
		return PN532_TIMEOUT;
	}
	uint8_t sum = PN532_PN532TOHOST + cmd;
	for(uint8_t i=0; i<length[0]; i++){
		sum += buf[i];
	}
	
	/** checksum and postamble */
	if(receive(tmp, 2, timeout) <= 0){
		return PN532_TIMEOUT;
	}
	if( 0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1] ){
		DMSG("Checksum error");
		return PN532_INVALID_FRAME;
	}
	return length[0];
}

int8_t pn532_tgInitAsTarget(uint16_t timeout){
	const uint8_t command[] = {
		PN532_COMMAND_TGINITASTARGET,
		0,
		0x00, 0x00,         //SENS_RES
		0x00, 0x00, 0x00,   //NFCID1
		0x40,               //SEL_RES

		0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, // POL_RES
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF,

		0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, 0x00, 0x00, //NFCID3t: Change this to desired value

		0x06, 0x46,  0x66, 0x6D, 0x01, 0x01, 0x10, 0x00// LLCP magic number and version parameter
	};
	
	uint8_t len = sizeof(command);
	int8_t status = pn532_write_command(command, len, NULL, 0);
	if (status < 0) {
		return -1;
	}

	status = pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
	if (status > 0) {
		return 1;
		} else if (PN532_TIMEOUT == status) {
		return 0;
		} else {
		return -2;
	}
}

int16_t pn532_tgGetData(uint8_t *buf, uint8_t len)
{
	buf[0] = PN532_COMMAND_TGGETDATA;

	if (pn532_write_command(buf, 1, NULL, 0)) {
		return -1;
	}

	int16_t status = pn532_read_response(buf, len, 3000);
	if (0 >= status) {
		return status;
	}

	uint16_t length = status - 1;

	if (buf[0] != 0) {
		DMSG("status is not ok\n");
		return -5;
	}

	for (uint8_t i = 0; i < length; i++) {
		buf[i] = buf[i + 1];
	}

	return length;
}

bool pn532_tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
	if (hlen > (sizeof(pn532_packetbuffer) - 1)) {
		if ((body != 0) || (header == pn532_packetbuffer)) {
			DMSG("tgSetData:buffer too small\n");
			return false;
		}

		pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;
		if (pn532_write_command(pn532_packetbuffer, 1, header, hlen)) {
			return false;
		}
	} else {
		for (int8_t i = hlen - 1; i >= 0; i--){
			pn532_packetbuffer[i + 1] = header[i];
		}
		pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;

		if (pn532_write_command(pn532_packetbuffer, hlen + 1, body, blen)) {
			return false;
		}
	}

	if (0 > pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000)) {
		return false;
	}

	if (0 != pn532_packetbuffer[0]) {
		return false;
	}

	return true;
}

uint32_t pn532_get_firmware_version(void)
{
	uint32_t response;

	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

	if (pn532_write_command(pn532_packetbuffer, 1, NULL, 0)) {
		return 0;
	}

	// read data packet
	int16_t status = pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer), 0);
	if (0 > status) {
		return 0;
	}

	response = pn532_packetbuffer[0];
	response <<= 8;
	response |= pn532_packetbuffer[1];
	response <<= 8;
	response |= pn532_packetbuffer[2];
	response <<= 8;
	response |= pn532_packetbuffer[3];

	return response;
}

uint8_t pn532_setPassiveActivationRetries(uint8_t maxRetries) {
	uint8_t pb[64];
	pb[0] = PN532_COMMAND_RFCONFIGURATION;
	pb[1] = 5;
	pb[2] = 0xFF;
	pb[3] = 0x01;
	pb[4] = maxRetries;
	
	/** dump serial buffer */
	if(pn532_write_command(pb, 5, NULL, 0)) {
		return 0x00;
	}
	
	return (0 < pn532_read_response(pb, sizeof(pb), 50));
}

uint8_t pn532_SAMConfig(void) {
	//uint8_t pb[64];
	pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
	pn532_packetbuffer[1] = 0x01; // normal mode
	pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
	pn532_packetbuffer[3] = 0x01; // use IRQ pin!
	
	printf("[pn532] Writing SAMConfig");
	
	if (pn532_write_command(pn532_packetbuffer, 4, NULL, 0)) {
		return 0;
	}
	
	return (0 < pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

uint8_t pn532_readPassiveTargetID(uint8_t cardBaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout, uint8_t inlist) {
	uint8_t pb[64];
	pb[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pb[1] = 0x01; // max 1 cards at once
	pb[2] = cardBaudrate;
	
	//printf("[pn532] SAMConfig\n");
	
	if (pn532_write_command(pb, 3, NULL, 0)) {
		return 0x0; // command failed
	}
	
	if ( pn532_read_response(pb, sizeof(pb), 1000) < 0 ) {
		return 0x0;
	}
	
	// check some basic stuff
	/* ISO14443A card response should be in the following format:

	byte            Description
	-------------   ------------------------------------------
	b0              Tags Found
	b1              Tag Number (only one used in this example)
	b2..3           SENS_RES
	b4              SEL_RES
	b5              NFCID Length
	b6..NFCIDLen    NFCID
	*/
	
	if (pb[0] != 1) {
		return 0x0;
	}
	
	uint16_t sens_res = pb[2];
	sens_res <<= 8;
	sens_res |= pb[3];
	
	printf("ATQA: 0x"); DMSG_HEX(sens_res);
	printf("SAK: 0x"); DMSG_HEX(pb[4]);
	DMSG("\n");
	
	// card appears to be mifare classic
	*uidLength = pb[5];
	
	for (uint8_t i = 0; i < pb[5]; i++) {
		uid[i] = pb[6 + i];
	}
	
	if (inlist) {
		inListedTag = pb[1];
	}
	
	return 1;

}
