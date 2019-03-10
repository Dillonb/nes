#pragma once
#include "rom.h"
byte mapper_prg_read(rom* r, uint16_t address);
void mapper_prg_write(rom* r, uint16_t address, byte value);

byte mapper_chr_read(rom* r, uint16_t address);
void mapper_chr_write(rom* r, uint16_t address, byte value);
