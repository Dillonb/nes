#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"
byte mapper0_prg_read(rom* r, uint16_t address);
void mapper0_prg_write(rom* r, uint16_t address, byte value);
byte mapper0_chr_read(rom* r, uint16_t address);
void mapper0_chr_write(rom* r, uint16_t address, byte value);

