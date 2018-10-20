#include <stdlib.h>
#include <stdio.h>

#include "rom.h"

/*
 * For extracting information out of the header
 */
int has_trainer(ines_header* header) {
    return (header->flags_6 & 0b00000100) > 0;
}

size_t get_prg_rom_bytes(rom* r) {
  return BYTES_PER_PRG_ROM_BLOCK * r->header->prg_rom_blocks;
}

size_t get_chr_rom_bytes(rom* r) {
  int blocks = BYTES_PER_CHR_ROM_BLOCK;

  // See https://wiki.nesdev.com/w/index.php/INES#iNES_file_format
  if (blocks == 0) {
    blocks = 1;
  }

  return blocks * r->header->chr_rom_blocks;
}


unsigned char get_mapper_number(rom* r) {
  unsigned char lower_nybble = r->header->flags_6 & 0b11110000;
  unsigned char upper_nybble = r->header->flags_7 & 0b11110000;

  return (lower_nybble >> 4 ) | upper_nybble;
}

/*
 * For reading the rom itself
 */
void read_trainer(FILE* fp, rom* r) {
    if (has_trainer(r->header)) {
        r->trainer = malloc(TRAINER_BYTES);
        fread(r->trainer, TRAINER_BYTES, 1, fp);
        printf("Has trainer.\n");
    } else {
        r->trainer = NULL;
        printf("No trainer.\n");
    }
}

void read_prg_rom(FILE* fp, rom* r) {
    size_t prg_rom_bytes = get_prg_rom_bytes(r);
    r->prg_rom = malloc(prg_rom_bytes);
    fread(r->prg_rom, prg_rom_bytes, 1, fp);
    printf("Read %lu bytes of PRG ROM\n", prg_rom_bytes);
}

void read_chr_rom(FILE* fp, rom* r) {
    size_t chr_rom_bytes = get_chr_rom_bytes(r);
    r->chr_rom = malloc(chr_rom_bytes);
    fread(r->chr_rom, chr_rom_bytes, 1, fp);
    printf("Read %lu bytes of CHR ROM\n", chr_rom_bytes);
}

rom* read_rom(char* filename) {
    rom* r = malloc(sizeof(rom));
    ines_header* header = malloc(sizeof(ines_header));

    FILE* fp = fopen(filename, "rb");

    fread(header, sizeof(*header), 1, fp);
    r->header = header;

    read_trainer(fp, r);
    read_prg_rom(fp, r);
    read_chr_rom(fp, r);

    r->mapper = get_mapper_number(r);

    printf("Rom has mapper %d\n", r->mapper);

    fclose(fp);
    return r;
}
