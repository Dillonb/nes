#pragma once
#include <stdint.h>
#include "util.h"
#include "mem.h"
byte mapper1_prg_read(memory* mem, uint16_t address);
void mapper1_prg_write(memory* mem, uint16_t address, byte value);
byte mapper1_chr_read(ppu_memory* ppu_mem, uint16_t address);
void mapper1_chr_write(ppu_memory* ppu_mem, uint16_t address, byte value);


int get_last_prg_bank(memory* mem);
