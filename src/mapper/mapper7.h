#pragma once
#include <stdint.h>
#include "../util.h"
#include "rom.h"

void mapper7_init(mapper_data* mapperdata);
byte mapper7_prg_read(rom* r, uint16_t address);
void mapper7_prg_write(rom* r, uint16_t address, byte value);
byte mapper7_chr_read(rom* r, uint16_t address);
void mapper7_chr_write(rom* r, uint16_t address, byte value);

