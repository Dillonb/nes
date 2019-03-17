#include <err.h>
#include <stdlib.h>

#include "mapper2.h"
#include "mapper1.h" // For helper functions

#include "../debugger.h"

void mapper2_init(mapper_data* mapperdata) {
    mapperdata->prg_bank_0_offset = 0;
    mapperdata->prg_bank_1_offset = -1;
}

byte mapper2_prg_read(rom* r, uint16_t address) {
    byte result;

    if (address < 0x6000) {
        dprintf("Mapper 2: Sub-0x6000 unsupported memory address read, 0x%04X\n", address);
        result = 0x00;
    }
    else if (address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        result = r->prg_ram[address];
    }
    else if (address < 0xC000) { // PRG bank 0, 0x8000 - 0xBFFF
        result = r->prg_rom[r->mapperdata.prg_bank_0_offset + (address % 0x4000)];
    }
    else { // PRG bank 1, 0xC000 - 0xFFFF
        if (r->mapperdata.prg_bank_1_offset == -1) {
            r->mapperdata.prg_bank_1_offset = get_last_prg_bank(r);
        }

        result = r->prg_rom[r->mapperdata.prg_bank_1_offset + (address % 0x4000)];
    }
    return result;
}

void mapper2_prg_write(rom* r, uint16_t address, byte value) {
    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
        printf("Wrote 0x%02X to 0x%04X\n", value, address + 0x6000);
    }
    else if (address >= 0x8000) {
        r->mapperdata.prg_bank_0_offset = (value % (r->header->prg_rom_blocks - 1)) * BYTES_PER_PRG_ROM_BLOCK;
    }
    else {
        printf("Mapper 2: unhandled write 0x%02X to PRG at 0x%04X\n", value, address);
    }
}

byte mapper2_chr_read(rom* r, uint16_t address) {
    return r->chr_rom[address];
}

void mapper2_chr_write(rom* r, uint16_t address, byte value) {
    printf("WARNING: UxROM pattern tables (CHR ROM) written to! allowing it because I'm a dumb emulator\n");
    r->chr_rom[address] = value;
}
