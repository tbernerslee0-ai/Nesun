#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace Nesun::Cheats {

struct CheatCode {
    uint16_t address;
    uint8_t patch_value;
    std::optional<uint8_t> compare_value;
    bool enabled = true;
};

class GameGenie {
public:
    // Decodes a 6-character or 8-character Game Genie code into address, patch, and optional compare value
    static std::optional<CheatCode> decode(const std::string& code_str);

private:
    static int char_to_value(char c);
};

} // namespace Nesun::Cheats
