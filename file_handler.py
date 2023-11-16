# coding: utf-8
import argparse
import subprocess

import versioneer

import platformio_helpers as pioh

from typing import Dict
from importlib import import_module

from path_helpers import path
from base_node_rpc.helpers import generate_all_code

DEFAULT_ARDUINO_BOARDS = []
PLATFORMIO_ENVS = ['v2_1', 'v3_1']


def get_properties(**kwargs) -> Dict:
    version = versioneer.get_version()

    try:
        base_node_version = import_module('base_node_rpc').__version__
    except ImportError:
        base_node_version = None

    module_name = kwargs.get('module_name')
    package_name = kwargs.get('package_name')
    url = f'https://github.com/sci-bots/{package_name}'

    properties = dict(package_name=package_name,
                      display_name=package_name,
                      manufacturer='Wheeler Lab',
                      software_version=version,
                      base_node_software_version=base_node_version,
                      url=url
                      )

    meta = dict(short_description='Arduino-based high-voltage switching board firmware and Python API.',
                long_description='',
                author='Christian Fobel',
                author_email='christian@fobel.net',
                version=version,
                license='MIT',
                category='Communication',
                architectures='avr',
                )

    lib_properties = {**properties, **meta}

    options = dict(rpc_module=import_module(module_name) if module_name else None,
                   PROPERTIES=properties,
                   LIB_PROPERTIES=lib_properties,
                   )
    return {**kwargs, **options}


def transfer(**kwargs) -> None:
    # Copy Arduino library to Conda include directory
    source_dir = kwargs.get('source_dir')
    lib_name = kwargs.get('lib_name')
    # source_dir = path(source_dir).joinpath(module_name, 'Arduino', 'library', lib_name) # Use this for Arduino libs
    source_dir = path(source_dir).joinpath('lib', lib_name)
    install_dir = pioh.conda_arduino_include_path().joinpath(lib_name)
    source_dir.copytree(install_dir)
    print(f"Copied tree from '{source_dir}' to '{install_dir}'")


def copy_compiled_firmware(**kwargs) -> None:
    # Copy compiled firmware to Conda bin directory
    source_dir = kwargs.get('source_dir')
    package_name = kwargs.get('package_name')
    src_dir = path(source_dir)
    pio_bin_dir = pioh.conda_bin_path().joinpath(package_name)

    pio_bin_dir.makedirs(exist_ok=True)

    src_dir.joinpath('platformio.ini').copy2(pio_bin_dir.joinpath('platformio.ini'))

    for pio_platform in PLATFORMIO_ENVS:
        hex_dir = pio_bin_dir.joinpath(pio_platform)
        hex_dir.makedirs(exist_ok=True)
        src = src_dir.joinpath('.pio', 'build', pio_platform, 'firmware.hex')
        dest = hex_dir.joinpath('firmware.hex')
        src.copy2(dest)


def create_config(**kwargs) -> None:
    source_dir = kwargs.get('source_dir')
    lib_name = kwargs.get('lib_name')
    source_path = path(source_dir).joinpath('src', 'Config.h.skeleton')
    target_path = path(source_dir).joinpath('lib', lib_name, 'src', 'HVSwitchingBoard', 'Config.h')
    source_path.copy2(target_path)


def cli_parser():
    parser = argparse.ArgumentParser(description='Transfer header files to include directory.')
    parser.add_argument('source_dir')
    parser.add_argument('prefix')
    parser.add_argument('package_name')
    parser.add_argument('module_name')
    parser.add_argument('lib_name')

    args = parser.parse_args()
    execute(**vars(args))


def execute(**kwargs):
    top = '>' * 180
    print(top)

    create_config(**kwargs)
    transfer(**kwargs)
    try:
        subprocess.run(['pio', 'run', '--verbose'])
        copy_compiled_firmware(**kwargs)
    except FileNotFoundError:
        print('Failed to generate firmware')

    print('<' * len(top))


if __name__ == '__main__':
    cli_parser()
