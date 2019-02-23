#pragma once
#include <stdint.h>
#include "util.h"
#include "mem.h"
byte mapper0_prg_read(memory* mem, uint16_t address);
void mapper0_prg_write(memory* mem, uint16_t address, byte value);

