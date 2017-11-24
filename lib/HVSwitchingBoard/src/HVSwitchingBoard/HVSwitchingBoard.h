#ifndef ___HV_SWITCHING_BOARD__H___
#define ___HV_SWITCHING_BOARD__H___

#include <avr/wdt.h>
#if ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
#include <SPI.h>
#endif
#include <BaseNode.h>

#ifndef HV_SWITCHING_BOARD_BAUD_RATE
/*
 * .. note::
 *     ATMEGA328 running with **8 MHz clock** **MUST** use baud rate 57600 **at
 *     most**.
 *
 *    For example, **115200 baud rate** does **not** work **8 MHz clock**.
 */
#define HV_SWITCHING_BOARD_BAUD_RATE 57600
#endif

class HVSwitchingBoardClass : public BaseNode {
public:
  // PCA9505 (gpio) chip/register addresses (for emulation)
  static const uint8_t PCA9505_CONFIG_IO_REGISTER_ = 0x18;
  static const uint8_t PCA9505_OUTPUT_PORT_REGISTER_ = 0x08;

  static const uint8_t CMD_REBOOT = 0xA2;

  // digital pins
  static const uint8_t OE = 8;
  static const uint8_t SRCLR = 9;
#if ___HARDWARE_MAJOR_VERSION___==2
  // Version 2 hardware uses **software** SPI.
  static const uint8_t S_SS = 3;
  static const uint8_t S_SCK = 4;
  static const uint8_t S_MOSI = 5;
#elif ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
  static const uint8_t SS_595 = 3;
#endif

  HVSwitchingBoardClass();
  void begin(uint32_t baud_rate);
  void begin() { begin(HV_SWITCHING_BOARD_BAUD_RATE); }
  void process_wire_command();
  bool process_serial_input();
protected:
  bool supports_isp() { return true; }
private:
  void update_all_channels();
  uint8_t state_of_channels_[5];
  uint8_t config_io_register_[5];
};

extern HVSwitchingBoardClass HVSwitchingBoard;

#endif // ___HV_SWITCHING_BOARD__H___
