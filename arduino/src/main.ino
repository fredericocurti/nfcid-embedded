// send a NDEF message to adnroid or get a NDEF message
//
// note: [NDEF library](https://github.com/Don/NDEF) is needed.

#include "PN532_HSU.h"
#include "snep.h"
#include "NdefMessage.h"

PN532_HSU pn532hsu(Serial1);
SNEP nfc(pn532hsu);
uint8_t ndefBuf[128];

void setup()
{
    Serial.begin(115200);
    Serial.println("-------Peer to Peer--------");
}

void loop() {
    // it seems there are some issues to use NdefMessage to decode the received data from Android
    Serial.println("Get a message from Android");
    int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
    if (msgSize > 0) {
        NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
        msg.print();
        Serial.println("\nSuccess");
    } else {
        Serial.println("failed");
    }
    delay(1500);
}
