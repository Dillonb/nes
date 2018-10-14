#pragma once
#include "mem.h"

char read_byte_and_inc_pc(memory* mem);
void cpu_step(memory* mem);
