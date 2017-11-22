import logging
import time

from base_node.driver import BaseNode, CONFIG_DTYPE
from base_node_rpc.bootloader_driver import TwiBootloader
import numpy as np


logger = logging.getLogger(__name__)

CMD_SET_STATE_OF_ALL_CHANNELS = 0xA0
CMD_GET_STATE_OF_ALL_CHANNELS = 0xA1
CMD_REBOOT = 0xA2
CMD_RESET_CONFIG = 0xA3


class HVSwitchingBoard(BaseNode):
    def __init__(self, proxy, address, bootloader_address=0x29):
        '''
        Parameters
        ----------
        proxy : base_node_rpc.Proxy
        address : int
            I2C address of switching board.
        '''
        BaseNode.__init__(self, proxy, address)
        self.bootloader_address = bootloader_address
        self.bootloader = TwiBootloader(self.proxy, self.bootloader_address)

    def set_i2c_address(self, address):
        '''
        Set I2C address in EEPROM configuration.

        .. warning::
            This **reboots** the switching board.

        Parameters
        ----------
        address : int
            I2C address
        '''
        self.reboot_recovery()
        config = self.read_config()
        config['i2c_address'] = address
        self.write_config(config)
        self.address = address
        self.bootloader.start_application()

    def reset_config(self):
        '''
        Reset configuration from switching board EEPROM.

        .. warning::
            This resets the I2C address of the switching board to **10**.
        '''
        self.proxy.i2c_write(self.address, [CMD_RESET_CONFIG])
        self.address = 10

    def read_config(self):
        '''
        Read configuration from switching board EEPROM.

        Returns
        -------
        base_node.driver.CONFIG_DTYPE
            Switching board configuration as a `numpy` type.
        '''
        config_str = (self.bootloader.read_eeprom(0, CONFIG_DTYPE.itemsize)
                      .tostring())
        return np.fromstring(config_str, dtype=CONFIG_DTYPE)[0]

    def write_config(self, config):
        '''
        Write configuration switching board EEPROM.

        Parameters
        ----------
        config : base_node.driver.CONFIG_DTYPE
            Switching board configuration as a `numpy` type.
        '''
        self.bootloader.write_eeprom(0, map(ord, config.tobytes()))

    def reboot_recovery(self):
        self.proxy.i2c_write(self.address, CMD_REBOOT)

        for i in xrange(10 * 200):
            if self.bootloader_address in self.proxy.i2c_scan():
                logger.debug('Found device at %s', self.bootloader_address)
                self.bootloader.abort_boot_timeout()
                logger.debug('Aborted timeout to stay in bootloader')
                break
            time.sleep(1. / 200)

    def set_state_of_all_channels(self, state):
        data = np.array([0]*5, dtype=np.uint8)
        for i in range(len(state)):
            data[i / 8] |= (state[i] << i % 8)
        for i in range(5):
            self.serialize_uint8(~data[i])
        self.send_command(CMD_SET_STATE_OF_ALL_CHANNELS)

    def state_of_all_channels(self):
        self.data = []
        self.send_command(CMD_GET_STATE_OF_ALL_CHANNELS)
        state = np.zeros(40, dtype=np.uint8)
        for i in range(len(state)):
            state[i] = self.data[i / 8] & (0x01 << i % 8) == 0
        return state
