

#ifndef __MAC_LINK_H__
#define __MAC_LINK_H__

#include "pn532.h"

PN532 pn532;

/**
* @brief    write a PDU packet, the packet should be less than (255 - 2) bytes
* @param    header  packet header
* @param    hlen    length of header
* @param 	body	packet body
* @param 	blen	length of body
* @return   true    success
*           false   failed
*/
bool write(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

/**
* @brief    read a PDU packet, the packet will be less than (255 - 2) bytes
* @param    buf     the buffer to contain the PDU packet
* @param    len     lenght of the buffer
* @return   >=0     length of the PDU packet 
*           <0      failed
*/
int16_t read(uint8_t *buf, uint8_t len);

uint8_t *getHeaderBuffer(uint8_t *len) {
    return pn532.getBuffer(len);
};



#endif // __MAC_LINK_H__
