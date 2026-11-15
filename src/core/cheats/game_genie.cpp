#include "nesun/cheats/game_genie.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace Nesun::Cheats {

// Official 16-character NES Game Genie alphabet
static const char GG_ALPHABET[] = "APZLIKDCJESVFTGY";

int GameGenie::char_to_value(char c) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    for (int i = 0; i < 16; ++i) {
        if (GG_ALPHABET[i] == upper) {
            return i;
        }
    }
    return -1; // Invalid character
}

std::optional<CheatCode> GameGenie::decode(const std::string& code_str) {
    // Clean string (remove spaces or hyphens)
    std::string clean_code;
    for (char c : code_str) {
        if (c != '-' && !std::isspace(static_cast<unsigned char>(c))) {
            clean_code += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }

    if (clean_code.length() != 6 && clean_code.length() != 8) {
        return std::nullopt; // Game Genie codes must be either 6 or 8 characters
    }

    // Convert ASCII characters to 4-bit nibbles using the alphabet
    std::vector<int> vals(clean_code.length());
    for (size_t i = 0; i < clean_code.length(); ++i) {
        int val = char_to_value(clean_code[i]);
        if (val == -1) {
            return std::nullopt; // Invalid character found
        }
        vals[i] = val;
    }

    CheatCode cheat;
    cheat.enabled = true;

    if (clean_code.length() == 6) {
        // 6-Character Code Decoding: [a1 a0 f3 f2 f1 f0] [d3 d2 d1 d0] [e3 e2 e1 e0] ...
        // Decodes to: Address (16-bit), Patch Value (8-bit)
        
        // Nibble extraction and bit-shuffling for 6-letter codes:
        // c1 = vals[0], c2 = vals[1], c3 = vals[2], c4 = vals[3], c5 = vals[4], c6 = vals[5]
        
        uint8_t n0 = vals[0];
        uint8_t n1 = vals[1];
        uint8_t n2 = vals[2];
        uint8_t n3 = vals[3];
        uint8_t n4 = vals[4];
        uint8_t n5 = vals[5];

        uint8_t val  = (n5 & 0x07) | ((n4 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val2 = (n4 & 0x07) | ((n3 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val3 = (n3 & 0x07) | ((n2 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val4 = (n2 & 0x07) | ((n1 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val5 = (n1 & 0x07) | ((n0 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val6 = n0 & 0x07;

        // Reconstruct Address and Patch Value
        cheat.patch_value = (val << 4) | val2;
        
        uint16_t addr_hi = (val6 << 4) | (val5 & 0x0F);
        // Apply NES PRG-ROM mapping offset ($8000 base)
        cheat.address = 0x8000 + (((val4 & 0x07) << 12) | ((val3 & 0x0F) << 8) | ((val5 & 0x10) != 0 ? 0x80 : 0) | addr_hi);
        cheat.compare_value = std::nullopt;
    } 
    else {
        // 8-Character Code Decoding (includes 8-bit comparison / check value)
        uint8_t n0 = vals[0];
        uint8_t n1 = vals[1];
        uint8_t n2 = vals[2];
        uint8_t n3 = vals[3];
        uint8_t n4 = vals[4];
        uint8_t n5 = vals[5];
        uint8_t n6 = vals[6];
        uint8_t n7 = vals[7];

        uint8_t val0 = (n7 & 0x07) | ((n6 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val1 = (n6 & 0x07) | ((n5 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val2 = (n5 & 0x07) | ((n4 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val3 = (n4 & 0x07) | ((n3 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val4 = (n3 & 0x07) | ((n2 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val5 = (n2 & 0x07) | ((n1 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val6 = (n1 & 0x07) | ((n0 & 0x08) != 0 ? 0x08 : 0);
        uint8_t val7 = n0 & 0x07;

        cheat.patch_value = (val0 << 4) | val1;
        uint8_t cmp_val   = (val2 << 4) | val3;
        
        uint16_t addr_hi = (val7 << 4) | (val6 & 0x0F);
        cheat.address = 0x8000 + (((val5 & 0x07) << 12) | ((val4 & 0x0F) << 8) | addr_hi);
        cheat.compare_value = cmp_val;
    }

    return cheat;
}

} // namespace Nesun::Cheats
