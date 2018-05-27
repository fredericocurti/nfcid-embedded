

#ifndef __MAC_LINK_H__
#define __MAC_LINK_H__

#include "pn532.h"

/**
* @brief    write a PDU packet, the packet should be less than (255 - 2) bytes
* @param    header  packet header
* @param    hlen    length of header
* @param 	body	packet body
* @param 	blen	length of body
* @return   true    success
*           false   failed
*/

//bool write(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
bool link_write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);

/**
* @brief    read a PDU packet, the packet will be less than (255 - 2) bytes
* @param    buf     the buffer to contain the PDU packet
* @param    len     length of the buffer
* @return   >=0     length of the PDU packet 
*           <0      failed
*/
int16_t link_read(uint8_t *buf, uint8_t len);
//int16_t read(uint8_t *buf, uint8_t len);

int8_t link_activateAsTarget(uint16_t timeout);

static uint8_t *link_getHeaderBuffer(uint8_t *len) {
	printf("Maclink getHeaderBuffer\n");
    return pn532_getBuffer(len);
};



#endif // __MAC_LINK_H__
