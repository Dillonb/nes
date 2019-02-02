#pragma once
#include <stdint.h>
#include "util.h"
#include "rom.h"

typedef enum high_or_low_t {
    HIGH,
    LOW
} high_or_low;

typedef struct tiledata_t {
    byte nametable;
    byte attribute_table;
    byte tile_bitmap_low;
    byte tile_bitmap_high;
} tiledata;

typedef struct color_t {
    byte r;
    byte g;
    byte b;
} color;

typedef struct ppu_memory_t {
    rom* r;
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
    byte name_tables[0x1000];
    byte palette_ram[0x20];

    uint16_t v; // Current VRAM address
    uint16_t t; // Temporary VRAM address
    byte x;     // Fine x scroll (this is only 3 bits)
    high_or_low w; // Keeps track of which byte to write to on 16 bit registers

    tiledata tile;

    color screen[256][240];

} ppu_memory;

ppu_memory get_ppu_mem(rom* r);
void ppu_step(ppu_memory* ppu_mem);
byte read_ppu_register(ppu_memory* ppu_mem, byte register_num);
void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value);
void write_oam_byte(ppu_memory* ppu_mem, byte value);
