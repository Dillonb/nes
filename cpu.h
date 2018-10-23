#pragma once
#include "mem.h"
#include "util.h"

const byte BRK = 0x0;
const byte SEI = 0x78;
const byte STA_Absolute = 0x8D;
const byte TXS = 0x9A;
const byte LDX_Immediate = 0xA2;
const byte LDA_Immediate = 0xA9;
const byte LDA_Absolute = 0xAD;
const byte CLD = 0xD8;

byte read_byte_and_inc_pc(memory* mem);
void cpu_step(memory* mem);
