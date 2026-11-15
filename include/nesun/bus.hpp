#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include "cpu.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "mappers/mapper.hpp"
#include "cheats/cheat_manager.hpp"

namespace Nesun {

class Bus {
public:
    Bus();
    ~Bus() = default;

    void reset();

    // CPU Bus Interfacing ($0000 - $FFFF)
    uint8_t cpu_read(uint16_t address, bool readonly = false);
    void cpu_write(uint16_t address, uint8_t value);

    // Cartridge Insertion
    void insert_cartridge(std::unique_ptr<Mapper> mapper);
    Mapper* get_mapper() { return mapper_.get(); }

    // Subsystem Accessors
    CPU& get_cpu() { return cpu_; }
    PPU& get_ppu() { return ppu_; }
    APU& get_apu() { return apu_; }
    Cheats::CheatManager& get_cheat_manager() { return cheat_manager_; }

private:
    CPU cpu_;
    PPU ppu_;
    APU apu_;
    std::unique_ptr<Mapper> mapper_;
    Cheats::CheatManager cheat_manager_;

    std::array<uint8_t, 2048> cpu_ram_{};

    std::array<uint8_t, 2> controller_state_{};
    std::array<uint8_t, 2> controller_shift_reg_{};
    bool controller_strobe_ = false;
};

} // namespace Nesun
