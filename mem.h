#pragma once
#include "rom.h"
#include "util.h"

typedef struct memory_t {
    // accumulator
    byte a;

    // x index register
    byte x;

    // y index register
    byte y;

    // stack pointer
    byte sp;

    // program counter
    unsigned short pc;

    // processor status register (bitwise flags)
    byte p;

    // currently loaded nes rom
    rom* r;

    // Internal RAM
    char ram[0x800];
} memory;

byte read_byte(memory* mem, unsigned short address);
void write_byte(memory* mem, unsigned short address, byte value);

memory get_blank_memory();

void load_rom_into_memory(memory* mem, rom* r);
