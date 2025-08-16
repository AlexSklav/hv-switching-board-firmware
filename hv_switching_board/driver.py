# coding: utf-8
import time
import logging

import numpy as np

from typing import Optional, List, Union

from base_node_rpc import proxy as Proxy
from base_node.driver import BaseNode, CONFIG_DTYPE
from base_node_rpc.bootloader_driver import TwiBootloader

logger = logging.getLogger(__name__)

CMD_SET_STATE_OF_ALL_CHANNELS = 0xA0
CMD_GET_STATE_OF_ALL_CHANNELS = 0xA1
CMD_REBOOT = 0xA2
CMD_RESET_CONFIG = 0xA3
CMD_GET_SHIFT_REGISTER_COUNT = 0xA6


class HVSwitchingBoard(BaseNode):
    def __init__(self, proxy: Proxy, address: int,
                 bootloader_address: Optional[int] = 0x29,
                 shift_register_count: int = 5):
        """
        Parameters
        ----------
        proxy : base_node_rpc.Proxy
        address : int
            I2C address of switching board.
        bootloader_address : int, optional
            I2C address of bootloader (default: 0x29).
        shift_register_count : int, optional
            Number of shift registers on the board (default: 5).
        """
        BaseNode.__init__(self, proxy, address)
        self.bootloader_address = bootloader_address
        self.bootloader = TwiBootloader(self.proxy, self.bootloader_address)
        self.shift_register_count = shift_register_count

    def set_i2c_address(self, address: int) -> None:
        """
        Set I2C address in EEPROM configuration.

        .. warning::
            This **reboots** the switching board.

        Parameters
        ----------
        address : int
            I2C address
        """
        self.reboot_recovery()
        config = self.read_config()
        config['i2c_address'] = address
        self.write_config(config)
        self.address = address
        self.bootloader.start_application()

    def reset_config(self) -> None:
        """
        Reset configuration from switching board EEPROM.

        .. warning::
            This resets the I2C address of the switching board to **10**.

        .. versionadded:: 0.10
        """
        self.proxy.i2c_write(self.address, [CMD_RESET_CONFIG])
        self.address = 10

    def get_shift_register_count(self) -> int:
        """
        Query the number of shift registers supported by this board.

        Returns
        -------
        int
            Number of shift registers available on this board.

        .. versionadded:: 4.1
        """
        response = self.proxy.i2c_write(self.address, [CMD_GET_SHIFT_REGISTER_COUNT])
        if response and len(response) > 0:
            return response[0]
        return 5  # Default fallback for older firmware

    def read_config(self) -> CONFIG_DTYPE:
        """
        Read configuration from switching board EEPROM.

        Returns
        -------
        base_node.driver.CONFIG_DTYPE
            Switching board configuration as a `numpy` type.
        """
        config_str = self.bootloader.read_eeprom(0, CONFIG_DTYPE.itemsize)
        return np.frombuffer(config_str, dtype=CONFIG_DTYPE)[0]

    def write_config(self, config: CONFIG_DTYPE) -> None:
        """
        Write configuration switching board EEPROM.

        Parameters
        ----------
        config : base_node.driver.CONFIG_DTYPE
            Switching board configuration as a `numpy` type.
        """
        self.bootloader.write_eeprom(0, map(ord, config.tobytes()))

    def reboot_recovery(self) -> None:
        self.proxy.i2c_write(self.address, CMD_REBOOT)

        for i in range(10 * 200):
            if self.bootloader_address in self.proxy.i2c_scan():
                logger.debug(f'Found device at {self.bootloader_address}')
                self.bootloader.abort_boot_timeout()
                logger.debug('Aborted timeout to stay in bootloader')
                break
            time.sleep(1. / 200)

    def set_state_of_all_channels(self, state: Union[List, np.array]) -> None:
        data = np.array([0] * self.shift_register_count, dtype=np.uint8)
        for i in range(len(state)):
            data[i // 8] |= (state[i] << (i % 8))
        for i in range(self.shift_register_count):
            self.serialize_uint8(~data[i])
        self.send_command(CMD_SET_STATE_OF_ALL_CHANNELS)

    def state_of_all_channels(self) -> np.array:
        self.data = []
        self.send_command(CMD_GET_STATE_OF_ALL_CHANNELS)
        total_channels = self.shift_register_count * 8
        state = np.zeros(total_channels, dtype=np.uint8)
        for i in range(len(state)):
            state[i] = self.data[i // 8] & (0x01 << (i % 8)) == 0
        return state
