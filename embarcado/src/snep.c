#include "snep.h"
#include "pn532.h"

int16_t snep_read(uint8_t *buf, uint8_t len, uint16_t timeout){
 if (0 >= llcp_activate(timeout)) {
		DMSG("failed to activate PN532 as a target\n");
		return -1;
	}

	if (0 >= llcp_waitForConnection(timeout)) {
		DMSG("failed to set up a connection\n");
		return -2;
	}

	uint16_t status = llcp_read(buf, len);
	if (6 > status) {
		return -3;
	}

	// check SNEP version
	if (SNEP_DEFAULT_VERSION != buf[0]) {
		DMSG("The received SNEP message's major version is different\n");
		// To-do: send Unsupported Version response
		return -4;
	}

	// expect a put request
	if (SNEP_REQUEST_PUT != buf[1]) {
		DMSG("Expect a put request\n");
		return -4;
	}

	// check message's length
	uint32_t length = (buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5];
	// length should not be more than 244 (header + body < 255, header = 6 + 3 + 2)
	if (length > (status - 6)) {
		DMSG("The SNEP message is too large: "); 
        DMSG_INT(length);
        DMSG_INT(status - 6);
		DMSG("\n");
		return -4;
	}
	for (uint8_t i = 0; i < length; i++) {
		buf[i] = buf[i + 6];
	}

	// response a success SNEP message
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_RESPONSE_SUCCESS;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = 0;
	llcp_write(headerBuf, 6, 0, 0);
	
	return length;
}
