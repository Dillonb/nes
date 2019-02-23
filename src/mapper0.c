#include <err.h>
#include <stdlib.h>

#include "mapper0.h"
byte mapper0_prg_read(memory* mem, uint16_t address) {

    if (address >= 0x8000) { // Can't be more than 0xFFFF
        uint16_t prg_rom_address = (address - 0x8000) % get_prg_rom_bytes(mem->r); // TODO optimize
        byte result = mem->r->prg_rom[prg_rom_address];

        return result;
    }
    errx(EXIT_FAILURE, "attempted to read_cartridge_space_address() at 0x%x, but this is not implemented (yet?)", address);
}

byte mapper0_chr_read(rom* r, uint16_t address) {
    return 0x00;
}

void mapper0_prg_write(memory* mem, uint16_t address, byte value) {

}
