Dependent on the SEEED-STUDIO PN532 Library

Library Installation:

1. Download https://github.com/Seeed-Studio/PN532

2. Extract the 5 PN532_ folders contained within PN532-arduino into your Arduino libraries folder.

3. Navigate to the PN532_SPI folder.

4. Change the first two lines of PN532_SPI.cpp from this: 


#include "PN532/PN532_SPI/PN532_SPI.h"

#include "PN532/PN532/PN532_debug.h"

To this: 

#include "PN532_SPI.h"

#include <PN532_debug.h>
