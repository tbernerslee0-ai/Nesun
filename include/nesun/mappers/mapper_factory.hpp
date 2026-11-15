#include "mapper.hpp"
#include <memory>
#include <vector>

namespace Nesun {

class MapperFactory {
public:
    // Parses iNES / NES 2.0 headers and returns the correct mapper implementation
    static std::unique_ptr<Mapper> create_mapper(
        uint8_t mapper_id, 
        const std::vector<uint8_t>& prg_rom, 
        const std::vector<uint8_t>& chr_rom,
        bool is_vtxx_system = false
    );
};

} // namespace Nesun
