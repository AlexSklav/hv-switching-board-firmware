#include <Wire.h>
#include <EEPROM.h>
#include <BaseNode.h>
#include <ArduinoSTL.h>

#include "HVSwitchingBoard.h"


void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void) {
  MCUSR = 0;
  wdt_disable();
  return;
}


void setup() {
  /*
   * .. versionchanged:: 0.9
   *    Use default baud rate, defined using ``HV_SWITCHING_BOARD_BAUD_RATE``.
   */
  HVSwitchingBoard.begin();
}

void loop() {
  HVSwitchingBoard.listen();
}
