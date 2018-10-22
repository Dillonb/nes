#include <stdio.h>

#include "rom.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 1;
    }
    rom* r = read_rom(argv[1]);

    int bytes = get_prg_rom_bytes(r);
    for (int i = 0; i < bytes; i++) {
      if (i % 16 == 0) {
        printf("\n0x%04x: ", i);
      }
      printf("%02x", r->prg_rom[i]);
      if (i % 2 == 1) {
          printf(" ");
      }
    }

    printf("\n");
}
