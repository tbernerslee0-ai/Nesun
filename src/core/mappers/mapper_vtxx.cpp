#include "nesun/mappers/mapper_vtxx.hpp"
#include <iostream>
#include <algorithm>

namespace Nesun {

MapperVTxx::MapperVTxx(const std::vector<uint8_t>& prg_rom, 
                       const std::vector<uint8_t>& chr_rom, 
                       uint8_t vtxx_mode_flags)
    : Mapper(prg_rom.size() / 16384, chr_rom.size() / 8192),
      prg_rom_(prg_rom), chr_rom_(chr_rom),
      extended_ram_(8192, 0), // 8KB extended work/attribute RAM common on VTxx hardware
      mode_flags_(vtxx_mode_flags) {}

void MapperVTxx::reset() {
    prg_bank_select_ = 0;
    chr_bank_select_ = 0;
    std::fill(extended_ram_.begin(), extended_ram_.end(), 0);
}

bool MapperVTxx::cpu_read(uint16_t address, uint8_t& data) {
    // 1. Extended Work RAM ($6000 - $7FFF)
    if (address >= 0x6000 && address < 0x8000) {
        data = extended_ram_[address - 0x6000];
        return true;
    }
    // 2. VTxx Extended PRG Banking ($8000 - $FFFF)
    else if (address >= 0x8000) {
        size_t bank_size = 16384; // 16KB PRG banks
        size_t total_banks = prg_rom_.size() / bank_size;
        if (total_banks == 0) return false;

        size_t active_bank = prg_bank_select_ % total_banks;
        uint32_t mapped_addr = (active_bank * bank_size) + (address - 0x8000);
        
        if (mapped_addr < prg_rom_.size()) {
            data = prg_rom_[mapped_addr];
            return true;
        }
    }
    return false;
}

bool MapperVTxx::cpu_write(uint16_t address, uint8_t data) {
    // 1. Extended Work RAM ($6000 - $7FFF)
    if (address >= 0x6000 && address < 0x8000) {
        extended_ram_[address - 0x6000] = data;
        return true;
    }
    // 2. VTxx Banking Register Write ($8000 - $FFFF or custom ranges)
    else if (address >= 0x8000) {
        prg_bank_select_ = data & 0x1F;
        return true;
    }
    return false;
}

bool MapperVTxx::ppu_read(uint16_t address, uint8_t& data) {
    // VTxx expanded CHR banking ($0000 - $1FFF)
    if (address < 0x2000) {
        size_t bank_size = 8192; // 8KB CHR banks
        size_t total_banks = chr_rom_.size() / bank_size;
        if (total_banks == 0) {
            return false; // Fallback to CHR-RAM
        }

        size_t active_bank = chr_bank_select_ % total_banks;
        uint32_t mapped_addr = (active_bank * bank_size) + address;
        
        if (mapped_addr < chr_rom_.size()) {
            data = chr_rom_[mapped_addr];
            return true;
        }
    }
    return false;
}

bool MapperVTxx::ppu_write(uint16_t address, uint8_t data) {
    if (address < 0x2000 && chr_rom_.empty()) {
        // CHR-RAM write support
        return true;
    }
    (void)data;
    return false;
}

void MapperVTxx::write_vtxx_register(uint16_t address, uint8_t data) {
    // GeneralPlus VTxx hardware control registers (e.g., enhanced color depths & banking switches)
    if (address == 0x2010 || address == 0x4100) {
        prg_bank_select_ = data & 0x1F;
    } 
    else if (address == 0x2011 || address == 0x4101) {
        chr_bank_select_ = data & 0x1F;
    }
}

} // namespace Nesun
