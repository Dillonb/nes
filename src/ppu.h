#pragma once
#include <stdint.h>
#include "util.h"

typedef struct ppu_memory_t {
    unsigned long long frame;
    uint16_t scan_line;
    uint16_t cycle;
    byte control;
    byte mask;
    byte status;
    byte oam_address;
    byte oam_data[0xFF];
    byte scroll;
    uint16_t address;
    byte data;
    byte dma;
} ppu_memory;

ppu_memory get_ppu_mem();
void ppu_step(ppu_memory* ppu_mem);
byte read_ppu_register(ppu_memory* ppu_mem, byte register_num);
void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value);
void write_oam_byte(ppu_memory* ppu_mem, byte value);
