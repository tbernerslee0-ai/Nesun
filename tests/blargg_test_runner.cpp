#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "nesun/bus.hpp"
#include "nesun/cpu.hpp"
#include "nesun/ppu.hpp"
#include "nesun/apu.hpp"
#include "nesun/cartridge.hpp"

using namespace Nesun;

int main(int argc, char* argv[]) {
    std::cout << "=== Nesun Blargg PPU/APU Test Runner ===\n";

    if (argc < 2) {
        std::cerr << "Usage: ./nesun_blargg_runner <path_to_blargg_rom.nes>\n";
        return 1;
    }

    std::string rom_path = argv[1];

    // 1. Load the Blargg ROM
    Cartridge cartridge;
    if (!cartridge.load_from_file(rom_path)) {
        std::cerr << "Error: Failed to load ROM file: " << rom_path << "\n";
        return 1;
    }

    // 2. Initialize Bus and Mapper
    Bus bus;
    bus.insert_cartridge(cartridge.create_mapper());
    bus.reset();

    CPU& cpu = bus.get_cpu();
    PPU& ppu = bus.get_ppu();
    APU& apu = bus.get_apu();

    std::cout << "Executing Blargg test ROM: " << rom_path << " ...\n";

    // 3. Emulation Execution Loop
    // Blargg tests typically run for several seconds (approx. 300 to 600 frames)
    const int max_frames = 600; 
    bool test_finished = false;
    int status_code = 0;

    for (int frame = 0; frame < max_frames; ++frame) {
        // Run one full frame of CPU, PPU, and APU cycles
        ppu.clear_frame_complete();
        while (!ppu.is_frame_complete()) {
            cpu.step();
            ppu.step();
            apu.step();
        }

        // Check Blargg standard communication protocol in RAM ($6000 range)
        // $6000: Execution status (0x00 = running, 0x80 = tests running/passed text ready, etc.)
        // $6001: Result code / test result byte
        uint8_t run_status = bus.cpu_read(0x6000, true);
        status_code = bus.cpu_read(0x6001, true);

        // If $6000 contains text output signature or execution has terminated
        if (run_status == 0x80 || run_status == 0x01 || run_status == 0x00) {
            // Optional: Read out diagnostic string buffer starting at $6004 if present
            uint8_t char_test = bus.cpu_read(0x6004, true);
            if (char_test != 0 && run_status != 0x00) {
                // Read null-terminated diagnostic text written by test ROM
                std::string diagnostic_msg = "";
                uint16_t ptr = 0x6004;
                while (true) {
                    uint8_t c = bus.cpu_read(ptr++, true);
                    if (c == 0 || c == 0xFF) break;
                    diagnostic_msg += static_cast<char>(c);
                }
                
                if (!diagnostic_msg.empty()) {
                    std::cout << "\n[Blargg Output]:\n" << diagnostic_msg << "\n";
                }
            }
        }

        // Check if test reached a final status state
        if (status_code > 0 && status_code < 0x80) {
            // Test completed with a definitive pass/fail status code
            test_finished = true;
            break;
        }
    }

    // 4. Evaluate Test Results
    std::cout << "\n--- Test Execution Summary ---\n";
    if (test_finished) {
        if (status_code == 1) {
            std::cout << "RESULT: PASSED! All assertions in " << rom_path << " cleared successfully.\n";
            return 0;
        } else {
            std::cerr << "RESULT: FAILED! Test returned error code: 0x" << std::hex << status_code << std::dec << "\n";
            return 1;
        }
    } else {
        std::cerr << "RESULT: TIMEOUT! Test did not complete within the allotted frame window.\n";
        return 2;
    }
}
