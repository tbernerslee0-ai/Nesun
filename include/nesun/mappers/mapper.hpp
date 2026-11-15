#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace Nesun {

enum class Mirroring {
    Horizontal,
    Vertical,
    FourScreen,
    OneScreenLower,
    OneScreenUpper
};

class Mapper {
public:
    Mapper(uint32_t prg_rom_banks, uint32_t chr_rom_banks)
        : prg_bank_count_(prg_rom_banks), chr_bank_count_(chr_rom_banks) {}
    
    virtual ~Mapper() = default;

    // Core CPU & PPU Bus Mapping Interceptors
    virtual bool cpu_read(uint16_t address, uint8_t& data) = 0;
    virtual bool cpu_write(uint16_t address, uint8_t data) = 0;
    virtual bool ppu_read(uint16_t address, uint8_t& data) = 0;
    virtual bool ppu_write(uint16_t address, uint8_t data) = 0;

    // Reset state hook
    virtual void reset() = 0;

    // Nametable mirroring control
    virtual Mirroring get_mirroring() const { return mirroring_; }

    // Famiclone & System-Specific Extension Hooks
    virtual void write_vtxx_register(uint16_t address, uint8_t data) {}
    virtual void write_subor_register(uint16_t address, uint8_t data) {}

protected:
    uint32_t prg_bank_count_;
    uint32_t chr_bank_count_;
    Mirroring mirroring_ = Mirroring::Horizontal;
};

} // namespace Nesun
