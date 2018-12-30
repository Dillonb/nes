#pragma once
#include "mem.h"
#include "util.h"

typedef enum interrupt_type_t {
    NONE,
    nmi,
    irq
} interrupt_type;


byte read_byte_and_inc_pc(memory* mem);
int cpu_step(memory* mem);
const char* opcode_to_name_full(byte opcode);
const char* opcode_to_name_short(byte opcode);
void trigger_nmi();
