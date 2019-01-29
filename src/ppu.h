#pragma once
#include <stdint.h>
#include "util.h"

typedef enum high_or_low_t {
    HIGH,
    LOW
} high_or_low;

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
    byte data;
    byte dma;
    byte pattern_tables[0x2000];
    byte name_tables[0x1000];
    byte palette_ram[0x20];

    uint16_t v; // Current VRAM address
    uint16_t t; // Temporary VRAM address
    byte x;     // Fine x scroll (this is only 3 bits)
    high_or_low w; // Keeps track of which byte to write to on 16 bit registers
} ppu_memory;

ppu_memory get_ppu_mem();
void ppu_step(ppu_memory* ppu_mem);
byte read_ppu_register(ppu_memory* ppu_mem, byte register_num);
void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value);
void write_oam_byte(ppu_memory* ppu_mem, byte value);
