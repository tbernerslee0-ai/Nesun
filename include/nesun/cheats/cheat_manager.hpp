#pragma once

#include "game_genie.hpp"
#include <string>
#include <unordered_map>

namespace Nesun::Cheats {

class CheatManager {
public:
    bool add_cheat(const std::string& name, const std::string& code_str);
    void add_raw_patch(const std::string& name, uint16_t address, uint8_t value, std::optional<uint8_t> compare = std::nullopt);
    void remove_cheat(const std::string& name);
    void clear();

    // Hot-path bus interceptor: returns true if 'data' was modified
    bool apply_patches(uint16_t address, uint8_t& data) const;

    // Accessor for ImGui overlay iteration
    std::unordered_map<std::string, CheatCode>& get_cheats() { return active_cheats_; }
    const std::unordered_map<std::string, CheatCode>& get_cheats() const { return active_cheats_; }

private:
    std::unordered_map<std::string, CheatCode> active_cheats_;
};

} // namespace Nesun::Cheats
