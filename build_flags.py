# coding: utf-8
import sys

from hv_switching_board import __version__ as VERSION

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
    # Use define build flags to set the following during build:
    #
    #  - Firmware software version (``___SOFTWARE_VERSION___``)
    #  - Hardware major version (``___HARDWARE_MAJOR_VERSION``)
    #  - Hardware minor version (``___HARDWARE_MINOR_VERSION``)
    #  - Serial baud rate (``HV_SWITCHING_BOARD_BAUD_RATE``)
    #
    # .. versionchanged:: 0.9
    #     Add serial baud rate (``HV_SWITCHING_BOARD_BAUD_RATE``).
    software_version = VERSION
    major_version = sys.argv[1]
    minor_version = sys.argv[2]

    create_config(software_version, major_version, minor_version)

    flags_ = [rf'-D___SOFTWARE_VERSION___=\"{software_version}\"',
              rf'-D___HARDWARE_MAJOR_VERSION___={major_version}',
              rf'-D___HARDWARE_MINOR_VERSION___={minor_version}',
              rf'-DHV_SWITCHING_BOARD_BAUD_RATE={sys.argv[3]}',
              rf'-DHV_SWITCHING_BOARD_I2C_RATE={sys.argv[4]}']

    return flags_


if __name__ == '__main__':
    flags = get_flags()

    print(' '.join(flags))
