#pragma once

#include <cstdint>
#include <functional>

namespace Nesun {

// Forward declaration of the system Bus/Memory mapper
class Bus;

class CPU {
public:
    explicit CPU(Bus& bus);
    ~CPU() = default;

    // Execution control
    void reset();
    void irq();
    void nmi();
    uint8_t step(); // Executes the next instruction, returns cycles consumed

    // CPU Registers
    struct Registers {
        uint16_t PC = 0x0000; // Program Counter
        uint8_t  A  = 0x00;   // Accumulator
        uint8_t  X  = 0x00;   // X Index Register
        uint8_t  Y  = 0x00;   // Y Index Register
        uint8_t  SP = 0xFD;   // Stack Pointer
        uint8_t  P  = 0x34;   // Status Register (Processor Flags)
    } regs;

    // Status Flag Bitmasks
    enum Flags : uint8_t {
        C = (1 << 0), // Carry Flag
        Z = (1 << 1), // Zero Flag
        I = (1 << 2), // Interrupt Disable
        D = (1 << 3), // Decimal Mode (Unused on NES 6502)
        B = (1 << 4), // Break Command
        U = (1 << 5), // Unused (Always 1)
        V = (1 << 6), // Overflow Flag
        N = (1 << 7)  // Negative / Sign Flag
    };

    // Inspection helpers for debugging and UI overlays
    uint16_t get_pc() const { return regs.PC; }
    uint8_t get_accumulator() const { return regs.A; }
    uint8_t get_status() const { return regs.P; }
    uint8_t get_stack_pointer() const { return regs.SP; }

private:
    Bus& bus_;
    uint8_t cycles_remaining_ = 0;

    // Low-level bus helper wrappers
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
    uint8_t fetch_opcode();
    uint16_t read_address16(uint16_t address);

    // Internal instruction execution helpers
    void push_stack(uint8_t value);
    uint8_t pop_stack();
    void set_zn_flags(uint8_t value);
};

} // namespace Nesun
