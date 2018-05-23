
#ifndef __LLCP_H__
#define __LLCP_H__

#include "mac_link.h"

#define LLCP_DEFAULT_TIMEOUT  20000
#define LLCP_DEFAULT_DSAP     0x04
#define LLCP_DEFAULT_SSAP     0x20

int8_t llcp_activate(uint16_t timeout = 0);

int8_t llcp_waitForConnection(uint16_t timeout = LLCP_DEFAULT_TIMEOUT);

bool llcp_write(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

int16_t llcp_read(uint8_t *buf, uint8_t len);

uint8_t *llcp_getHeaderBuffer(uint8_t *len) {
    uint8_t *buf = link_getHeaderBuffer(len);
    len -= 3;       // I PDU header has 3 bytes
    return buf;
};

uint8_t ns;         // Number of I PDU Sent
uint8_t nr;         // Number of I PDU Received