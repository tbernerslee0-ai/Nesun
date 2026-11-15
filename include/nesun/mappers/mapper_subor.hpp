#pragma once

#include "mapper.hpp"
#include <vector>
#include <array>
#include <string>

namespace Nesun {

class MapperSuborSB2000 : public Mapper {
public:
    MapperSuborSB2000(const std::vector<uint8_t>& prg_rom, 
                      const std::vector<uint8_t>& chr_rom);

    bool cpu_read(uint16_t address, uint8_t& data) override;
    bool cpu_write(uint16_t address, uint8_t data) override;
    bool ppu_read(uint16_t address, uint8_t& data) override;
    bool ppu_write(uint16_t address, uint8_t data) override;
    
    void reset() override;

    // Subor Specific Educational Hardware Hooks
    void write_subor_register(uint16_t address, uint8_t data) override;

    // Keyboard & Mouse Input State
    void set_key_state(uint8_t row, uint8_t col, bool pressed);
    void set_mouse_position(int16_t x, int16_t y, bool left_click, bool right_click);

    // Virtual Floppy Disk (.img) Mount Support
    bool mount_floppy_disk(const std::string& image_path);

private:
    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> chr_rom_;
    std::vector<uint8_t> extended_work_ram_; // 32KB - 64KB expanded RAM for SB-OS / SB-Win98
    std::vector<uint8_t> floppy_disk_data_;   // Virtual disk sector buffer

    // Bank registers
    uint32_t prg_bank_select_ = 0;
    uint32_t chr_bank_select_ = 0;

    // Subor Input & FDC State Registers
    std::array<uint8_t, 16> keyboard_matrix_{};
    uint8_t keyboard_scan_row_ = 0;
    
    struct MouseState {
        int16_t x = 0;
        int16_t y = 0;
        bool left_button = false;
        bool right_button = false;
    } mouse_;

    struct FloppyController {
        uint8_t status_reg = 0x80; // Ready flag
        uint8_t track_reg = 0;
        uint8_t sector_reg = 0;
        uint32_t data_offset = 0;
    } fdc_;
};

} // namespace Nesun
