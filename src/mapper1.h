#pragma once
#include <stdint.h>
#include "util.h"
#include "mem.h"
byte mapper1_prg_read(memory* mem, uint16_t address);
void mapper1_prg_write(memory* mem, uint16_t address, byte value);
