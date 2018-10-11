#pragma once

typedef struct cpu_registers_t {
    // accumulator
    unsigned char a;

    // x index register
    unsigned char x;

    // y index register
    unsigned char y;

    // stack pointer
    unsigned char sp;

    // program counter
    unsigned short pc;

    // processor status register (bitwise flags)
    unsigned char p;
} cpu_registers;

cpu_registers get_blank_registers() {
    cpu_registers registers;

    registers.a = 0x00;
    registers.x = 0x00;
    registers.y = 0x00;
    registers.sp = 0x00;
    registers.pc = 0x0000;
    registers.p = 0b00000100;
}
