#pragma once
#include "util.h"

typedef struct apu_memory_t {
} apu_memory;

byte read_apu_register(apu_memory* apu_mem, byte register_num);
void write_apu_register(apu_memory* apu_mem, int register_num, byte value);
void apu_step(apu_memory* apu_mem);