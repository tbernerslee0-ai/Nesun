#pragma once

#include "mapper.hpp"
#include <vector>

namespace Nesun {

class MapperWiii3 : public Mapper {
public:
    MapperWiii3(const std::vector<uint8_t>& prg_rom, 
                const std::vector<uint8_t>& chr_rom);

    bool cpu_read(uint16_t address, uint8_t& data) override;
    bool cpu_write(uint16_t address, uint8_t data) override;
    bool ppu_read(uint16_t address, uint8_t& data) override;
    bool ppu_write(uint16_t address, uint8_t& data) override;
    
    void reset() override;

private:
    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> chr_rom_;
    std::vector<uint8_t> nvram_; // Integrated flash/save memory for built-in menus

    // Wiii3 Banking & Control Registers
    uint8_t control_register_ = 0;
    uint32_t prg_bank_mask_ = 0;
    uint32_t active_prg_bank_ = 0;
    uint32_t active_chr_bank_ = 0;

    // Internal helper for address remapping
    uint32_t map_prg_address(uint16_t address) const;
    uint32_t map_chr_address(uint16_t address) const;
};

} // namespace Nesun
