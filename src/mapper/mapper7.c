#include <err.h>
#include <stdlib.h>
#include <stdint.h>

#include "rom.h"

void mapper7_init(mapper_data* mapperdata) {}

byte mapper7_prg_read(rom* r, uint16_t address) {

    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        byte result = r->prg_ram[address];
        return result;
    }
    else if (address >= 0x8000) { // Can't be more than 0xFFFF
        int prg_rom_index = (address - 0x8000) + r->mapperdata.prg_bank_0_offset;
        byte result = r->prg_rom[prg_rom_index];

        return result;
    }
    else {
        errx(EXIT_FAILURE, "Mapper 7: attempted to read at 0x%x, but this is not implemented (yet?)", address);
    }
}

void mapper7_prg_write(rom* r, uint16_t address, byte value) {
    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
        printf("Wrote 0x%02X to 0x%04X\n", value, address + 0x6000);
    }
    else if (address >= 0x8000){
        switch ((value & 0b00010000) >> 4) {
            case 0:
                r->nametable_mirroring_mode = SINGLE_LOWER;
                break;
            case 1:
                r->nametable_mirroring_mode = SINGLE_UPPER;
                break;
            default:
                break;
        }

        int prg_bank = value & (byte)0b00000111;
        r->mapperdata.prg_bank_0_offset = prg_bank * 0x8000;
    }
}

byte mapper7_chr_read(rom* r, uint16_t address) {
    return r->chr_rom[address];
}

void mapper7_chr_write(rom* r, uint16_t address, byte value) {
    r->chr_rom[address] = value;
}
