#include <Wire.h>
#include <EEPROM.h>
#include <BaseNode.h>

#include "HVSwitchingBoard.h"

void setup() {
  // Use 57600 since 115200 does not seem to work on atmega328 running with 8
  // MHz clock.
  HVSwitchingBoard.begin(57600);
}

void loop() {
  HVSwitchingBoard.listen();
}
