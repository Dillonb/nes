#include <stdio.h>
#include <stdbool.h>

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

    memory mem = get_blank_memory(r);

    while (true) {
        cpu_step(&mem);
    }
}
