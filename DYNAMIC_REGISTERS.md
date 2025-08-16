# Dynamic Shift Register Configuration

This firmware now supports configurable shift register counts for version 4.1 hardware and later.

## Usage

### New Environment: v4_1

The new `v4_1` environment in `platformio.ini` allows you to specify the number of shift registers:

```ini
[env:v4_1]
; Build flags: <major hw version> <minor hw version> <serial baud rate> <i2c rate> <shift register count>
build_flags = !python build_flags.py 4 1 57600 400000 8
```

### Parameters

The `build_flags.py` script now accepts a 5th parameter for the shift register count:

1. **Hardware Major Version**: `4`
2. **Hardware Minor Version**: `1`
3. **Serial Baud Rate**: `57600` (max for 8MHz clock)
4. **I2C Rate**: `400000`
5. **Shift Register Count**: `8` (configurable - default was 5)

### Configuration Examples

For 8 shift registers:
```ini
build_flags = !python build_flags.py 4 1 57600 400000 8
```

For 12 shift registers:
```ini
build_flags = !python build_flags.py 4 1 57600 400000 12
```

For 3 shift registers:
```ini
build_flags = !python build_flags.py 4 1 57600 400000 3
```

### Building

To build with the new configuration:

```bash
pio run -e v4_1
```

To upload:
```bash
pio run -e v4_1 -t upload
```

### Backward Compatibility

Older environments (`v2_1`, `v3_1`) continue to work unchanged with the default 5 shift registers.

### Technical Details

The shift register count is:
- Defined as `___SHIFT_REGISTER_COUNT___` in the generated `Config.h`
- Used as `SHIFT_REGISTER_COUNT` constant in `HVSwitchingBoard.h`
- Applied to array sizes in `HVSwitchingBoard.h` and `HVSwitchingBoard.cpp`
- Used as default template parameter in `ChannelStates.h`

### Hardware Compatibility

Version 4+ hardware uses the same SPI configuration as version 3 (hardware SPI), ensuring compatibility with existing designs while adding the flexibility of configurable shift register counts.

### Python Driver Usage

The Python driver now supports configurable shift register counts:

```python
from hv_switching_board.driver import HVSwitchingBoard

# For 5 shift registers (default)
board = HVSwitchingBoard(proxy, address=10)

# For 8 shift registers
board = HVSwitchingBoard(proxy, address=10, shift_register_count=8)

# For 12 shift registers  
board = HVSwitchingBoard(proxy, address=10, shift_register_count=12)
```

### Upload Script Changes

The upload script now defaults to `v3_1` environment for backward compatibility:

```bash
# Upload with default v3_1 (5 shift registers)
python -m hv_switching_board.bin.upload

# Upload with v4_1 (8 shift registers as configured)
python -m hv_switching_board.bin.upload -b v4_1

# Upload custom configuration
python -m hv_switching_board.bin.upload -b v4_1 -p /dev/ttyUSB0
```

### DropBot Controller Changes

The DropBot controller now **automatically detects** the number of shift registers on each board:

```python
# Just initialize normally - no configuration needed!
proxy = DropBot()

# The controller will automatically query each board and detect:
# - v3_1 firmware (5 shift registers) = 40 channels per board  
# - v4_1 firmware (8 shift registers) = 64 channels per board
# - Mixed setups are supported (different firmware versions per board)

print(f"Total channels detected: {proxy.number_of_channels}")
```

**Automatic Detection**: The controller queries each switching board using the new `CMD_GET_SHIFT_REGISTER_COUNT` command (v4.1+). Older firmware defaults to 5 shift registers for backward compatibility.

### Memory Considerations

Each shift register uses:
- 1 byte in `state_of_channels_[]` array
- 1 byte in `config_io_register_[]` array
- Additional bytes in `WindowChannelStates` template (if used)

Ensure your microcontroller has sufficient RAM for the configured number of registers.
