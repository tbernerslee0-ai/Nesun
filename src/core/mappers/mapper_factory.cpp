#include "nesun/mappers/mapper_factory.hpp"
#include "nesun/mappers/mapper_vtxx.hpp"
#include "nesun/mappers/mapper_subor.hpp"
#include "nesun/mappers/mapper_wiii3.hpp"
#include <iostream>

namespace Nesun {

// Simple NROM (Mapper 0) baseline implementation for standard cartridges
class MapperNROM : public Mapper {
public:
    MapperNROM(const std::vector<uint8_t>& prg_rom, const std::vector<uint8_t>& chr_rom)
        : Mapper(prg_rom.size() / 16384, chr_rom.size() / 8192),
          prg_rom_(prg_rom), chr_rom_(chr_rom) {}

    bool cpu_read(uint16_t address, uint8_t& data) override {
        if (address >= 0x8000) {
            // Support 16KB PRG mirroring if only 1 bank is present
            uint16_t mapped_addr = (address - 0x8000) % prg_rom_.size();
            data = prg_rom_[mapped_addr];
            return true;
        }
        return false;
    }

    bool cpu_write(uint16_t address, uint8_t data) override {
        (void)address; (void)data;
        return false; // NROM has no CPU write registers
    }

    bool ppu_read(uint16_t address, uint8_t& data) override {
        if (address < 0x2000 && !chr_rom_.empty()) {
            data = chr_rom_[address % chr_rom_.size()];
            return true;
        }
        return false;
    }

    bool ppu_write(uint16_t address, uint8_t data) override {
        if (address < 0x2000 && chr_rom_.empty()) {
            // CHR-RAM write support
            // chr_ram_[address] = data;
            return true;
        }
        (void)data;
        return false;
    }

    void reset() override {}

private:
    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> chr_rom_;
};

std::unique_ptr<Mapper> MapperFactory::create_mapper(
    uint8_t mapper_id, 
    const std::vector<uint8_t>& prg_rom, 
    const std::vector<uint8_t>& chr_rom,
    bool is_vtxx_system
) {
    // 1. Force VTxx Handler if system flag is set
    if (is_vtxx_system || (mapper_id >= 200 && mapper_id <= 210)) {
        std::cout << "MapperFactory: Instantiating MapperVTxx for Famiclone hardware.\n";
        return std::make_unique<MapperVTxx>(prg_rom, chr_rom, 0x01);
    }

    // 2. Route Subor Educational Systems (e.g., SB-2000 / Mapper 256 or 518)
    if (mapper_id == 256 || mapper_id == 518) {
        std::cout << "MapperFactory: Instantiating MapperSuborSB2000 for educational computer.\n";
        return std::make_unique<MapperSuborSB2000>(prg_rom, chr_rom);
    }

    // 3. Route Wiii3 Plug-and-Play Systems
    if (mapper_id == 220) { // Example Wiii3 mapper ID identifier
        std::cout << "MapperFactory: Instantiating MapperWiii3 plug-and-play system.\n";
        return std::make_unique<MapperWiii3>(prg_rom, chr_rom);
    }

    // 4. Standard NES Mappers
    switch (mapper_id) {
        case 0: // NROM
            std::cout << "MapperFactory: Instantiating standard NROM (Mapper 0).\n";
            return std::make_unique<MapperNROM>(prg_rom, chr_rom);
            
        default:
            std::cerr << "MapperFactory Warning: Mapper ID " << static_init_cast<int>(mapper_id) 
                      << " not fully implemented. Falling back to NROM.\n";
            return std::make_unique<MapperNROM>(prg_rom, chr_rom);
    }
}

} // namespace Nesun
