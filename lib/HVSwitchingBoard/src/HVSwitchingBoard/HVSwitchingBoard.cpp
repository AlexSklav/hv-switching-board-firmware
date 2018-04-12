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

  // initialize channel state
  // Note: high bit means that the channel is off
  memset(state_of_channels_, 255, 5);
  update_all_channels();

  // set the i2c clock
  Wire.setClock(HV_SWITCHING_BOARD_I2C_RATE);
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
  uint8_t cmd = cmd_ & B00111111;
  bool auto_increment = (1 << 7) & cmd_;
  uint8_t port;

  if ( (cmd >= PCA9505_CONFIG_IO_REGISTER_) &&
       (cmd <= PCA9505_CONFIG_IO_REGISTER_ + 4) ) {
    return_code_ = RETURN_OK;
    send_payload_length_ = false;
    port = cmd - PCA9505_CONFIG_IO_REGISTER_;
    if (payload_length_ == 0) {
      serialize(&config_io_register_[port], 1);
    } else if (payload_length_ == 1) {
      config_io_register_[port] = read<uint8_t>();
      serialize(&config_io_register_[port], 1);
    } else if (auto_increment && port+payload_length_ <= 5) {
      for (uint8_t i = port; i < port+payload_length_; i++) {
        config_io_register_[i] = read<uint8_t>();
      }
      serialize(&config_io_register_[port+payload_length_-1], 1);
    } else {
      return_code_ = RETURN_GENERAL_ERROR;
    }
  } else if ( (cmd >= PCA9505_OUTPUT_PORT_REGISTER_) &&
       (cmd <= PCA9505_OUTPUT_PORT_REGISTER_ + 4) ) {
    return_code_ = RETURN_OK;
    send_payload_length_ = false;
    port = cmd - PCA9505_OUTPUT_PORT_REGISTER_;
    if (payload_length_ == 0) {
      serialize(&state_of_channels_[port], 1);
    } else if (payload_length_ == 1) {
      state_of_channels_[port] = read<uint8_t>();
      serialize(&state_of_channels_[port], 1);
      update_all_channels();
    } else if (auto_increment && port+payload_length_ <= 5) {
      for (uint8_t i = port; i < port+payload_length_; i++) {
        state_of_channels_[i] = read<uint8_t>();
      }
      serialize(&state_of_channels_[port+payload_length_-1], 1);
      update_all_channels();
    } else {
      return_code_ = RETURN_GENERAL_ERROR;
    }
  } else if (cmd_ == CMD_REBOOT) {
    // Reboot.
    Serial.println("Rebooting...");
    do {
      wdt_enable(WDTO_15MS);
      for(;;)
      {
      }
    } while(0);
  } else if (cmd_ == CMD_RESET_CONFIG) {
    load_config(true);
  } else {
    // emulate the PCA9505 config io registers (used by the control board to
    // determine the chip type)
    BaseNode::process_wire_command();
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
    shiftOutFast(S_MOSI, S_SCK, MSBFIRST, ~state_of_channels_[4-i]);
#elif ___HARDWARE_MAJOR_VERSION___==3
  // Version 3 hardware uses **hardware** SPI.
    SPI.transfer(~state_of_channels_[4-i]);
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
