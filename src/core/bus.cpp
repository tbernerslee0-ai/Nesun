#include "nesun/bus.hpp"

namespace Nesun {

Bus::Bus() : cpu_(*this) {
    cpu_ram_.fill(0);
}

void Bus::reset() {
    cpu_ram_.fill(0);
    if (mapper_) {
        mapper_->reset();
    }
    // Note: Keep cheat_manager_ intact across soft resets so active codes persist
}

uint8_t Bus::cpu_read(uint16_t address, bool readonly) {
    uint8_t data = 0;

    // 1. Internal CPU RAM ($0000 - $1FFF, mirrored every 2KB)
    if (address <= 0x1FFF) {
        data = cpu_ram_[address & 0x07FF];
        // Also allow RAM freeze / Pro Action Replay patches on mirrored addresses
        cheat_manager_.apply_patches(address & 0x07FF, data);
        return data;
    }
    // 2. PPU Registers ($2000 - $3FFF, mirrored every 8 bytes)
    else if (address <= 0x3FFF) {
        return ppu_.cpu_read(address & 0x2007, readonly);
    }
    // 3. APU & Controller I/O ($4000 - $4017)
    else if (address <= 0x4017) {
        return apu_.cpu_read(address);
    }
    // 4. Cartridge Mapper Space ($4020 - $FFFF: Work RAM, Expansion, PRG-ROM)
    else if (address >= 0x4020) {
        if (mapper_) {
            mapper_->cpu_read(address, data);
        }

        // Apply Game Genie / Work RAM cheats after mapper populates original byte
        // so 8-character compare values can verify the active PRG bank
        cheat_manager_.apply_patches(address, data);
        return data;
    }

    return 0;
}

void Bus::cpu_write(uint16_t address, uint8_t value) {
    if (address <= 0x1FFF) {
        cpu_ram_[address & 0x07FF] = value;
    }
    else if (address <= 0x3FFF) {
        ppu_.cpu_write(address & 0x2007, value);
    }
    else if (address <= 0x401F) {
        apu_.cpu_write(address, value);
    }
    else if (address >= 0x4020) {
        if (mapper_) {
            mapper_->cpu_write(address, value);
        }
    }
}

void Bus::insert_cartridge(std::unique_ptr<Mapper> mapper) {
    mapper_ = std::move(mapper);
    cheat_manager_.clear(); // Clear cheats from previous ROM
    reset();
}

} // namespace Nesun
