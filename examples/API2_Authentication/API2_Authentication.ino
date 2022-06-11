#include "arduino_secrets.h"
#include <ESP32VivokeyLibrary.h>

#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"

PN532_SPI pn532spi(SPI, 34);
PN532 nfc(pn532spi);

API2NFC api2NFC(nfc);
API2Json api2Json(SECRET_APIKEY);

#ifdef RGB_BUILTIN
  #include <FastLED.h>
  CRGB leds[1];
  int hue = 0;
#endif

void setup(void) {

  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println(F("\nHello!"));

  api2NFC.begin();
  api2Json.begin(SECRET_SSID, SECRET_PASS);
  api2Json.setRootCACert(SECRET_ROOTCACERT);


#ifndef RGB_BUILTIN
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(500);
  }
#else
  FastLED.addLeds<WS2812, RGB_BUILTIN, RGB>(leds, 1);
  while(WiFi.status() != WL_CONNECTED) {
    leds[0] = CHSV(90, 255, (hue++ % 2 == 0) ? 255 : 0); //Alternate Red/OFF
    FastLED.show();
    delay(500);
  }
#endif
  Serial.println(F("Wifi Connected!"));

#ifdef BUTTON_PIN
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println(F("Waiting For Button Press..."));
#endif
}

void loop(void) {
  
#ifdef RGB_BUILTIN
  leds[0] = CHSV(hue++, 255, 255);//Rainbow LED = Waiting
  FastLED.show();
#endif
#ifdef BUTTON_PIN
  if (digitalRead(BUTTON_PIN)) return;
  Serial.print(F("Button Pressed! "));
#endif
#ifdef RGB_BUILTIN
  leds[0] = CHSV(155, 255, 255); 
  FastLED.show(); //Blue LED = Thinking
#endif


  Serial.println(F("Grabbing New Challenge from HOST..."));
  uint8_t piccChallenge[16] = {0};
  if(!api2Json.getChallenge(piccChallenge)) return;
  
#ifdef RGB_BUILTIN
  leds[0] = CHSV(255, 255, 255); //Green LED = Challenge Received
  FastLED.show();
#endif

  Serial.println(F("Scanning for tag..."));
  uint8_t uid[7] = {0};
  if(!api2NFC.findSpark2(uid)) return;

  Serial.println(F("Found Spark 2! Grabbing PCD-Challenge From PICC."));
  uint8_t pcdChallenge[16] = {0};
  if(!api2NFC.getPCDChallenge(pcdChallenge)) return;

  Serial.println(F("POSTing PCDChallenge to PCD (/pcd-challenge)"));
  uint8_t pcdResponse[32] = {0};
  if(!api2Json.getPCDResponse(uid, piccChallenge, pcdChallenge, pcdResponse)) return;

  Serial.println(F("Passing Response To PICC"));
  uint8_t piccResponse[32] = {0}; 
  if(!api2NFC.getPICCResponse(pcdResponse, piccResponse)) return;


  Serial.println(F("POSTing Response To PCD (/check-result)"));
  if(api2Json.checkResult(uid, piccChallenge, piccResponse)) {
    Serial.println(F("Authenticated!"));
  } else {
    Serial.println(F("Failed Authentication!"));
  }
}