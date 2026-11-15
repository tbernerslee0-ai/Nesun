#pragma once

#include <cstdint>
#include <array>

namespace Nesun::Subor {

class Keyboard {
public:
    Keyboard();
    ~Keyboard() = default;

    void reset();

    // Matrix state updates from host frontend (SDL2)
    void set_key_state(uint8_t row, uint8_t col, bool pressed);
    void handle_sdl_key(uint32_t scancode, bool pressed);

    // CPU Bus Register Interfacing
    void write_scan_row(uint8_t row);
    uint8_t read_matrix_column() const;

private:
    uint8_t active_row_ = 0;
    // 8 rows x 8 columns keyboard matrix representation
    std::array<uint8_t, 8> matrix_rows_{};
};

} // namespace Nesun::Subor
