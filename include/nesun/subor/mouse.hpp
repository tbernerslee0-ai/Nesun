#pragma once

#include <cstdint>

namespace Nesun::Subor {

class Mouse {
public:
    Mouse();
    ~Mouse() = default;

    void reset();

    // Position & Click updates
    void move_relative(int dx, int dy);
    void set_absolute_position(int x, int y);
    void set_button_state(bool left, bool right);

    // CPU Bus Register Interfacing ($4016 / custom Subor mouse ports)
    uint8_t read_data_register();
    void write_control_register(uint8_t value);

private:
    int16_t x_pos_ = 0;
    int16_t y_pos_ = 0;
    int16_t last_dx_ = 0;
    int16_t last_dy_ = 0;
    bool left_button_pressed_ = false;
    bool right_button_pressed_ = false;
    uint8_t packet_index_ = 0;
};

} // namespace Nesun::Subor
