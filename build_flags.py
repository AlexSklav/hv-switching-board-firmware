# coding: utf-8
import sys
import versioneer

import platformio_helpers as pioh

from path_helpers import path


def create_config(software_version: str, major_version: str = '2', minor_version: str = '1') -> None:
    config_template = f"""
#define STR_VALUE(arg) #arg
#define DEFINE_TO_STRING(name) STR_VALUE(name)

#ifndef ___HARDWARE_MAJOR_VERSION___
    #define ___HARDWARE_MAJOR_VERSION___ {major_version}
#endif
#ifndef ___HARDWARE_MINOR_VERSION___
    #define ___HARDWARE_MINOR_VERSION___ {minor_version}
#endif
#ifndef ___HARDWARE_VERSION___
    #define ___HARDWARE_VERSION___ DEFINE_TO_STRING(___HARDWARE_MAJOR_VERSION___.___HARDWARE_MINOR_VERSION___)
#endif
#ifndef ___SOFTWARE_VERSION___
    #define ___SOFTWARE_VERSION___ "{software_version}"
#endif
    """.strip()
    lib_directory = path(__file__).parent.joinpath('lib', 'HVSwitchingBoard')
    lib_directory.joinpath('src', 'HVSwitchingBoard', 'Config.h').write_text(config_template)


def get_flags() -> list:
    # Store the version of the firmware in a variable.
    software_version = versioneer.get_version()

    # Use define build flags to set the following during build:
    #
    #  - Firmware software version (``___SOFTWARE_VERSION___``)
    #  - Hardware major version (``___HARDWARE_MAJOR_VERSION``)
    #  - Hardware minor version (``___HARDWARE_MINOR_VERSION``)
    #  - Serial baud rate (``HV_SWITCHING_BOARD_BAUD_RATE``)
    #
    # .. versionchanged:: 0.9
    #     Add serial baud rate (``HV_SWITCHING_BOARD_BAUD_RATE``).

    major_version = sys.argv[1]
    minor_version = sys.argv[2]

    if 'dirty' in software_version:
        software_version = 0.2

    create_config(software_version, major_version, minor_version)

    flags_ = [f'-D___SOFTWARE_VERSION___={software_version}',
              f'-D___HARDWARE_MAJOR_VERSION___={major_version}',
              f'-D___HARDWARE_MINOR_VERSION___={minor_version}',
              f'-DHV_SWITCHING_BOARD_BAUD_RATE={sys.argv[3]}',
              f'-DHV_SWITCHING_BOARD_I2C_RATE={sys.argv[4]}']

    return flags_


if __name__ == '__main__':
    flags = get_flags()
    libs = [f'-I{pioh.conda_arduino_include_path().joinpath("BaseNode")}',
            f'-I{pioh.conda_arduino_include_path().joinpath("Wire")}']

    print(' '.join(flags + libs))
