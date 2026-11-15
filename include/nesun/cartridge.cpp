#include "nesun/cartridge.hpp"
#include "nesun/mappers/mapper_factory.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

namespace Nesun {

bool Cartridge::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Cartridge Error: Failed to open file: " << filepath << "\n";
        return false;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size < 16) {
        std::cerr << "Cartridge Error: File too small to contain an iNES header.\n";
        return false;
    }

    // Read 16-byte iNES / NES 2.0 header
    std::vector<uint8_t> header(16);
    if (!file.read(reinterpret_cast<char*>(header.data()), 16)) {
        std::cerr << "Cartridge Error: Failed to read ROM header.\n";
        return false;
    }

    // Verify "NES\x1A" magic signature
    if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A) {
        std::cerr << "Cartridge Error: Invalid iNES magic number.\n";
        return false;
    }

    uint8_t prg_rom_chunks = header[4];
    uint8_t chr_rom_chunks = header[5];
    uint8_t flags_6 = header[6];
    uint8_t flags_7 = header[7];
    uint8_t flags_8 = header[8];
    uint8_t flags_9 = header[9];

    // Determine if format is NES 2.0
    bool is_nes_2_0 = (header[7] & 0x0C) == 0x08;

    // Extract Mapper ID
    if (is_nes_2_0) {
        mapper_id_ = ((header[8] & 0x0F) << 8) | (header[7 & 0xF0]) | (header[6] >> 4);
        submapper_id_ = header[8] >> 4;
        
        // Extended PRG/CHR sizing for NES 2.0
        uint16_t prg_msb = header[9] & 0x0F;
        if (prg_msb == 0x0F) {
            // Exponent-multiplier notation parsing could go here if needed
        }
    } else {
        mapper_id_ = (header[7] & 0xF0) | (header[6] >> 4);
    }

    // Mirroring detection
    if (flags_6 & 0x08) {
        default_mirroring_ = Mirroring::FourScreen;
    } else if (flags_6 & 0x01) {
        default_mirroring_ = Mirroring::Vertical;
    } else {
        default_mirroring_ = Mirroring::Horizontal;
    }

    has_battery_ram_ = (flags_6 & 0x02) != 0;

    // Check for 512-byte Trainer ($7000-$71FF)
    bool has_trainer = (flags_6 & 0x04) != 0;
    if (has_trainer) {
        trainer_.resize(512);
        file.read(reinterpret_cast<char*>(trainer_.data()), 512);
    }

    // Calculate PRG and CHR ROM sizes
    size_t prg_size = prg_rom_chunks * 16384; // 16KB per bank
    size_t chr_size = chr_rom_chunks * 8192;  // 8KB per bank

    prg_rom_.resize(prg_size);
    if (!file.read(reinterpret_cast<char*>(prg_rom_.data()), prg_size)) {
        std::cerr << "Cartridge Error: Failed to read PRG-ROM data.\n";
        return false;
    }

    if (chr_size > 0) {
        chr_rom_.resize(chr_size);
        file.read(reinterpret_cast<char*>(chr_rom_.data()), chr_size);
    } else {
        // If CHR-ROM is 0, allocate 8KB CHR-RAM
        chr_rom_.resize(8192, 0);
    }

    // =========================================================================
    // Famiclone & System Heuristic Detection (VTxx, Subor, Wiii3)
    // =========================================================================
    // Mappers in high ranges (e.g., custom multicarts or specific IDs) or 
    // filename indicators can flag specialized hardware emulation.
    if (mapper_id_ == 256 || mapper_id_ == 518) { // Example Subor / Famiclone IDs
        system_type_ = SystemType::SuborSB2000;
    } else if (mapper_id_ >= 200 && mapper_id_ <= 210) {
        system_type_ = SystemType::VTxx;
    } else if (filepath.find("wiii3") != std::string::npos) {
        system_type_ = SystemType::Wiii3;
    } else {
        system_type_ = SystemType::StandardNES;
    }

    valid_ = true;
    std::cout << "Successfully loaded ROM: " << filepath << "\n"
              << " - Mapper ID: " << mapper_id_ << "\n"
              << " - PRG-ROM: " << (prg_size / 1024) << " KB\n"
              << " - CHR-ROM: " << (chr_size / 1024) << " KB\n"
              << " - System Type: " << (system_type_ == SystemType::SuborSB2000 ? "Subor SB-2000" :
                                        system_type_ == SystemType::VTxx ? "VTxx Famiclone" :
                                        system_type_ == SystemType::Wiii3 ? "Wiii3" : "Standard NES") << "\n";

    return true;
}

std::unique_ptr<Mapper> Cartridge::create_mapper() const {
    if (!valid_) return nullptr;

    bool is_vtxx = (system_type_ == SystemType::VTxx);
    return MapperFactory::create_mapper(static_cast<uint8_t>(mapper_id_ & 0xFF), prg_rom_, chr_rom_, is_vtxx);
}

} // namespace Nesun
