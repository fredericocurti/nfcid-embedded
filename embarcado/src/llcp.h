
#ifndef __LLCP_H__
#define __LLCP_H__

#include "mac_link.h"

#define LLCP_DEFAULT_TIMEOUT  20000 // IMPORTANT !!!!!!!!!!!!!!!!!!! @TODO
#define LLCP_DEFAULT_DSAP     0x04
#define LLCP_DEFAULT_SSAP     0x20

uint8_t mode;
uint8_t ssap;
uint8_t dsap;
uint8_t *headerBuf;
uint8_t headerBufLen;
uint8_t ns;         // Number of I PDU Sent
uint8_t nr;         // Number of I PDU Received

static uint8_t SYMM_PDU[2];

//int8_t llcp_activate(uint16_t timeout = 0);
int8_t llcp_activate(uint16_t timeout);

//int8_t llcp_waitForConnection(uint16_t timeout = LLCP_DEFAULT_TIMEOUT);
int8_t llcp_waitForConnection(uint16_t timeout);

//bool llcp_write(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
bool llcp_write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);

void llcp_init();

int16_t llcp_read(uint8_t *buf, uint8_t len);

static uint8_t *llcp_getHeaderBuffer(uint8_t *len) {
    uint8_t *buf = link_getHeaderBuffer(len);
    len -= 3;       // I PDU header has 3 bytes
    return buf;
};

#endif