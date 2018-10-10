#include <stdio.h>

#include "rom.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 1;
    }
    rom* r = read_rom(argv[1]);
    printf("Sup %s", r->header->nes);
}
