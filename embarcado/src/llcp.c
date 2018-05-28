#include "llcp.h"
#include "pn532.h"

// LLCP PDU Type Values
#define PDU_SYMM    0x00
#define PDU_PAX     0x01
#define PDU_CONNECT 0x04
#define PDU_DISC    0x05
#define PDU_CC      0x06
#define PDU_DM      0x07
#define PDU_I       0x0c
#define PDU_RR      0x0d

inline uint8_t getPType(const uint8_t *buf)
{
    return ((buf[0] & 0x3) << 2) + (buf[1] >> 6);
}

inline uint8_t getSSAP(const uint8_t *buf)
{
    return  buf[1] & 0x3f;
}

inline uint8_t getDSAP(const uint8_t *buf)
{
    return buf[0] >> 2;
}

void llcp_init() {
	headerBuf = link_getHeaderBuffer(&headerBufLen);
	ns = 0;
	nr = 0;
};

int8_t llcp_activate(uint16_t timeout) {
    return link_activateAsTarget(timeout);
}

int8_t llcp_waitForConnection(uint16_t timeout) {
    uint8_t type;
    mode = 1;
    ns = 0;
    nr = 0;

    // Get CONNECT PDU
    DMSG("wait for a CONNECT PDU\n");
    do {
        if (2 > link_read(headerBuf, headerBufLen)) {
            return -1;
        }

        type = getPType(headerBuf);
        if (PDU_CONNECT == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link_write(SYMM_PDU, sizeof(SYMM_PDU), 0, 0)) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    // Put CC PDU
    DMSG("put a CC(Connection Complete) PDU to response the CONNECT PDU\n");
    ssap = getDSAP(headerBuf);
    dsap = getSSAP(headerBuf);
    headerBuf[0] = (dsap << 2) + ((PDU_CC >> 2) & 0x3);
    headerBuf[1] = ((PDU_CC & 0x3) << 6) + ssap;
    if (!link_write(headerBuf, 2, 0, 0)) {
        return -2;
    }

    return 1;
}

bool llcp_write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
    uint8_t type;
    uint8_t buf[3];

    if (mode) {
        // Get a SYMM PDU
        if (2 != link_read(buf, sizeof(buf))) {
            return false;
        }
    }

    if (headerBufLen < (hlen + 3)) {
        return false;
    }

    for (int8_t i = hlen - 1; i >= 0; i--) {
        headerBuf[i + 3] = header[i];
    }

    headerBuf[0] = (dsap << 2) + (PDU_I >> 2);
    headerBuf[1] = ((PDU_I & 0x3) << 6) + ssap;
    headerBuf[2] = (ns << 4) + nr;
    if (!link_write(headerBuf, 3 + hlen, body, blen)) {
        return false;
    }

    ns++;

    // Get a RR PDU
    int16_t status;
    do {
        status = link_read(headerBuf, headerBufLen);
        if (2 > status) {
            return false;
        }

        type = getPType(headerBuf);
        if (PDU_RR == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link_write(SYMM_PDU, sizeof(SYMM_PDU), 0, 0)) {
                return false;
            }
        } else {
            return false;
        }
    } while (1);

    if (!link_write(SYMM_PDU, sizeof(SYMM_PDU), 0, 0)) {
        return false;
    }

    return true;
}

int16_t llcp_read(uint8_t *buf, uint8_t length) {
    uint8_t type;
    uint16_t status;

    // Get INFO PDU
    do {
        status = link_read(buf, length);
        if (2 > status) {
            return -1;
        }

        type = getPType(buf);
        if (PDU_I == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link_write(SYMM_PDU, sizeof(SYMM_PDU), 0, 0)) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    uint8_t len = status - 3;
    ssap = getDSAP(buf);
    dsap = getSSAP(buf);

    headerBuf[0] = (dsap << 2) + (PDU_RR >> 2);
    headerBuf[1] = ((PDU_RR & 0x3) << 6) + ssap;
    headerBuf[2] = (buf[2] >> 4) + 1;
    if (!link_write(headerBuf, 3, 0, 0)) {
        return -2;
    }

    for (uint8_t i = 0; i < len; i++) {
        buf[i] = buf[i + 3];
    }

    nr++;

    return len;
}
