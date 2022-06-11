#ifndef API2Json_h
#define API2Json_h

#include "Arduino.h"
#include "HTTPClient.h"

class API2Json
{
  public:
    API2Json(const char* APIKEY);
    void begin(const char* SSID, const char * PASS);
	void setHost(const char * host);
    void setRootCACert(const char * cert);
	
    bool getChallenge(uint8_t *piccChallenge);
    bool getPCDResponse(uint8_t _UID[7], uint8_t piccChallenge[16], uint8_t PCDChallenge[16], uint8_t *byteArray);
    bool checkResult(uint8_t _UID[7], uint8_t piccChallenge[16], uint8_t PICCResponse[32]);
    bool storeKVP(uint8_t * challenge, const char* Key, const char* Value);
    String readKVP(uint8_t * challenge, const char* Key);

  private:
    WiFiClientSecure wclientsecure;
	HTTPClient http;
	const char* host = "https://api2.vivokey.com";
	
	uint8_t _APIKEY[30];
	char post[1024];
	String response;
    
    String splitString(String data, char separator, int index);
    void addHexToString(uint8_t array[], unsigned int len, char *finalst);
    void hexStringToByteArray(uint8_t *byteArray, const char *hexString);
    byte nibble(char c);
	uint8_t emptyArray[32] = {0};
};

#endif