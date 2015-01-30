from base_node import BaseNode


CMD_SET_STATE_OF_ALL_CHANNELS = 0xA0
CMD_GET_STATE_OF_ALL_CHANNELS = 0xA1


class HVSwitchingBoard(BaseNode):
    def __init__(self, proxy, address):
        BaseNode.__init__(self, proxy, address)

    def set_state_of_all_channels(self, state):
        for i in range(5):
          self.serialize_uint8(state[i])
        self.send_command(CMD_SET_STATE_OF_ALL_CHANNELS)

    def state_of_all_channels(self):
        self.data = []
        self.send_command(CMD_GET_STATE_OF_ALL_CHANNELS)
        return self.data
