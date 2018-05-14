#include "pn532.h"
#include "systick.h"
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
	char buffer[16];
	snprintf(buffer, 16, "%x", val);
    DMSG(buffer);
}

/**
    @brief receive data .
    @param buf --> return value buffer.
           len --> length expect to receive.
           timeout --> time of reveiving
    @retval number of received bytes, 0 means no data received.
*/


int8_t receive(uint8_t *buf, int len, uint16_t timeout)
{
  int read_bytes = 0;
  int ret;
  unsigned long start_millis;
  
  while (read_bytes < len) {
    start_millis = millis();
    do {
	  ret = 0;
	  while(!usart_is_rx_ready(USART0));
	  usart_read(USART0, &ret);
	  
      if (ret >= 0) {
        break;
     }
    } while((timeout == 0) || ((millis()- start_millis ) < timeout));
    
    if (ret < 0) {
        if(read_bytes){
            return read_bytes;
        }else{
            return PN532_TIMEOUT;
        }
    }
    buf[read_bytes] = (uint8_t)ret;
    DMSG_HEX(ret);
    read_bytes++;
  }
  return read_bytes;
}

int8_t pn532_read_ack_frame()
{
	const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
	uint8_t ackBuf[sizeof(PN532_ACK)];
	
	DMSG("\nAck: ");
	
	if( receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 ){
		DMSG("Timeout\n");
		return PN532_TIMEOUT;
	}
	
	if( memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)) ){
		DMSG("Invalid\n");
		return PN532_INVALID_ACK;
	}
	return 0;
}

void pn532_config(void) {
	usart_serial_options_t config;
	config.baudrate = 115200;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);

	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

void pn532_begin()
{
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);
}

void pn532_wakeup()
{
	usart_write(USART0, 0x55);
	usart_write(USART0, 0x55);
	usart_write(USART0, 0x0);
	usart_write(USART0, 0x0);
	usart_write(USART0, 0x0);

    /** dump serial buffer */
    if(usart_is_rx_ready(USART0)){
        DMSG("Dump serial buffer: ");
    }
    while(usart_is_rx_ready(USART0)){
        uint32_t ret;
		usart_read(USART0, &ret);
        DMSG_HEX(ret);
    }

}

int8_t pn532_write_command(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    /** dump serial buffer */
    if(usart_is_rx_ready(USART0)){
        DMSG("Dump serial buffer: ");
    }
    while(usart_is_rx_ready(USART0)){
        uint32_t ret;
        usart_read(USART0, &ret);
        DMSG_HEX(ret);
    }

    command = header[0];
    
    usart_write(USART0, PN532_PREAMBLE);
    usart_write(USART0, PN532_STARTCODE1);
    usart_write(USART0, PN532_STARTCODE2);
    
    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    usart_write(USART0, length);
    usart_write(USART0, ~length + 1);         // checksum of length
    
    usart_write(USART0, PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    DMSG("\nWrite: ");
    
	usart_serial_write_packet(USART0, header, hlen);
    for (uint8_t i = 0; i < hlen; i++) {
        sum += header[i];

        DMSG_HEX(header[i]);
    }

    usart_serial_write_packet(USART0, body, blen);
    for (uint8_t i = 0; i < blen; i++) {
        sum += body[i];

        DMSG_HEX(body[i]);
    }
    
    uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
    usart_write(USART0, checksum);
    usart_write(USART0, PN532_POSTAMBLE);

    return pn532_read_ack_frame();
}

int16_t pn532_read_response(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];
    
    DMSG("\nRead:  ");
    
    /** Frame Preamble and Start Code */
    if(receive(tmp, 3, timeout)<=0){
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


uint32_t pn532_get_firmware_version(void)
{
	uint32_t response;

	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

	if (pn532_write_command(pn532_packetbuffer, 1, 0, 0)) {
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
