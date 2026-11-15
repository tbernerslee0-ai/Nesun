#include "nesun/mappers/mapper_subor.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace Nesun {

MapperSuborSB2000::MapperSuborSB2000(const std::vector<uint8_t>& prg_rom, 
                                     const std::vector<uint8_t>& chr_rom)
    : Mapper(prg_rom.size() / 16384, chr_rom.size() / 8192),
      prg_rom_(prg_rom), chr_rom_(chr_rom),
      extended_work_ram_(64 * 1024, 0), // 64KB expanded work RAM for SB-OS / SB-Win98
      floppy_disk_data_() {
    keyboard_matrix_.fill(0xFF); // Unpressed state (active low typical for matrix)
}

void MapperSuborSB2000::reset() {
    prg_bank_select_ = 0;
    chr_bank_select_ = 0;
    keyboard_scan_row_ = 0;
    fdc_ = FloppyController{};
    std::fill(extended_work_ram_.begin(), extended_work_ram_.end(), 0);
}

bool MapperSuborSB2000::cpu_read(uint16_t address, uint8_t& data) {
    // 1. Extended Work RAM ($6000 - $7FFF)
    if (address >= 0x6000 && address < 0x8000) {
        data = extended_work_ram_[address - 0x6000];
        return true;
    }
    // 2. Subor Hardware I/O Registers ($4020 - $40FF or custom ranges)
    // Keyboard Matrix Read Port ($4028 example)
    else if (address == 0x4028) {
        if (keyboard_scan_row_ < 8) {
            data = keyboard_matrix_[keyboard_scan_row_];
        } else {
            data = 0xFF;
        }
        return true;
    }
    // Floppy Disk Controller (FDC) Status / Data Read ($4030 - $4033)
    else if (address == 0x4030) {
        data = fdc_.status_reg;
        return true;
    }
    else if (address == 0x4031) {
        // Read sector data byte from buffer
        if (fdc_.data_offset < floppy_disk_data_.size()) {
            data = floppy_disk_data_[fdc_.data_offset++];
        } else {
            data = 0x00;
        }
        return true;
    }
    // 3. PRG-ROM Banked Area ($8000 - $FFFF)
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

bool MapperSuborSB2000::cpu_write(uint16_t address, uint8_t data) {
    // 1. Extended Work RAM ($6000 - $7FFF)
    if (address >= 0x6000 && address < 0x8000) {
        extended_work_ram_[address - 0x6000] = data;
        return true;
    }
    // 2. Subor Keyboard Scan Row Selection ($4027)
    else if (address == 0x4027) {
        keyboard_scan_row_ = data & 0x07; // Select row 0-7
        return true;
    }
    // 3. Floppy Disk Controller Commands ($4030 - $4033)
    else if (address == 0x4030) {
        // FDC Command Register (e.g. Seek Track / Read Sector)
        fdc_.track_reg = data;
        return true;
    }
    else if (address == 0x4031) {
        fdc_.sector_reg = data;
        // Calculate buffer offset based on track and sector (Standard 512-byte sectors)
        fdc_.data_offset = (fdc_.track_reg * 18 + (fdc_.sector_reg - 1)) * 512;
        return true;
    }
    // 4. PRG Bank Switching Register ($8000 - $FFFF)
    else if (address >= 0x8000) {
        prg_bank_select_ = data & 0x3F;
        return true;
    }
    return false;
}

bool MapperSuborSB2000::ppu_read(uint16_t address, uint8_t& data) {
    if (address < 0x2000 && !chr_rom_.empty()) {
        size_t bank_size = 8192;
        size_t total_banks = chr_rom_.size() / bank_size;
        if (total_banks == 0) return false;

        size_t active_bank = chr_bank_select_ % total_banks;
        uint32_t mapped_addr = (active_bank * bank_size) + address;
        
        if (mapped_addr < chr_rom_.size()) {
            data = chr_rom_[mapped_addr];
            return true;
        }
    }
    return false;
}

bool MapperSuborSB2000::ppu_write(uint16_t address, uint8_t data) {
    if (address < 0x2000 && chr_rom_.empty()) {
        return true; // CHR-RAM
    }
    (void)data;
    return false;
}

void MapperSuborSB2000::write_subor_register(uint16_t address, uint8_t data) {
    cpu_write(address, data);
}

void MapperSuborSB2000::set_key_state(uint8_t row, uint8_t col, bool pressed) {
    if (row < 8 && col < 8) {
        if (pressed) {
            keyboard_matrix_[row] &= ~(1 << col); // Active low bit clear
        } else {
            keyboard_matrix_[row] |= (1 << col);  // Bit set
        }
    }
}

void MapperSuborSB2000::set_mouse_position(int16_t x, int16_t y, bool left_click, bool right_click) {
    mouse_.x = x;
    mouse_.y = y;
    mouse_.left_button = left_click;
    mouse_.right_button = right_click;
}

bool MapperSuborSB2000::mount_floppy_disk(const std::string& image_path) {
    std::ifstream file(image_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Subor FDC Error: Failed to open floppy disk image: " << image_path << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    floppy_disk_data_.resize(size);
    if (file.read(reinterpret_cast<char*>(floppy_disk_data_.data()), size)) {
        fdc_.status_reg = 0x40; // Ready / Disk Loaded
        std::cout << "Subor FDC: Successfully mounted floppy image (" << (size / 1024) << " KB)\n";
        return true;
    }

    std::cerr << "Subor FDC Error: Failed to read floppy disk image data.\n";
    return false;
}

} // namespace Nesun
