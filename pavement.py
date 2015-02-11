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

setup(name='wheeler.hv-switching-board',
      version=version.getVersion(),
      description='Arduino-based high-voltage switching board firmware and '
      'Python API.',
      author='Ryan Fobel',
      author_email='ryan@fobel.net',
      url='http://microfluidics.utoronto.ca/git/firmware___hv_switching_board.git',
      license='GPLv2',
      packages=['hv_switching_board'],
      install_requires=['wheeler.base-node>=0.3'])


@task
def create_config():
    import hv_switching_board
    sketch_directory = path(hv_switching_board.get_sketch_directory())
    sketch_directory.joinpath('Config.h.skeleton').copy(sketch_directory
                                                        .joinpath('Config.h'))


@task
@needs('create_config')
@cmdopts([('sconsflags=', 'f', 'Flags to pass to SCons.')])
def build_firmware():
    sh('scons %s' % getattr(options, 'sconsflags', ''))


@task
@needs('generate_setup', 'minilib', 'build_firmware',
       'setuptools.command.sdist')
def sdist():
    """Overrides sdist to make sure that our setup.py is generated."""
    pass
