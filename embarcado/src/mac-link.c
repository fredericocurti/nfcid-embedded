#include "mac_link.h"

bool MACLink_write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    return pn532_tgSetData(header, hlen, body, blen);
}

int16_t MACLink_read(uint8_t *buf, uint8_t len)
{
    return pn532_tgGetData(buf, len);
}
