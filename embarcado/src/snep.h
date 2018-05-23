#ifndef __SNEP_H__
#define __SNEP_H__

#include "llcp.h"

#define SNEP_DEFAULT_VERSION	0x10	// Major: 1, Minor: 0

#define SNEP_REQUEST_PUT		0x02
#define SNEP_REQUEST_GET		0x01

#define SNEP_RESPONSE_SUCCESS	0x81
#define SNEP_RESPONSE_REJECT	0xFF


int16_t snep_read(uint8_t *buf, uint8_t len, uint16_t timeout = 0);


LLCP llcp;
uint8_t *headerBuf;
uint8_t headerBufLen;



 #endif // __SNEP_H__