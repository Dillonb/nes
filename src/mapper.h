#pragma once
#include "mem.h"
byte mapper_prg_read(memory* mem, uint16_t address);
void mapper_prg_write(memory* mem, uint16_t address, byte value);

byte mapper_chr_read(ppu_memory* ppu_mem, uint16_t address);
void mapper_chr_write(ppu_memory* ppu_mem, uint16_t address, byte value);
