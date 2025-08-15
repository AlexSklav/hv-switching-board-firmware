Import("env")
from pathlib import Path
from intelhex import IntelHex
from SCons.Script import AlwaysBuild

# Use PIO's configured avrdude ($UPLOADER) and flags ($UPLOAD_FLAGS)
# so it automatically picks your port, baud, programmer, mcu, avrdude.conf, etc.
DEFAULT_COMMAND = '$UPLOADER $UPLOADERFLAGS -P $UPLOAD_PORT -b $UPLOAD_SPEED '
SET_FUSES_COMMAND = '-U lfuse:w:0xEF:m -U hfuse:w:0xD4:m -U efuse:w:0xFD:m '


def make_eeprom(variant_value, filename, size=64, fill=0xFF):
    assert 0 <= variant_value <= 0xFF

    ih = IntelHex()
    # 1) Initialize everything to fill 0xFF
    ih.frombytes(bytearray([fill]*size), offset=0x0000)
    # 2) Zero-out 0x0000..0x0029 (inclusive)
    ih.frombytes(bytearray([0x00] * 0x2A), offset=0x0000)
    # 3) Set required bytes
    ih[0x0004] = 0x03  # fixed marker/version byte
    ih[0x0006] = variant_value # variant value

    with open(filename, "w") as f:
        ih.write_hex_file(f, byte_count=32)


def do_setfuses(source, target, env):
    """Set the AVR fuses to specific values.
    This is useful for setting the clock source, boot
    size, and other low-level settings.
    """

    env.AutodetectUploadPort()
    return env.Execute(env.VerboseAction(
        DEFAULT_COMMAND + SET_FUSES_COMMAND,
        "Setting AVR fuses (lfuse=0xEF, hfuse=0xD4, efuse=0xFD)"
    ))

def do_seteeprom(source, target, env):
    """
    Generate an EEPROM file with the address at byte 0 and flash it.
    """
    print("Enter the EEPROM address you wish to set:")
    addr = input()
    try:
        addr = int(addr)
    except ValueError:
        print("Invalid address. Defaulting to 32.")
        addr = int(env.GetProjectOption("board_default_address", "32"))

    eep_path = Path(env.subst("$PROJECT_DIR")) / ".pio" / "eeproms" / f"eeprom-address{addr}.hex"
    eep_path.parent.mkdir(parents=True, exist_ok=True)
    eep_path = str(eep_path)
    make_eeprom(addr, eep_path)

    firmware_path = Path(env.subst("$PROJECT_DIR")) / ".pio" / "build" / env.subst("$PIOENV") / "firmware.hex"
    if firmware_path.exists():
        print(f"Firmware found, will flash it as well!")
        flash = f'-U flash:w:{firmware_path}:i '
    else:
        flash = ''

    print(f"Generated {eep_path} with address {addr}")

    # Flash EEPROM
    env.AutodetectUploadPort()
    return env.Execute(env.VerboseAction(
        DEFAULT_COMMAND +
        f'{flash}-U eeprom:w:{eep_path}:i',
        f"Writing EEPROM with address {addr}"
    ))

def do_upload(source, target, env):
    """
    Upload the firmware and the default address to the device.
    """
    addr = int(env.GetProjectOption("board_default_address", "32"))

    # Create a HEX with just the address at byte 0
    eep_path = Path(env.subst("$PROJECT_DIR")) / ".pio" / "eeproms" / f"eeprom-address{addr}.hex"
    eep_path.parent.mkdir(parents=True, exist_ok=True)
    eep_path = str(eep_path)
    make_eeprom(addr, eep_path)

    firmware_path = Path(env.subst("$PROJECT_DIR")) / ".pio" / "build" / env.subst("$PIOENV") / "firmware.hex"
    if firmware_path.exists():
        print(f"Firmware found, will flash it as well!")
        flash = f'-U flash:w:{firmware_path}:i '
    else:
        print("No firmware found, only flashing EEPROM.")
        flash = ''

    print(f"Generated {eep_path} with address {addr}")

    # Flash EEPROM
    env.AutodetectUploadPort()
    return env.Execute(env.VerboseAction(
        DEFAULT_COMMAND + SET_FUSES_COMMAND +
        f'{flash}-U eeprom:w:{eep_path}:i',
        f"Writing EEPROM with address {addr}"
    ))

fuses_target = env.Alias("setfuses", None, do_setfuses)
eeprom_target = env.Alias("seteeprom", None, do_seteeprom)
upload_target = env.Alias("flash", None, do_upload)
AlwaysBuild(fuses_target)
AlwaysBuild(eeprom_target)
AlwaysBuild(upload_target)
