#include <iostream>
#include <cassert>
#include <vector>
#include "nesun/cartridge.hpp"
#include "nesun/mappers/mapper_factory.hpp"

using namespace Nesun;

void test_mapper_factory_routing() {
    std::cout << "[Test] Testing MapperFactory system routing...\n";

    std::vector<uint8_t> dummy_prg(16384 * 2, 0xFF); // 32KB PRG
    std::vector<uint8_t> dummy_chr(8192, 0x00);      // 8KB CHR

    // Test VTxx Famiclone mapping
    auto vtxx_mapper = MapperFactory::create_mapper(200, dummy_prg, dummy_chr, true);
    assert(vtxx_mapper != nullptr);

    // Test Subor Educational System mapping (Mapper 256)
    auto subor_mapper = MapperFactory::create_mapper(256, dummy_prg, dummy_chr, false);
    assert(subor_mapper != nullptr);

    std::cout << "[Test] MapperFactory routing tests passed successfully!\n";
}

int main() {
    test_mapper_factory_routing();
    std::cout << "All Mapper unit tests passed!\n";
    return 0;
}
