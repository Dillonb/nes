#pragma once
#include "util.h"

typedef struct ppu_memory_t {
     byte control;
     byte mask;
     byte oamAddress;
     byte oamData;
     byte scroll;
     byte address;
     byte data;
     byte dma;
} ppu_memory;

ppu_memory get_ppu_mem();
void ppu_step(ppu_memory* ppu_mem);
byte read_ppu_register(ppu_memory* ppu_mem, byte register_num);
void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value);
