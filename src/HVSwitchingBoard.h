#ifndef ___HV_SWITCHING_BOARD__H___
#define ___HV_SWITCHING_BOARD__H___

#include <BaseNode.h>

class HVSwitchingBoardClass : public BaseNode {
public:
  // PCA9505 (gpio) chip/register addresses (for emulation)
  static const uint8_t PCA9505_CONFIG_IO_REGISTER_ = 0x18;
  static const uint8_t PCA9505_OUTPUT_PORT_REGISTER_ = 0x08;

  // digital pins
  static const uint8_t OE = 8;
  static const uint8_t SRCLR = 9;
  static const uint8_t S_SS = 3;
  static const uint8_t S_SCK = 4;
  static const uint8_t S_MOSI = 5;

  HVSwitchingBoardClass();
  void begin();
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
