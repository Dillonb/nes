#pragma once
#include "mem.h"
#include "util.h"

typedef enum interrupt_type_t {
    NONE,
    nmi,
    irq
} interrupt_type;


byte read_byte_and_inc_pc(memory* mem);
uint16_t read_address(memory* mem, uint16_t address);
int cpu_step(memory* mem);
const char* opcode_to_name_full(byte opcode);
const char* opcode_to_name_short(byte opcode);
void trigger_nmi();
void stall_cpu(int cycles);
void trigger_oam_dma(memory* mem, uint16_t address);
long get_total_cpu_cycles();

typedef enum addressing_mode_t {
    Implied,
    Immediate,
    Zeropage,
    Zeropage_Y,
    Zeropage_X,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Relative,
    Indirect,
    Indirect_X,
    Indirect_Y,
    Accumulator
} addressing_mode;
