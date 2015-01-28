#include <Wire.h>
#include <EEPROM.h>
#include <BaseNode.h>

#include "HVSwitchingBoard.h"

void setup() {
  HVSwitchingBoard.begin();
}

void loop() {
  HVSwitchingBoard.listen();
}
