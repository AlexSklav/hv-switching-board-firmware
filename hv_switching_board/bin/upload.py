from argparse import ArgumentParser
from platformio_helpers.upload import upload_conda
import platformio_helpers as pioh


if __name__ == '__main__':
    environments = sorted([dir_i.name for dir_i in pioh.conda_bin_path().joinpath('hv-switching-board').dirs()])

    parser = ArgumentParser(description='Upload firmware to board.')
    parser.add_argument('-p', '--port', default=None)
    parser.add_argument('-b', '--hardware-version', default=environments[-1], choices=environments)
    args = parser.parse_args()
    extra_args = None if args.port is None else ['--port', args.port]
    print(upload_conda('hv-switching-board', env_name=args.hardware_version, extra_args=extra_args))
