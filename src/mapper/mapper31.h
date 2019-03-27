#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"

void mapper31_init(mapper_data* mapperdata);
byte mapper31_prg_read(rom* r, uint16_t address);
void mapper31_prg_write(rom* r, uint16_t address, byte value);
byte mapper31_chr_read(rom* r, uint16_t address);
void mapper31_chr_write(rom* r, uint16_t address, byte value);

