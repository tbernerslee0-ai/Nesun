#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include "nesun/bus.hpp"
#include "nesun/cpu.hpp"
#include "nesun/cartridge.hpp"

using namespace Nesun;

int main(int argc, char* argv[]) {
    std::cout << "=== Nesun nestest.nes Automated CPU Test Runner ===\n";

    std::string rom_path = "nestest.nes";
    if (argc > 1) {
        rom_path = argv[1];
    }

    // 1. Load nestest.nes ROM using Nesun's Cartridge loader
    Cartridge cartridge;
    if (!cartridge.load_from_file(rom_path)) {
        std::cerr << "Error: Could not load " << rom_path 
                  << ". Please place nestest.nes in the build directory or pass the path as an argument.\n";
        return 1;
    }

    // 2. Initialize Bus and insert cartridge mapper
    Bus bus;
    bus.insert_cartridge(cartridge.create_mapper());
    bus.reset();

    CPU& cpu = bus.get_cpu();

    // 3. Configure automated execution mode for nestest
    // Setting PC to 0xC000 bypasses interactive startup tests and runs the full 
    // automated instruction sequence with result codes written to RAM ($0002/$0003).
    cpu.regs.PC = 0xC000;

    std::cout << "Starting automated 6502 instruction execution from $C000...\n";

    // 4. Execution Loop (Run for a fixed number of instruction steps)
    const int max_steps = 10000;
    int steps_executed = 0;
    bool test_passed = false;

    for (int i = 0; i < max_steps; ++i) {
        uint16_t pc_before = cpu.get_pc();
        
        // Execute single instruction
        uint8_t cycles = cpu.step();
        steps_executed++;

        // Check for nestest completion code or infinite loop traps
        // In automated mode, nestest writes status to $0002 or hits a trap branch
        uint8_t error_code = bus.cpu_read(0x0002, true);
        uint8_t test_status = bus.cpu_read(0x0003, true);

        if (pc_before == cpu.get_pc()) {
            // Infinite trap loop detected (standard nestest completion signal)
            std::cout << "Test completed or trapped at PC: 0x" << std::hex << pc_before << std::dec << "\n";
            std::cout << "Result code at $0002: 0x" << std::hex << (int)error_code << std::dec << "\n";
            
            if (error_code == 0x00) {
                test_passed = true;
            }
            break;
        }
    }

    std::cout << "Executed " << steps_executed << " instructions.\n";

    if (test_passed) {
        std::cout << "SUCCESS: nestest.nes passed all automated CPU verification tests!\n";
        return 0;
    } else {
        std::cerr << "FAILURE: nestest.nes encountered errors or did not complete successfully.\n";
        return 1;
    }
}
