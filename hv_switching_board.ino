#include <Wire.h>
#include <BaseNode.h>

#include "HVSwitchingBoard.h"

void setup() {
  HVSwitchingBoard.begin();
}

void loop() {
  HVSwitchingBoard.listen();
}
