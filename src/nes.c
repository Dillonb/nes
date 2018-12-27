#include <stdio.h>
#include <stdbool.h>

#include "system.h"
#include "cpu.h"
#include "mem.h"
#include "rom.h"
#include "util.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 2;
    }
    rom* r = read_rom(argv[1]);

    memory mem = get_blank_memory(r);

    //set_debug();
    set_breakpoint(0x8181);
    set_breakpoint(0x90dc);

    while (true) {
        system_step(&mem);
    }
}
