#!/usr/bin/python
# scons script for the Arduino sketch
import re

from arduino_build import ArduinoBuildContext
import hv_switching_board


Import('sketch_build_root')

context = ArduinoBuildContext(ARGUMENTS, build_root=sketch_build_root)
arduino_hex = context.build(extra_sources=hv_switching_board.get_sources(),
                            register_upload=True,
                            env_dict={'CPPPATH':
                                      hv_switching_board.get_includes()})

Export('arduino_hex')
