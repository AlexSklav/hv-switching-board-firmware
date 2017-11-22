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
  // Use 57600 since 115200 does not seem to work on atmega328 running with 8
  // MHz clock.
  HVSwitchingBoard.begin(57600);
}

void loop() {
  HVSwitchingBoard.listen();
}
