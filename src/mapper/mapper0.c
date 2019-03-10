#include <err.h>
#include <stdlib.h>
#include <stdint.h>

#include "rom.h"
byte mapper0_prg_read(rom* r, uint16_t address) {

    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        byte result = r->prg_ram[address];
        return result;
    }
    else if (address >= 0x8000) { // Can't be more than 0xFFFF
        uint16_t prg_rom_address = (uint16_t) ((address - 0x8000) % (int)get_prg_rom_bytes(r)); // TODO optimize
        byte result = r->prg_rom[prg_rom_address];

        return result;
    }
    else {
        errx(EXIT_FAILURE, "Mapper 0: attempted to read at 0x%x, but this is not implemented (yet?)", address);
    }
}

void mapper0_prg_write(rom* r, uint16_t address, byte value) {
    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
        printf("Wrote 0x%02X to 0x%04X\n", value, address + 0x6000);
    }
    else {
        printf("Tried to write 0x%02X to PRG at 0x%04X\n", value, address);
    }
}

byte mapper0_chr_read(rom* r, uint16_t address) {
    return r->chr_rom[address];
}

void mapper0_chr_write(rom* r, uint16_t address, byte value) {
    printf("WARNING: NROM pattern tables (CHR ROM) written to! allowing it because I'm a dumb emulator\n");
    r->chr_rom[address] = value;
}
