#ifndef API2NFC_h
#define API2NFC_h

#include "Arduino.h"
#include "PN532.h"


class API2NFC
{
  public:
    API2NFC(PN532 &nfc);
    void begin();
    bool findSpark2(uint8_t *uid);
    bool getPCDChallenge(uint8_t *nfcResponse);
    bool getPICCResponse(uint8_t PCDResponse[32], uint8_t *piccResponse);

  private:
    PN532* _nfc;
	uint8_t emptyArray[32] = {0};
};

#endif