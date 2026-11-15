#include "nesun/cpu.hpp"
#include "nesun/bus.hpp"
#include <iostream>

namespace Nesun {

CPU::CPU(Bus& bus) : bus_(bus) {}

void CPU::reset() {
    regs.A = 0;
    regs.X = 0;
    regs.Y = 0;
    regs.SP = 0xFD;
    regs.P = Flags::U | Flags::I;

    // Read Reset vector from $FFFC-$FFFD
    uint16_t lo = read(0xFFFC);
    uint16_t hi = read(0xFFFD);
    regs.PC = (hi << 8) | lo;

    cycles_remaining_ = 8;
}

void CPU::nmi() {
    push_stack((regs.PC >> 8) & 0xFF);
    push_stack(regs.PC & 0xFF);
    
    uint8_t flags = regs.P;
    flags &= ~Flags::B;
    flags |= Flags::U;
    push_stack(flags);

    regs.P |= Flags::I;

    uint16_t lo = read(0xFFFA);
    uint16_t hi = read(0xFFFB);
    regs.PC = (hi << 8) | lo;
    cycles_remaining_ = 7;
}

uint8_t CPU::step() {
    if (cycles_remaining_ == 0) {
        uint8_t opcode = fetch_opcode();
        
        // TODO: Expand full 6502 instruction decode table & opcode handlers here.
        // For baseline testing, dummy execution cycle:
        cycles_remaining_ = 2; 
    }

    cycles_remaining_--;
    return 1;
}

uint8_t CPU::read(uint16_t address) {
    return bus_.cpu_read(address);
}

void CPU::write(uint16_t address, uint8_t value) {
    bus_.cpu_write(address, value);
}

uint8_t CPU::fetch_opcode() {
    uint8_t op = read(regs.PC++);
    return op;
}

void CPU::push_stack(uint8_t value) {
    write(0x0100 + regs.SP--, value);
}

uint8_t CPU::pop_stack() {
    return read(0x0100 + ++regs.SP);
}

} // namespace Nesun
