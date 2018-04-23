from collections import OrderedDict
import os
import sys

from base_node_rpc.pavement_base import develop_link, develop_unlink
from paver.easy import task, needs, path, sh, options
from paver.setuputils import install_distutils_tasks
import base_node_rpc
import versioneer

# add the current directory as the first listing on the python path
# so that we import the correct version.py
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))
# Add package directory to Python path. This enables the use of
# `hv_switching_board` functions for discovering, e.g., the path to the Arduino
# firmware sketch source files.
sys.path.append(path('.').abspath())

install_distutils_tasks()


PROPERTIES = OrderedDict([('base_node_software_version',
                           base_node_rpc.__version__),
                          ('package_name', 'hv-switching-board'),
                          ('software_version', versioneer.get_version()),
                          ('url', 'https://github.com/wheeler-microfluidics/'
                           'hv-switching-board-firmware')])

options(
    PROPERTIES=PROPERTIES,
    setup=dict(name=PROPERTIES['package_name'],
               version=PROPERTIES['software_version'],
               cmdclass=versioneer.get_cmdclass(),
               description='Arduino-based high-voltage switching board '
               'firmware and Python API.',
               author='Ryan Fobel and Christian Fobel',
               author_email='ryan@fobel.net and christian@fobel.net',
               url=PROPERTIES['url'],
               license='MIT',
               packages=['hv_switching_board', 'hv_switching_board.bin'],
               install_requires=['wheeler.base-node>=0.4']))


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
