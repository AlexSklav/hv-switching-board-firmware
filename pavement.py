import os
import sys
from pprint import pprint

from paver.easy import task, needs, path, sh, cmdopts, options
from paver.setuputils import setup, find_package_data

# add the current directory as the first listing on the python path
# so that we import the correct version.py
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))
import version
# Add package directory to Python path. This enables the use of
# `hv_switching_board` functions for discovering, e.g., the path to the Arduino
# firmware sketch source files.
sys.path.append(path('.').abspath())

setup(name='hv-switching-board',
      version=version.getVersion(),
      description='Arduino-based high-voltage switching board firmware and '
                  'Python API.',
      author='Ryan Fobel and Christian Fobel',
      author_email='ryan@fobel.net and christian@fobel.net',
      url='https://github.com/wheeler-microfluidics/hv-switching-board-firmware',
      license='MIT',
      packages=['hv_switching_board'],
      install_requires=['wheeler.base-node>=0.4'])


@task
def create_config():
    import hv_switching_board
    lib_directory = path(hv_switching_board.get_lib_directory())
    sketch_directory = path(hv_switching_board.get_sketch_directory())
    (sketch_directory.joinpath('Config.h.skeleton')
     .copy(lib_directory.joinpath('src', 'HVSwitchingBoard', 'Config.h')))


@task
@needs('create_config')
def build_firmware():
    sh('pio run')


@task
def upload():
    sh('pio run --target upload --target nobuild')


@task
@needs('generate_setup', 'minilib', 'build_firmware',
       'setuptools.command.sdist')
def sdist():
    """Overrides sdist to make sure that our setup.py is generated."""
    pass
