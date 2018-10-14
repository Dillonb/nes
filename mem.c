#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "mem.h"

char read_cartridge_space_address(rom* r, unsigned short address) {
    errx(EXIT_FAILURE, "read_cartridge_space_address() not implemented");
}

char read_byte(memory* mem, unsigned short address) {
    if (address < 0x2000) {
        return mem->ram[address % 0x800];
    }
    if (address >= 0x4020 && address < 0x10000) {
        // cartridge space
        return read_cartridge_space_address(mem->r, address);
    }

    errx(EXIT_FAILURE, "Access attempted for invalid address: %x", address);
}

memory get_blank_memory(rom* r) {
    // http://wiki.nesdev.com/w/index.php/CPU_power_up_state
    memory mem;

    mem.a = 0x00;
    mem.x = 0x00;
    mem.y = 0x00;
    mem.sp = 0xFD;
    mem.p = 0x34;
    mem.r = r;

    mem.pc = (read_cartridge_space_address(r, 0xFFFD) << 8) & read_cartridge_space_address(r, 0xFFFC);

    return mem;
}
