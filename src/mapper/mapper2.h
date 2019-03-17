#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"

void mapper2_init(mapper_data* mapperdata);
byte mapper2_prg_read(rom* r, uint16_t address);
void mapper2_prg_write(rom* r, uint16_t address, byte value);
byte mapper2_chr_read(rom* r, uint16_t address);
void mapper2_chr_write(rom* r, uint16_t address, byte value);
