#include <iostream>
#include <cassert>
#include "nesun/bus.hpp"
#include "nesun/cpu.hpp"

using namespace Nesun;

void test_cpu_initialization_and_reset() {
    Bus bus;
    CPU& cpu = bus.get_cpu();

    // Mock reset vector at $FFFC-$FFFD pointing to $8000
    // (In a real test, you'd load a small ROM into the bus)
    std::cout << "[Test] Running CPU initialization & reset checks...\n";
    
    cpu.reset();
    
    // Verify initial stack pointer and flag defaults
    assert(cpu.get_stack_pointer() == 0xFD);
    assert((cpu.get_status() & CPU::Flags::I) != 0); // Interrupt disable flag set
    
    std::cout << "[Test] CPU reset tests passed successfully!\n";
}

int main() {
    std::cout << "=== Nesun Core Unit Test Suite ===\n";
    
    test_cpu_initialization_and_reset();
    
    std::cout << "All CPU unit tests passed!\n";
    return 0;
}
