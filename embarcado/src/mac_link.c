#include "mac_link.h"

int8_t link_activateAsTarget(uint16_t timeout) {
	pn532_begin();
	pn532_SAMConfig();
	return pn532_tgInitAsTarget(timeout);
}

bool link_write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
    return pn532_tgSetData(header, hlen, body, blen);
}

int16_t link_read(uint8_t *buf, uint8_t len) {
    return pn532_tgGetData(buf, len);
}
