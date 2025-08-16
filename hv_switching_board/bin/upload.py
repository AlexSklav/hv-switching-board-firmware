from argparse import ArgumentParser
from platformio_helpers.upload import upload_conda
import platformio_helpers as pioh


if __name__ == '__main__':
    environments = sorted([dir_i.name for dir_i in pioh.conda_bin_path()
                           .joinpath('hv-switching-board').dirs()])
    parser = ArgumentParser(description='Upload firmware to board.')
    parser.add_argument('-p', '--port', default=None)
    parser.add_argument('-b', '--hardware-version', default='v3_1', choices=environments)
    parser.add_argument('-e', '--eeprom', action='store_true',
                        help='If set, program EEPROM instead of flashing firmware.')
    args = parser.parse_args()

    # Pick the PlatformIO target based on the flag
    target = 'seteeprom' if args.eeprom else 'flash'

    # Build extra args for upload_conda / platformio
    extra_args = []
    if args.port:
        extra_args += ['--port', args.port]

    extra_args += ['-t', target]

    upload_conda('hv-switching-board',
                 env_name=args.hardware_version,
                 extra_args=extra_args, spi=True)
