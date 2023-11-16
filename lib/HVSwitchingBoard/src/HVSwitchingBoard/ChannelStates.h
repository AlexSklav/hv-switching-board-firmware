#ifndef ___CHANNEL_STATES__H___
#define ___CHANNEL_STATES__H___

#ifndef bitRead
#define bitRead(b, i)  ((b >> i) & 0x0001)
#endif
#ifndef bitWrite
#define bitWrite(b, i, new_i)  ((new_i) ? (b |= (0x01 << i)) : (b &= ~(0x01 << i)))
#endif


template <uint8_t NSwitchPorts>
struct WindowChannelStates {
  static const uint8_t N_SWITCH_PORTS = NSwitchPorts;
  static const uint8_t N_CHANNELS = 8 * N_SWITCH_PORTS;
  static const uint8_t M_MEASURE_OFF = (1 << 0);  // Turn off to measure (subtractive).
  static const uint8_t M_ACTUATE_OFF = (1 << 1);  // Turn off outside measuring region.
  uint8_t channel_states_[N_SWITCH_PORTS];
  uint8_t window_channel_states_[N_SWITCH_PORTS];
  int8_t active_index_[N_CHANNELS];
  uint8_t period_;
  uint8_t mode_;
  uint8_t active_count_;

  WindowChannelStates() : period_(0), mode_(0), active_count_(0) {
    reset();
  }

  WindowChannelStates(uint8_t mode) : period_(0), mode_(mode),
                                      active_count_(0) {
    reset();
  }

  WindowChannelStates(uint8_t period, uint8_t mode) : period_(period),
                                                      mode_(mode),
                                                      active_count_(0) {
    reset();
  }

  void reset() {
    reset_channel_states();
    reset_window_states();
  }

  void reset_channel_states() {
    memset(channel_states_, 0, N_SWITCH_PORTS);
    sync_active();
  }

  void reset_window_states() {
    memset(window_channel_states_, 0, N_SWITCH_PORTS);
  }

  void sync_active() {
    /* Assign a unique index to each actuated electrode. */
    active_count_ = 0;

    for (int i = 0; i < N_SWITCH_PORTS; i++) {
      for (int j = 7; j >= 0; j--) {
        bool bit_state = bitRead(channel_states_[i], j);
        if (bit_state) {
          active_index_[i * 8 + j] = active_count_;
          active_count_ += 1;
        } else {
          active_index_[i * 8 + j] = -1;
        }
      }
    }
    // Set period explicitly, to trigger validation/correction if necessary.
    set_period(period_);
  }

  void set_mode(uint8_t mode) {
    mode_ = mode;
    // Set period explicitly, to trigger validation/correction if necessary.
    set_period(period_);
  }

  uint8_t set_period(uint8_t period) {
    const bool subtractive = mode_ & M_MEASURE_OFF;
    const uint8_t min_period = active_count_ + subtractive;

    period_ = (period < min_period) ? min_period : period;
    return period_;
  }

  void set_ports(uint8_t const *states) {
    memcpy(channel_states_, states, N_SWITCH_PORTS);
    sync_active();
  }

  void set_port(uint8_t port_i, uint8_t states, bool sync=true) {
    channel_states_[port_i] = states;
    if (sync) { sync_active(); }
  }

  void get_window_ports(uint8_t *window_states_) {
    memcpy(window_states_, window_channel_states_, N_SWITCH_PORTS);
  }

  void select_window_index(const uint8_t window_i) {
    const bool subtractive = mode_ & M_MEASURE_OFF;
    const bool actuate = !(mode_ & M_ACTUATE_OFF);

    /* Set state of each channel for window with index `window_i`. */
    const int8_t active_offset = period_ - active_count_;

    if (active_count_ > 1) {
      if (static_cast<int8_t>(window_i) >= active_offset) {
        /* The window `window_i` is in the measurement region of the period_. */
        for (int i = 0; i < N_SWITCH_PORTS; i++) {
          for (int j = 0; j < 8; j++) {
            if (window_i == active_offset + active_index_[i * 8 + j]) {
              bitWrite(window_channel_states_[i], j, !subtractive);
            } else {
              const bool bit_state = bitRead(channel_states_[i], j);
              bitWrite(window_channel_states_[i], j, subtractive & bit_state);
            }
          }
        }
        return;
      } else if (!actuate && (window_i < active_offset - subtractive)) {
        /* Do not actuate channel when not measuring. */
        memset(window_channel_states_, 0, N_SWITCH_PORTS);
        return;
      }
    }

    for (int i = 0; i < N_SWITCH_PORTS; i++) {
      /* Apply base actuation state when not measuring. */
      memcpy(window_channel_states_, channel_states_, N_SWITCH_PORTS);
    }
  }
};

#endif  // #ifndef ___CHANNEL_STATES__H___
