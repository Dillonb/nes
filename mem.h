#pragma once
#include "rom.h"

typedef struct memory_t {
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

    // currently loaded nes rom
    rom* r;

    // Internal RAM
    char ram[0x800];
} memory;

char read_byte(memory* mem, unsigned short address);

memory get_blank_memory();

void load_rom_into_memory(memory* mem, rom* r);
