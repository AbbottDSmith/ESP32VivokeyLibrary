#include "API2NFC.h"

API2NFC::API2NFC(PN532& nfc) {
  _nfc = &nfc;
}

void API2NFC::begin() {
  _nfc -> begin();
  while(!(uint32_t(_nfc -> getFirmwareVersion()))) { 
    Serial.println(F("Didn't find PN53x board. Trying again!"));
    delay(500); 
    _nfc -> begin();
  }
  _nfc -> setPassiveActivationRetries(0xFF);
  _nfc -> SAMConfig();
  Serial.println(F("PN532 connected."));
}

bool API2NFC::findSpark2(uint8_t *uid) {
  uint8_t length = 0xff;
  if (_nfc -> readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &length, 25000, true)) {  //Inlists tag. Stores UID in 'uid'
    uint8_t nfcresponse[64] = {0}, nfcresponseLength = 0xFF;
    uint8_t NFC_Select[] = { 0x00, 0xa4, 0x04, 0x0c, 0x07, 0xd2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00 };
    if (_nfc -> inDataExchange(NFC_Select, sizeof(NFC_Select), nfcresponse, &nfcresponseLength)) {
	  if(memcmp(nfcresponse, emptyArray, 16)==0) {
	    _nfc -> inRelease();
	    return false;
	  }
	  return true;
    }
	Serial.println("NFC ERROR: AID Not Found On Tag, Probably Not Spark 2.");
	_nfc -> inRelease();
    return false;
  }

  Serial.println("NFC ERROR: Timeout");
  _nfc -> inRelease();
  return false;
}

bool API2NFC::getPCDChallenge(uint8_t *nfcResponse) {
  uint8_t NFC_AuthenticateFirst[] = { 0x90, 0x71, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00 };
  uint8_t nfcresponse[64] = {0}, nfcresponseLength = 0xFF;
  if (_nfc -> inDataExchange(NFC_AuthenticateFirst, sizeof(NFC_AuthenticateFirst), nfcresponse, &nfcresponseLength)) {
    memcpy(nfcResponse, nfcresponse, 16);
	if(memcmp(nfcResponse, emptyArray, 16)==0) {
	  _nfc -> inRelease();
	  return false;
	}
	return true;
  }
  
  Serial.println("NFC ERROR: Get PCD Challenge.");
  _nfc -> inRelease();
  return false;
}

bool API2NFC::getPICCResponse(uint8_t PCDResponse[32], uint8_t *piccResponse) {
  uint8_t NFC_AuthenticateFirstPart2[38] = { 0x90, 0xaf, 0x00, 0x00, 0x20 };
  for (int i = 0; i < 32; i++) {
   NFC_AuthenticateFirstPart2[5 + i] = PCDResponse[i];
  }
  NFC_AuthenticateFirstPart2[37] = 0;
  uint8_t nfcresponse[64] = {0}, nfcresponseLength = 0xFF;
  if (_nfc -> inDataExchange(NFC_AuthenticateFirstPart2, sizeof(NFC_AuthenticateFirstPart2), nfcresponse, &nfcresponseLength)) {
    _nfc -> inRelease();
	memcpy(piccResponse, nfcresponse, 32);
	return !(memcmp(piccResponse, emptyArray, 32)==0);
  }
  Serial.println("NFC ERROR: Get PICC Response.");
  return false;
}