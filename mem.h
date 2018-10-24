#pragma once
#include <stdint.h>

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
    uint16_t pc;

    // processor status register (bitwise flags)
    // NV-BDIZC
    byte p;

    // currently loaded nes rom
    rom* r;

    // Internal RAM
    char ram[0x800];
} memory;

byte read_byte(memory* mem, uint16_t address);
void write_byte(memory* mem, uint16_t address, byte value);


void set_flag(memory* mem, int index);
void clear_flag(memory* mem, int index);
int get_flag(memory* mem, int index);


memory get_blank_memory();

void load_rom_into_memory(memory* mem, rom* r);
