/**
 * @since **0.9**: Support both hardware major versions 2 and 3.
 * @since **0.10**: Add command to reset configuration.
 * @since **0.12**: Add **I2C broadcast** receiving **getter** and **setter**.
 */
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

#ifndef HV_SWITCHING_BOARD_I2C_RATE
/*
 * Set default I2C rate to 400 kHz.
 *
 * See https://www.arduino.cc/en/Reference/WireSetClock for more info.
 */
#define HV_SWITCHING_BOARD_I2C_RATE 400000
#endif

class HVSwitchingBoardClass : public BaseNode {
public:
  //! PCA9505 (gpio) chip **configuration** register address (for emulation)
  static constexpr uint8_t PCA9505_CONFIG_IO_REGISTER_ = 0x18;
  //! PCA9505 (gpio) chip **output** register address (for emulation)
  static constexpr uint8_t PCA9505_OUTPUT_PORT_REGISTER_ = 0x08;

  //! Perform a software reboot.
  static constexpr uint8_t CMD_REBOOT = 0xA2;
  //! Reset configuration to default.
  static constexpr uint8_t CMD_RESET_CONFIG = 0xA3;
  /**
   * @brief Enable/disable receiving of broadcast messages (i.e., messages sent
   * to address 0).
   */
  static constexpr uint8_t CMD_SET_GENERAL_CALL_ENABLED = 0xA4;
   /**
    * @brief Get current broadcast receiving setting.
    */
  static constexpr uint8_t CMD_GET_GENERAL_CALL_ENABLED = 0xA5;

  // digital pins
  static constexpr uint8_t OE = 8;
  static constexpr uint8_t SRCLR = 9;
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

  /**
   * @brief Initialize board.
   *
   * @since **0.9**: Support both hardware major versions 2 and 3.
   *
   * @param baud_rate Serial interface baud rate.
   */
  void begin(uint32_t baud_rate);
  void begin() { begin(HV_SWITCHING_BOARD_BAUD_RATE); }
  /**
   * @brief Process any requests available from **I2C/Wire** input
   *
   * @since **0.8**: Add \link CMD_REBOOT reboot command\endlink.
   * @since **0.10**: Add \link CMD_RESET_CONFIG command to reset configuration\endlink.
   * @since **0.12**: Add **I2C broadcast** receiving \link CMD_GET_GENERAL_CALL_ENABLED **getter**\endlink and
   *   \link CMD_SET_GENERAL_CALL_ENABLED **setter**\endlink commands.
   *
   * ## Commands
   *
   * | I2C RX                                       | Action                              | I2C TX                    |
   * |----------------------------------------------|-------------------------------------|---------------------------|
   * | `[#PCA9505_CONFIG_IO_REGISTER_+p]`           | N/A                                 | `#config_io_register_[p]` |
   * | `[#PCA9505_CONFIG_IO_REGISTER_+p, v]`        | `#config_io_register_[p] = v`       | N/A                       |
   * | `[#PCA9505_CONFIG_IO_REGISTER_+p, v1..vn]`   | `#config_io_register_[p:] = v1..vn` | N/A                       |
   * | `[#PCA9505_OUTPUT_PORT_REGISTER_+p]`         | N/A                                 | `#state_of_channels_[p]`  |
   * | `[#PCA9505_OUTPUT_PORT_REGISTER_+p, v]`      | `#state_of_channels_[p] = v`        | N/A                       |
   * | `[#PCA9505_OUTPUT_PORT_REGISTER_+p, v1..vn]` | `#state_of_channels_[p:] = v1..vn`  | N/A                       |
   * | `[#CMD_REBOOT]`                              | Reboot                              | N/A                       |
   * | `[#CMD_RESET_CONFIG]`                        | Reset config to default             | N/A                       |
   * | `[#CMD_SET_GENERAL_CALL_ENABLED, v]`         | Receive I2C broadcasts if `v`       | N/A                       |
   * | `[#CMD_GET_GENERAL_CALL_ENABLED, v]`         | N/A                                 | `[<receiving broadcasts]` |
   *
   * @return `true` if a request was processed.
   */
  void process_wire_command();
  /**
   * @brief Process any requests available from **serial input**
   *
   * @since **0.8**: Add `reboot()` serial command.
   *
   * @return `true` if a request was processed.
   */
  bool process_serial_input();
  /**
   * @brief Enable/disable receiving of broadcasts, i.e., messages sent to
   * address 0.
   *
   * @param state  If `true`, **enable**.  Otherwise, **disable**.
   */
  void general_call(bool state) {
    if (state) {
      // Enable receiving of broadcasts, i.e., messages sent to address 0.
      TWAR |= 1;
    } else {
      // Disable receiving of broadcasts, i.e., messages sent to address 0.
      TWAR &= ~0x01;
    }
  }
  /**
   * @brief Broadcast receiving setting.
   *
   * @return `true` if receiving of broadcasts is **enabled**.
   */
  bool general_call() const { return TWAR & 0x01; }
protected:
  bool supports_isp() { return true; }
private:
  /**
   * @brief Propagate channel states from #state_of_channels_ to output
   * registers.
   *
   * @since **0.9**: Support both hardware major versions 2 and 3.
   */
  void update_all_channels();
  //! Requested state of channels (packed, one bit per channel).
  uint8_t state_of_channels_[5];
  //! Configuration registers to emulate PCA9505 protocol.
  uint8_t config_io_register_[5];

  /**
   * @brief **Read/write operation** to/from one or more register ports
   *
   * where:
   *
   *  - The command (BaseNode::cmd_) corresponds to the starting address
   *  - The type of operation is defined by the length of the payload
   *    (BaseNode::payload_length_) .
   *
   * @tparam Ports  Ports array type (e.g., `uint8_t[]`)
   * @param ports  Ports array
   * @param port  Starting port index within array
   * @param auto_increment  Assign full payload to consecutive array addresses
   *
   * @return
   */
  template <typename Ports>
  int port_operation(Ports &ports, uint8_t port, bool auto_increment,
                     bool invert=false) {
    return_code_ = RETURN_OK;
    send_payload_length_ = false;

    if (payload_length_ == 0) {
      // Empty payload corresponds to a **read** operation.
      auto value = ports[port];
      if (invert) {
        value = ~value;
      }
      serialize(&value, 1);
      return 0;
    } else if (payload_length_ == 1) {
      // A single byte payload corresponds to a **write** operation to a single
      // port.
      auto value = read<uint8_t>();
      ports[port] = (invert) ? ~value : value;
      serialize(&value, 1);
      return 1;
    } else if (auto_increment && (port + payload_length_ <= 5)) {
      // Auto-increment was specified.
      // Sequentially write to consecutive ports, one byte at a time, starting
      // at the first byte in the payload and continue until the last byte in
      // the payload.
      uint8_t value = 0;
      for (uint8_t i = port; i < port + payload_length_; i++) {
        value = read<uint8_t>();
        ports[i] = (invert) ? ~value : value;
      }
      serialize(&value, 1);
      return payload_length_;
    } else {
      return_code_ = RETURN_GENERAL_ERROR;
    }
    return -1;
  }
};

extern HVSwitchingBoardClass HVSwitchingBoard;

#endif // ___HV_SWITCHING_BOARD__H___
