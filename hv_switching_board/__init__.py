# coding: utf-8
from typing import List, Dict

from path_helpers import path

from .driver import HVSwitchingBoard

from ._version import get_versions

__version__ = get_versions()['version']
del get_versions


def package_path() -> path:
    return path(__file__).parent


def get_sketch_directory() -> path:
    """
    Return directory containing the `hv_switching_board` Arduino sketch.
    """
    return package_path().joinpath('..', 'src').realpath()


def get_lib_directory() -> path:
    """
    Return directory containing the `HVSwitchingBoard` Arduino library.
    """
    return package_path().joinpath('..', 'lib', 'HVSwitchingBoard').realpath()


def get_includes() -> List[path]:
    """
    Return directories containing the `hv_switching_board` Arduino header
    files.

    Modules that need to compile against `hv_switching_board` should use this
    function to locate the appropriate include directories.

    Notes
    =====

    For example:

        import hv_switching_board
        ...
        print ' '.join(['-I%s' % i for i in hv_switching_board.get_includes()])
        ...

    """
    import base_node
    return [get_sketch_directory()] + base_node.get_includes()


def get_sources() -> List[path]:
    """
    Return `hv_switching_board` Arduino source file paths.

    Modules that need to compile against `hv_switching_board` should use this
    function to locate the appropriate source files to compile.

    Notes
    =====

    For example:

        import hv_switching_board
        ...
        print ' '.join(hv_switching_board.get_sources())
        ...

    """
    sources = []
    for p in get_includes():
        sources += path(p).files('*.c*')
    return sources


def get_firmwares() -> Dict:
    """
    Return `hv_switching_board` compiled Arduino hex file paths.

    This function may be used to locate firmware binaries that are available
    for flashing to [Arduino Uno][1] boards.

    [1]: http://arduino.cc/en/Main/arduinoBoardUno
    """
    return {board_dir.name: [f.abspath() for f in board_dir.walkfiles('*.hex')]
            for board_dir in package_path().joinpath('firmware').dirs()}
