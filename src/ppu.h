#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "mapper/rom.h"

typedef enum high_or_low_t {
    HIGH,
    LOW
} high_or_low;

typedef struct tiledata_t {
    byte nametable;
    uint32_t attribute_table;
    uint16_t tile_bitmap_low;
    uint16_t tile_bitmap_high;
} tiledata;

typedef struct color_t {
    byte a;
    byte r;
    byte g;
    byte b;
} color;

typedef struct sprite_pattern_t {
    byte high_byte;
    byte low_byte;
    byte palette;
    bool reverse;
} sprite_pattern;

typedef struct sprite_t {
    byte x_coord;
    bool priority;
    byte index;
    sprite_pattern pattern;
} sprite;

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
    byte data;
    byte name_tables[0x800];
    byte palette_ram[0x20];

    uint16_t v; // Current VRAM address
    uint16_t t; // Temporary VRAM address
    byte x;     // Fine x scroll (this is only 3 bits)
    high_or_low w; // Keeps track of which byte to write to on 16 bit registers

    uint16_t temp_bitmap_low;
    uint16_t temp_bitmap_high;
    uint32_t temp_attribute_table;
    tiledata tile;

    color screen[256][240];

    sprite sprites[8];
    byte num_sprites;

    // For reading from 0x2007
    byte fake_buffer;
} ppu_memory;

ppu_memory get_ppu_mem(rom* r);
void ppu_step(ppu_memory* ppu_mem);
byte read_ppu_register(ppu_memory* ppu_mem, byte register_num);
void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value);
void write_oam_byte(ppu_memory* ppu_mem, byte value);
int get_screen_x(ppu_memory* ppu_mem);
int get_screen_y(ppu_memory* ppu_mem);
