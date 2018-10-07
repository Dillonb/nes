#include <stdlib.h>

#include "rom.h"

int has_trainer(ines_header* header) {
  return (header->flags_6 & 0b00000100) > 0;
}

rom* read_rom(char* filename) {
  rom* r = malloc(sizeof(rom));

  return r;
}
