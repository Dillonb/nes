#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "../util.h"

#define BYTES_PER_PRG_ROM_BLOCK 16384
#define BYTES_PER_PRG_RAM_BLOCK 8192
#define BYTES_PER_CHR_ROM_BLOCK 8192
#define TRAINER_BYTES 512

typedef struct ines_header_t {
    byte nes[4]; // 4 bytes, 0x4E 0x45 0x53 0x1A - ASCII: NES<EOF>
    byte prg_rom_blocks; // multiply by 16KB to get actual size of PRG ROM
    byte chr_rom_blocks; // multiply by 8KB to get actual size of CHR ROM. Value of 0 means board uses CHR RAM
    byte flags_6; // http://wiki.nesdev.com/w/index.php/INES#Flags_6
    byte flags_7; // http://wiki.nesdev.com/w/index.php/INES#Flags_7
    byte prg_ram_blocks; // Multiply by 8KB, 0 also means 8KB (compatibility reasons, see http://wiki.nesdev.com/w/index.php/PRG_RAM_circuit)
    byte flags_9; // http://wiki.nesdev.com/w/index.php/INES#Flags_9
    byte flags_10_unofficial; // http://wiki.nesdev.com/w/index.php/INES#Flags_10. Ignored.
    byte zero[5]; // All zeros
} ines_header;

typedef enum nametable_mirroring_t {
    HORIZONTAL,
    VERTICAL,
    SINGLE_LOWER,
    SINGLE_UPPER
} nametable_mirroring;

typedef struct mapper_data_t {
    byte prg_bank_mode;
    byte chr_bank_mode;

    byte chr_bank_0;
    byte chr_bank_1;

    byte ram_enabled;

    byte prg_bank;

    int prg_bank_0_offset;
    int prg_bank_1_offset;
    int prg_bank_2_offset;
    int prg_bank_3_offset;

    int chr_bank_0_offset;
    int chr_bank_1_offset;
    int chr_bank_2_offset;
    int chr_bank_3_offset;
    int chr_bank_4_offset;
    int chr_bank_5_offset;

    byte shift_register;
    byte bank_register;
    byte ram_write_protect;
    bool irq_enable;
    byte irq_latch;
    byte counter;
} mapper_data;

typedef struct rom_t {
  ines_header* header;
  byte* trainer; // 512 bytes, or NULL.
  byte* prg_rom;
  byte* chr_rom;
  byte mapper;
  nametable_mirroring nametable_mirroring_mode;
  mapper_data mapperdata;
  byte prg_ram[0x2000];
} rom;

size_t get_prg_rom_bytes(rom* r);
size_t get_prg_ram_bytes(rom* r);
size_t get_chr_rom_bytes(rom* r);
int has_trainer(ines_header* header);
rom* read_rom(char* filename);
unsigned char get_mapper_number(rom* r);
