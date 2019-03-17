#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"

void mapper1_init(mapper_data* mapperdata);
byte mapper1_prg_read(rom* r, uint16_t address);
void mapper1_prg_write(rom* r, uint16_t address, byte value);
byte mapper1_chr_read(rom* r, uint16_t address);
void mapper1_chr_write(rom* r, uint16_t address, byte value);


int get_last_prg_bank(rom* r);
