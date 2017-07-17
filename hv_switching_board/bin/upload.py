from platformio_helpers.upload import upload_conda, parse_args


if __name__ == '__main__':
    from argparse import ArgumentParser

    parser = ArgumentParser(description='Upload firmware to board.')
    parser.add_argument('-p', '--port', default=None)
    args = parser.parse_args()
    extra_args = None if args.port is None else ['--port', args.port]
    print upload_conda('hv-switching-board', extra_args=extra_args)
