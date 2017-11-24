#include <Wire.h>
#include <EEPROM.h>
#include <BaseNode.h>

#include "HVSwitchingBoard.h"


void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void) {
  MCUSR = 0;
  wdt_disable();
  return;
}


void setup() {
  HVSwitchingBoard.begin();
}

void loop() {
  HVSwitchingBoard.listen();
}
