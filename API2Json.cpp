#include "API2Json.h"

API2Json::API2Json(const char *APIKEY)
{
  hexStringToByteArray(_APIKEY, APIKEY);
}

void API2Json::begin(const char* SSID, const char * PASS) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SSID, PASS); //Attempts connection to WiFi.
  http.useHTTP10(false);
  http.setConnectTimeout(5000);
  http.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36");
}

void API2Json::setHost(const char * HOST)
{
  host = HOST;
}

void API2Json::setRootCACert(const char * cert)
{
  wclientsecure.setCACert(cert);
}

bool API2Json::getChallenge(uint8_t *challenge)//needs to be called once every 30 seconds for fresh challenge
{
  memset(post, 0, sizeof(post));
  strcat(post, "{\"api-key\":\"");
  addHexToString(_APIKEY, 30, post);
  strcat(post, "\"}");
  
  http.begin(wclientsecure, host);
  
  http.setURL(F("/v1/get-challenge"));
  http.addHeader(F("Content-Type"), F("application/json"));
  
  int httpCode = http.POST(post);
  memset(post, 0, sizeof(post));
  if(!(httpCode > 0)) {
	  Serial.println("Get-Challenge Post Error Code: " + http.errorToString(httpCode) );
	  http.end();
	  return false;
  }
  
  response = http.getString();
  
  if (response.substring(2, 16).equals(F("picc-challenge"))) {
	hexStringToByteArray(challenge, response.substring(19, 51).c_str());
	return !(memcmp(challenge, emptyArray, 16)==0);
  } 
  
  http.end();
  return false;
}

bool API2Json::getPCDResponse(uint8_t uid[7], uint8_t piccChallenge[16], uint8_t PCDChallenge[16], uint8_t *byteArray)
{
  uint8_t msbUID[] = { uid[6], uid[5], uid[4], uid[3], uid[2], uid[1], uid[0] };  //Big Endian to Little Endian ¯\_(ツ)_/¯

  strcat(post, "{\"picc-uid\":\"");
  addHexToString(msbUID, 7, post);
  strcat(post, "\",\"picc-challenge\":\"");
  addHexToString(piccChallenge, 16, post);
  strcat(post, "\",\"pcd-challenge\":\"");
  addHexToString(PCDChallenge, 16, post);
  strcat(post, "\"}");
  
  http.setURL(F("/v1/pcd-challenge"));
  http.addHeader(F("Content-Type"), F("application/json"));

  int httpCode = http.POST(post);
  memset(post, 0, sizeof(post));
  if(!(httpCode > 0)) {
	  Serial.println("PCD-Challenge Post Error Code: " + http.errorToString(httpCode) );
	  http.end();
	  return false;
  }
  
  response = http.getString();
  
  if (response.substring(2, 14).equals(F("pcd-response"))) {
    hexStringToByteArray(byteArray, response.substring(17, 83).c_str());  //Stores pcdChallenge into Byte array.
    return !(memcmp(byteArray, emptyArray, 32)==0);
  }
  http.end();
  return false;
}

bool API2Json::checkResult(uint8_t uid[7], uint8_t piccChallenge[16], uint8_t PICCResponse[32])
{
  uint8_t msbUID[] = { uid[6], uid[5], uid[4], uid[3], uid[2], uid[1], uid[0] };

  strcat(post, "{\"picc-uid\":\"");
  addHexToString(msbUID, 7, post);
  strcat(post, "\",\"picc-challenge\":\"");
  addHexToString(piccChallenge, 16, post);
  strcat(post, "\",\"picc-response\":\"");
  addHexToString(PICCResponse, 32, post);
  strcat(post, "\"}");
  
  http.setReuse(false);
  http.setURL(F("/v1/check-result"));
  http.addHeader(F("Content-Type"), F("application/json"));

  int httpCode = http.POST(post);
  memset(post, 0, sizeof(post));
  if(!(httpCode > 0)) {
	  Serial.println("Check-Result Post Error Code: " + http.errorToString(httpCode) );
	  http.end();
	  return false;
  }

  response = http.getString();
  
  if ((response.substring(2, 14).equals(F("check-result")) && !response.substring(17, 22).equals(F("error")))) {
    return true;
  }
  http.end();
  return false;
}

bool API2Json::storeKVP(uint8_t * challenge, const char* Key, const char* Value)
{
  
  strcat(post, "{\"challenge\":\"");
  addHexToString(challenge, 16, post);
  strcat(post, "\",\"dict\":[{\"key\":\"");
  strcat(post, Key);
  strcat(post, "\",\"value\":\"");
  strcat(post, Value);
  strcat(post, "\"}]}");
  
  http.setURL(F("/v1/kvp-store"));
  http.addHeader(F("Content-Type"), F("application/json"));
  
  int httpCode = http.POST(post);
  memset(post, 0, sizeof(post));
  if(!(httpCode > 0)) {
	  Serial.println("KVP-Store Post Error Code: " + http.errorToString(httpCode) );
	  http.end();
	  return false;
  }
  
  response = http.getString();

  return true;
}

String API2Json::readKVP(uint8_t * challenge, const char * Key)
{
  strcat(post, "{\"challenge\":\"");
  addHexToString(challenge, 16, post);
  strcat(post, "\",\"dict\":[\"");
  strcat(post, Key);
  strcat(post, "\"]}");

  http.setURL(F("/v1/kvp-read"));
  http.addHeader(F("Content-Type"), F("application/json"));
  
  int httpCode = http.POST(post);
  memset(post, 0, sizeof(post));
  if(!(httpCode > 0)) {
	  Serial.println("KVP-Read Post Error Code: " + http.errorToString(httpCode) );
	  http.end();
	  return "";
  }
  
  response = http.getString();
  return splitString(response, '"', 9);
}

//Source: https://arduino.stackexchange.com/a/1237
String API2Json::splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void API2Json::addHexToString(uint8_t array[], unsigned int arrayLen, char *finalst) {
  for (int i = 0; i < arrayLen; i++) {
    char str[3];
    sprintf(str, "%02x", (int)array[i]);
    strcat(finalst, str);
  }
}

//Source: https://forum.arduino.cc/t/hex-string-to-byte-array/563827/3
//Turns CharArray '010203' into byte array {0x01, 0x02, 0x03}
void API2Json::hexStringToByteArray(uint8_t *byteArray, const char *hexString) {
  bool oddLength = strlen(hexString) & 1;
  uint8_t currentByte = 0;
  uint8_t byteIndex = 0;
  for (uint8_t charIndex = 0; charIndex < strlen(hexString); charIndex++) {
    bool oddCharIndex = charIndex & 1;
    if (oddLength) {
      if (oddCharIndex) {
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    } else {
      if (!oddCharIndex) {
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

byte API2Json::nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return 0;
}

