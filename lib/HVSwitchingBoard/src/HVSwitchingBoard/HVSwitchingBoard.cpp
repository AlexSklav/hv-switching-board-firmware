#include <algorithm>
#include "HVSwitchingBoard.h"
#include "Config.h"

#define P(str) (strcpy_P(p_buffer_, PSTR(str)), p_buffer_)

void shiftOutFast(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
HVSwitchingBoardClass HVSwitchingBoard;

const char BaseNode::PROTOCOL_NAME_[] PROGMEM = "Extension module protocol";
const char BaseNode::PROTOCOL_VERSION_[] PROGMEM = "0.1";
const char BaseNode::MANUFACTURER_[] PROGMEM = "Wheeler Microfluidics Lab";
const char BaseNode::NAME_[] PROGMEM = "HV Switching Board";
const char BaseNode::HARDWARE_VERSION_[] PROGMEM = ___HARDWARE_VERSION___;
const char BaseNode::SOFTWARE_VERSION_[] PROGMEM = ___SOFTWARE_VERSION___;
const char BaseNode::URL_[] PROGMEM = "http://microfluidics.utoronto.ca/dropbot";

HVSwitchingBoardClass::HVSwitchingBoardClass() {}

void HVSwitchingBoardClass::begin(uint32_t baud_rate) {
  /*
   * .. versionchanged:: 0.9
   *    Support both hardware major versions 2 and 3.
   *
   * .. versionchanged:: X.X.X
   *    Fill ``state_of_channels_`` as **active HIGH**.
   */
  BaseNode::begin(baud_rate);

#if ___HARDWARE_MAJOR_VERSION___==2
  // Version 2 hardware uses **software** SPI.
  pinMode(S_SS, OUTPUT);
  pinMode(S_SCK, OUTPUT);
  pinMode(S_MOSI, OUTPUT);
#elif ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
  pinMode(SS_595, OUTPUT);
  digitalWrite(SS_595, HIGH);

  // initialize SPI:
  SPI.begin();
#endif
  pinMode(OE, OUTPUT);
  pinMode(SRCLR, OUTPUT);

  digitalWrite(SRCLR, HIGH);
  digitalWrite(OE, LOW);

  // Initialize channel states
  state_of_channels_.fill(0);
  update_all_channels();

  // set the i2c clock
  Wire.setClock(HV_SWITCHING_BOARD_I2C_RATE);

  // By default, enable receiving of broadcast messages (i.e., messages sent to
  // address 0).  This can be enabled/disabled through the
  // `CMD_SET_GENERAL_CALL_ENABLED` I2C command.
  general_call(true);
}

void HVSwitchingBoardClass::process_wire_command() {
  /*
   * .. versionchanged:: 0.8
   *    Add reboot command.
   *
   * .. versionchanged:: 0.10
   *    Add command to reset configuration.
   */
  return_code_ = RETURN_UNKNOWN_COMMAND;
  uint8_t register_addr = cmd_ & B00111111;
  bool auto_increment = (1 << 7) & cmd_;
  uint8_t port;

  if ((register_addr >= PCA9505_CONFIG_IO_REGISTER_) &&
      (register_addr <= PCA9505_CONFIG_IO_REGISTER_ + 4)) {
    // Emulate the PCA9505 config io registers (used by the control board to
    // determine the chip type)
    port_operation(config_io_register_, register_addr -
                   PCA9505_CONFIG_IO_REGISTER_, auto_increment);
  } else if ((register_addr >= PCA9505_OUTPUT_PORT_REGISTER_) &&
             (register_addr <= PCA9505_OUTPUT_PORT_REGISTER_ + 4)) {
    // Emulate the PCA9505 output registers.
    if (port_operation(state_of_channels_, register_addr -
                       PCA9505_OUTPUT_PORT_REGISTER_, auto_increment,
                       // Invert from **active LOW** to **active HIGH**.
                       true) > 0) {
      // At least one port was updated.  Propagate update to channel states.
      update_all_channels();
    }
  } else {
    switch (cmd_) {
      case CMD_GET_GENERAL_CALL_ENABLED:
        {
          const uint8_t general_call_enabled = general_call();
          serialize(&general_call_enabled, sizeof(general_call_enabled));
        }
        return_code_ = RETURN_OK;
        break;
      case CMD_SET_GENERAL_CALL_ENABLED:
        {
          const uint8_t general_call_enabled = read<uint8_t>();
          general_call(general_call_enabled);
        }
        return_code_ = RETURN_OK;
        break;
      case CMD_REBOOT:
        // Reboot.
        Serial.println("Rebooting...");
        do {
          wdt_enable(WDTO_15MS);
          for(;;)
          {
          }
        } while(0);
        break;
      case CMD_RESET_CONFIG:
        load_config(true);
        break;
      default:
        BaseNode::process_wire_command();
        break;
    }
  }
}

bool HVSwitchingBoardClass::process_serial_input() {
  /*
   * .. versionchanged:: 0.8
   *    Add ``reboot()`` serial command.
   */
  if (BaseNode::process_serial_input()) {
    return true;
  }

  if (match_function(P("state_of_all_channels()"))) {
    for (uint8_t i = 0; i < 5; i++) {
      Serial.println("state_of_channels_[" + String(i) + "]=" + String(state_of_channels_[i]));
    }
    return true;
  } else if (match_function(P("reboot()"))) {
    Serial.println("Rebooting...");
    do {
      wdt_enable(WDTO_15MS);
      for(;;)
      {
      }
    } while(0);
  }
  return false;
}

void HVSwitchingBoardClass::update_all_channels() {
  /*
   * .. versionchanged:: 0.9
   *    Support both hardware major versions 2 and 3.
   */
  const uint8_t port_count = 5;
#if ___HARDWARE_MAJOR_VERSION___==2
  // Version 2 hardware uses **software** SPI.
  const uint8_t spi_chip_select_pin = S_SS;
#elif ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
  const uint8_t spi_chip_select_pin = SS_595;
#endif

  // Select PCA9505 chips for SPI access.
  digitalWrite(spi_chip_select_pin,  LOW);
  // Copy cached channel states to outputs of PCA9505 chips.
  for (uint8_t i = 0; i < port_count; i++) {
#if ___HARDWARE_MAJOR_VERSION___==2
  // Version 2 hardware uses **software** SPI.
    shiftOutFast(S_MOSI, S_SCK, MSBFIRST, state_of_channels_[4 - i]);
#elif ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
    SPI.transfer(state_of_channels_[4 - i]);
#endif
  }
  // Release PCA9505 chips for SPI access.
  digitalWrite(spi_chip_select_pin,  HIGH);
}

void shiftOutFast(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder,
                  uint8_t val) {
  uint8_t cnt;
  uint8_t bitData, bitNotData;
  uint8_t bitClock, bitNotClock;
  volatile uint8_t *outData;
  volatile uint8_t *outClock;

  outData = portOutputRegister(digitalPinToPort(dataPin));
  outClock = portOutputRegister(digitalPinToPort(clockPin));
  bitData = digitalPinToBitMask(dataPin);
  bitClock = digitalPinToBitMask(clockPin);

  bitNotClock = bitClock;
  bitNotClock ^= 0x0ff;

  bitNotData = bitData;
  bitNotData ^= 0x0ff;

  cnt = 8;
  if (bitOrder == LSBFIRST) {
    do {
      if (val & 1)
  *outData |= bitData;
      else
  *outData &= bitNotData;

      *outClock |= bitClock;
      *outClock &= bitNotClock;
      val >>= 1;
      cnt--;
    } while( cnt != 0 );
  } else {
    do {
      if (val & 128) {
  *outData |= bitData;
      } else {
  *outData &= bitNotData;
      }
      *outClock |= bitClock;
      *outClock &= bitNotClock;
      val <<= 1;
      cnt--;
    } while( cnt != 0 );
  }
}
