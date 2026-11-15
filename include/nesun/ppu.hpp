#pragma once

#include <cstdint>
#include <array>

namespace Nesun {

class Bus;

enum class PPUHardwareMode {
    StandardNES, // RP2C02 (2bpp tiles, 64 sprites, 8 per scanline)
    VTxx,        // GeneralPlus VT02/VT03 (4bpp tiles, extended palette RAM)
    VTxxx        // GeneralPlus VT09/VT369 (Direct RGB565, high sprite limits)
};

class PPU {
public:
    explicit PPU(Bus& bus);
    ~PPU() = default;

    // Lifecycle & Execution
    void reset();
    void step(); // Advances 1 PPU dot (3 PPU dots per 1 CPU cycle)

    // CPU-Mapped Register Interface ($2000 - $2007 + VTxx Extensions)
    uint8_t cpu_read(uint16_t address, bool readonly = false);
    void cpu_write(uint16_t address, uint8_t value);

    // Hardware Mode Configuration
    void set_hardware_mode(PPUHardwareMode mode) { mode_ = mode; }
    PPUHardwareMode get_hardware_mode() const { return mode_; }

    // Framebuffer & Timing Accessors
    static constexpr int SCREEN_WIDTH = 256;
    static constexpr int SCREEN_HEIGHT = 240;

    const uint32_t* get_framebuffer() const { return framebuffer_.data(); }
    bool is_frame_complete() const { return frame_complete_; }
    void clear_frame_complete() { frame_complete_ = false; }

    // Interrupt Signals
    bool nmi_requested = false;

private:
    Bus& bus_;
    PPUHardwareMode mode_ = PPUHardwareMode::StandardNES;

    // Scanline & Dot Counters
    int16_t scanline_ = 0;
    int16_t cycle_ = 0;
    bool frame_complete_ = false;
    bool odd_frame_ = false;

    // Standard RP2C02 Registers
    uint8_t ctrl_   = 0x00; // $2000 PPUCTRL
    uint8_t mask_   = 0x00; // $2001 PPUMASK
    uint8_t status_ = 0x00; // $2002 PPUSTATUS
    uint8_t oam_addr_ = 0x00; // $2003 OAMADDR

    // Internal Loopy Scroll Registers
    uint16_t vram_addr_ = 0x0000; // Current VRAM address (15-bit)
    uint16_t temp_addr_ = 0x0000; // Temporary VRAM address (15-bit)
    uint8_t  fine_x_    = 0x00;   // Fine X scroll (3-bit)
    bool     addr_latch_ = false; // First/second write toggle
    uint8_t  data_buffer_ = 0x00; // PPUDATA read buffer

    // Memory Banks
    std::array<uint8_t, 2048> vram_{};           // 2KB Internal Nametable RAM
    std::array<uint8_t, 256>  oam_{};            // Primary Sprite OAM
    std::array<uint8_t, 512>  palette_ram_{};    // Expanded to 512B for VTxx 4bpp palettes
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer_{};

    // VTxx / VTxxx Extension Registers ($2010 - $201F)
    std::array<uint8_t, 16> vtxx_ext_regs_{};

    // Internal Rendering Helpers
    uint8_t ppu_read(uint16_t address);
    void ppu_write(uint16_t address, uint8_t value);
    void render_pixel();
    uint32_t lookup_nes_color(uint8_t palette_index, uint8_t pixel_value) const;
    uint32_t lookup_vtxx_color(uint16_t extended_index) const;
};

} // namespace Nesun
