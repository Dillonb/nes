#include <stdlib.h>
#include <stdio.h>

#include "rom.h"

int has_trainer(ines_header* header) {
  return (header->flags_6 & 0b00000100) > 0;
}


// Read trainer
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

size_t get_prg_rom_bytes(rom* r) {
  return BYTES_PER_PRG_ROM_BLOCK * r->header->prg_rom_blocks;
}

rom* read_rom(char* filename) {
  rom* r = malloc(sizeof(rom));
  ines_header* header = malloc(sizeof(ines_header));

  FILE* fp = fopen(filename, "rb");

  fread(header, sizeof(*header), 1, fp);
  r->header = header;

  read_trainer(fp, r);
  read_prg_rom(fp, r);


  fclose(fp);
  return r;
}
