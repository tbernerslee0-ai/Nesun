#pragma once

#include <string>
#include <vector>
#include <memory>
#include "mappers/mapper.hpp"

namespace Nesun {

enum class SystemType {
    StandardNES,
    VTxx,
    SuborSB2000,
    Wiii3
};

class Cartridge {
public:
    Cartridge() = default;
    ~Cartridge() = default;

    // Loads and parses an iNES / NES 2.0 file from disk
    bool load_from_file(const std::string& filepath);

    // Getters for extracted ROM data and metadata
    bool is_valid() const { return valid_; }
    uint16_t get_mapper_id() const { return mapper_id_; }
    uint8_t get_submapper_id() const { return submapper_id_; }
    SystemType get_system_type() const { return system_type_; }
    
    const std::vector<uint8_t>& get_prg_rom() const { return prg_rom_; }
    const std::vector<uint8_t>& get_chr_rom() const { return chr_rom_; }

    // Instantiates and returns the correct Mapper subclass via MapperFactory
    std::unique_ptr<Mapper> create_mapper() const;

private:
    bool valid_ = false;
    uint16_t mapper_id_ = 0;
    uint8_t submapper_id_ = 0;
    SystemType system_type_ = SystemType::StandardNES;

    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> chr_rom_;
    std::vector<uint8_t> trainer_;
    
    Mirroring default_mirroring_ = Mirroring::Horizontal;
    bool has_battery_ram_ = false;
};

} // namespace Nesun
