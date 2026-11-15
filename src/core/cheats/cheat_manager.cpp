#include "nesun/cheats/game_genie.hpp"
#include <unordered_map>
#include <iostream>

namespace Nesun::Cheats {

class CheatManager {
public:
    void add_cheat(const std::string& name, const std::string& code_str) {
        auto decoded = GameGenie::decode(code_str);
        if (decoded) {
            active_cheats_[name] = *decoded;
            std::cout << "CheatManager: Successfully applied cheat '" << name << "' at 0x" 
                      << std::hex << decoded->address << std::dec << "\n";
        } else {
            std::cerr << "CheatManager Error: Invalid Game Genie code: " << code_str << "\n";
        }
    }

    void remove_cheat(const std::string& name) {
        active_cheats_.erase(name);
    }

    // Called inside Bus::cpu_read when accessing PRG-ROM or RAM
    bool apply_patches(uint16_t address, uint8_t& data) const {
        for (const auto& [name, cheat] : active_cheats_) {
            if (!cheat.enabled) continue;

            if (cheat.address == address) {
                // If a compare value is required, verify original data matches first
                if (cheat.compare_value.has_value() && data != *cheat.compare_value) {
                    continue; // Condition not met
                }
                data = cheat.patch_value;
                return true; // Patch applied
            }
        }
        return false;
    }

private:
    std::unordered_map<string, CheatCode> active_cheats_;
};

} // namespace Nesun::Cheats
