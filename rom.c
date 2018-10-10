#include <stdlib.h>
#include <stdio.h>

#include "rom.h"

int has_trainer(ines_header* header) {
  return (header->flags_6 & 0b00000100) > 0;
}

rom* read_rom(char* filename) {
  rom* r = malloc(sizeof(rom));
  ines_header* header = malloc(sizeof(ines_header));

  FILE* fp = fopen(filename, "rb");

  fread(header, sizeof(*header), 1, fp);
  r->header = header;

  if (!has_trainer(header)) {
      r->trainer = malloc(TRAINER_BYTES);
      fread(r->trainer, TRAINER_BYTES, 1, fp);
      printf("Has trainer.\n");
  } else {
      r->trainer = NULL;
      printf("No trainer!\n");
  }



  fclose(fp);
  return r;
}
