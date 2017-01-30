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

HVSwitchingBoardClass::HVSwitchingBoardClass() {
}

void HVSwitchingBoardClass::begin() {
  BaseNode::begin();
  
  pinMode(S_SS, OUTPUT);
  pinMode(S_SCK, OUTPUT);
  pinMode(S_MOSI, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(SRCLR, OUTPUT);

  digitalWrite(SRCLR, HIGH);
  digitalWrite(OE, LOW);

  // initialize channel state
  // Note: high bit means that the channel is off
  memset(state_of_channels_, 255, 5);
  update_all_channels();
}

void HVSwitchingBoardClass::process_wire_command() {
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
  // emulate the PCA9505 config io registers (used by the control board to determine
  // the chip type)
  } else {
    BaseNode::process_wire_command();
  }
}

bool HVSwitchingBoardClass::process_serial_input() {
  if (BaseNode::process_serial_input()) {
    return true;
  }
  
  if (match_function(P("state_of_all_channels()"))) {
    for (uint8_t i = 0; i < 5; i++) {
      Serial.println("state_of_channels_[" + String(i) + "]=" + String(state_of_channels_[i]));
    }
    return true;
  }
  return false;
}

void HVSwitchingBoardClass::update_all_channels() {
  digitalWrite(S_SS, LOW);
  for (uint8_t i = 0; i < 5; i++) {
    shiftOutFast(S_MOSI, S_SCK, MSBFIRST, ~state_of_channels_[4-i]);
  }
  digitalWrite(S_SS,  HIGH);
}

void shiftOutFast(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
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
