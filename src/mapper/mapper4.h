#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"

void mapper4_init(rom* r);
byte mapper4_prg_read(rom* r, uint16_t address);
void mapper4_prg_write(rom* r, uint16_t address, byte value);
byte mapper4_chr_read(rom* r, uint16_t address);
void mapper4_chr_write(rom* r, uint16_t address, byte value);

