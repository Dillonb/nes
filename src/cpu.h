#pragma once
#include "mem.h"
#include "util.h"

#define BRK 0x0
#define SEI 0x78
#define STA_Absolute 0x8D
#define TXS 0x9A
#define LDX_Immediate 0xA2
#define LDA_Immediate 0xA9
#define LDA_Absolute 0xAD
#define CLD 0xD8


byte read_byte_and_inc_pc(memory* mem);
void cpu_step(memory* mem);
