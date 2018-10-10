#include <stdio.h>

#include "rom.h"

int main(int argc, char** argv) {
    rom* r = read_rom("smd.nes");
    printf("Sup %s", r->header->nes);
}
