#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Nesun::Subor {

class FloppyController {
public:
    FloppyController();
    ~FloppyController() = default;

    void reset();

    // Disk Image Management
    bool load_image(const std::string& filepath);
    bool is_disk_loaded() const { return !disk_image_.empty(); }

    // Controller Register Interfacing
    uint8_t read_status();
    uint8_t read_data();
    void write_command(uint8_t cmd);
    void write_data(uint8_t data);

    // Sector geometry constants for standard Subor floppy images (e.g., 720KB / 1.44MB formats)
    static constexpr size_t SECTOR_SIZE = 512;

private:
    std::vector<uint8_t> disk_image_;
    
    uint8_t status_register_ = 0x80; // Ready flag set by default
    uint8_t current_track_ = 0;
    uint8_t current_sector_ = 1;
    uint8_t current_head_ = 0;
    uint32_t buffer_offset_ = 0;

    void update_buffer_offset();
};

} // namespace Nesun::Subor
