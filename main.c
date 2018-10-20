#include <stdio.h>

#include "cpu.h"
#include "mem.h"
#include "rom.h"
#include "util.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 1;
    }
    rom* r = read_rom(argv[1]);

    /*
    int bytes = get_prg_rom_bytes(r);
    for (int i = 0; i < bytes; i++) {
      if (i % 16 == 0) {
        printf("\n0x04%x ", i);
      }
      printf("%02x ", r->prg_rom[i]);
    }
    */

    printf("\n");

    memory mem = get_blank_memory(r);

    while (true) {
        cpu_step(&mem);
    }
}
