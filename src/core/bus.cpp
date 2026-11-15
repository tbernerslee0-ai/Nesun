#include "nesun/bus.hpp"
#include <iostream>

namespace Nesun {

Bus::Bus() {
    // Clear internal RAM ($0000 - $07FF, mirrored up to $1FFF)
    cpu_ram_.fill(0);
}

void Bus::reset() {
    cpu_ram_.fill(0);
    if (mapper_) {
        mapper_->reset();
    }
}

uint8_t Bus::cpu_read(uint16_t address, bool readonly) {
    uint8_t data = 0;

    // 1. Internal CPU RAM ($0000 - $1FFF, mirrored every 2KB)
    if (address >= 0x0000 && address <= 0x1FFF) {
        return cpu_ram_[address & 0x07FF];
    }
    // 2. PPU Registers ($2000 - $3FFF, mirrored every 8 bytes)
    else if (address >= 0x2000 && address <= 0x3FFF) {
        return ppu_.cpu_read(address & 0x2007, readonly);
    }
    // 3. APU & Controller I/O ($4000 - $401F)
    else if (address >= 0x4000 && address <= 0x4017) {
        return apu_.cpu_read(address);
    }
    // 4. Cartridge Mapper Space (PRG-ROM / Expansion / VTxx Registers)
    else if (address >= 0x4020 && address <= 0xFFFF) {
        if (mapper_) {
            mapper_->cpu_read(address, data);
        }
        return data;
    }

    return 0;
}

void Bus::cpu_write(uint16_t address, uint8_t value) {
    // 1. Internal CPU RAM ($0000 - $1FFF)
    if (address >= 0x0000 && address <= 0x1FFF) {
        cpu_ram_[address & 0x07FF] = value;
    }
    // 2. PPU Registers ($2000 - $3FFF)
    else if (address >= 0x2000 && address <= 0x3FFF) {
        ppu_.cpu_write(address & 0x2007, value);
    }
    // 3. APU & Controller I/O ($4000 - $401F)
    else if (address >= 0x4000 && address <= 0x401F) {
        apu_.cpu_write(address, value);
    }
    // 4. Cartridge Mapper Space ($4020 - $FFFF)
    else if (address >= 0x4020 && address <= 0xFFFF) {
        if (mapper_) {
            mapper_->cpu_write(address, value);
        }
    }
}

void Bus::insert_cartridge(std::unique_ptr<Mapper> mapper) {
    mapper_ = std::move(mapper);
    reset();
}

} // namespace Nesun
